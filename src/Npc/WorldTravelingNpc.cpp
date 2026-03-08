#include "Npc/WorldTravelingNpc.h"

#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "Library/Event/EventFlowExecutor.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActorGroup.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"

#include "Npc/TalkNpcActionAnimInfo.h"
#include "Npc/TalkNpcStateEvent.h"
#include "Npc/WorldTravelingPartner.h"
#include "System/GameDataFunction.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerDemoUtil.h"

namespace {
NERVE_IMPL(WorldTravelingNpc, Event);
NERVES_MAKE_NOSTRUCT(WorldTravelingNpc, Event);
}  // namespace

// NON_MATCHING: sead WFixedSafeString clear() implementation differs from target binary
WorldTravelingNpc::WorldTravelingNpc(const char* name) : al::LiveActor(name) {}

// NON_MATCHING: 3-line regswap in mEventFlowData assignment load order
void WorldTravelingNpc::init(const al::ActorInitInfo& info) {
    sead::SafeString archiveName("DesertMan");
    al::initActorWithArchiveName(this, info, archiveName, "WorldTravel");
    al::initNerve(this, &Event, 1);

    mTalkNpcActionAnimInfo = new TalkNpcActionAnimInfo();
    mTalkNpcActionAnimInfo->mWaitActionName = "Wait";
    mTalkNpcParam = rs::initTalkNpcParam(this, nullptr);

    const char* status;
    if (GameDataFunction::isFirstWorldTravelingStatus(this))
        status = "Init";
    else
        status = GameDataFunction::getWorldTravelingStatus(this);

    const char* eventName = al::getStringArg(info, "EventName");
    mEventFlowExecutor = rs::initEventFlow(this, info, "WorldTravelingNpc", eventName);

    s32 shineIndex = GameDataFunction::tryFindLinkedShineIndex(this, info);
    GameDataFunction::disableHintByShineIndex(this, shineIndex);

    if (!al::isExistEventEntry(mEventFlowExecutor, status)) {
        makeActorDead();
        return;
    }

    al::initEventReceiver(mEventFlowExecutor,
                          static_cast<al::IEventFlowEventReceiver*>(this));
    rs::initEventMovementWait(mEventFlowExecutor, info);
    rs::initEventCharacterName(mEventFlowExecutor, info, "WorldTravelingNpc");
    rs::makeEventCharacterName(&mCharacterName, info, "WorldTravelingPartner");
    rs::startEventFlow(mEventFlowExecutor, status);

    if (rs::isDefinedEventCamera(mEventFlowExecutor, "Default"))
        rs::initEventCameraObject(mEventFlowExecutor, info, "Default");

    auto* partner = new WorldTravelingPartner(u8"世界旅行の相棒");
    mPartner = partner;
    al::initLinksActor(partner, info, "Partner", 0);
    WorldTravelingPartner* p = mPartner;
    p->mEventFlowData = mEventFlowExecutor;
    al::invalidateClipping(mPartner);
    al::registerSubActorSyncClipping(this, mPartner);

    auto* midpoint = new sead::Vector3f();
    const sead::Vector3f& myTrans = al::getTrans(this);
    const sead::Vector3f& partnerTrans = al::getTrans(mPartner);
    *midpoint = (myTrans + partnerTrans) * 0.5f;

    const sead::Vector3f& selfTrans = al::getTrans(this);
    f32 dx = midpoint->x - selfTrans.x;
    f32 dy = midpoint->y - selfTrans.y;
    f32 dz = midpoint->z - selfTrans.z;
    f32 distance = sqrtf(dx * dx + dy * dy + dz * dz);
    al::setClippingInfo(this, distance + 200.0f, midpoint);

    mCarGroup = new al::LiveActorGroup(u8"世界旅行の装飾品", 1);
    if (al::isExistLinkChild(info, "Car", 0)) {
        al::LiveActor* car = al::createLinksActorFromFactory(info, "Car", 0);
        if (car != nullptr)
            mCarGroup->registerActor(car);
    }

    auto* stateEvent =
        new TalkNpcStateEvent(u8"イベント", this, mTalkNpcParam, mEventFlowExecutor, -1);
    mTalkNpcStateEvent = stateEvent;
    ActorStateReactionBase* reaction = rs::createNpcStateReaction(this, mTalkNpcParam, nullptr);
    stateEvent->initStateReaction(reaction);

    al::initNerveState(this, mTalkNpcStateEvent, &Event, u8"イベント");
    rs::setNpcMaterialAnimFromPlacementInfo(this, info);

    makeActorAlive();
}

void WorldTravelingNpc::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isSensorEye(self))
        rs::sendMsgEventFlowScareCheck(other, self, mEventFlowExecutor);
    else
        rs::attackSensorNpcCommon(self, other);
}

bool WorldTravelingNpc::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                                   al::HitSensor* self) {
    if (rs::isMsgPlayerDisregardHomingAttack(message))
        return true;

    if (rs::isInvalidTrampleSensor(self, mTalkNpcParam)) {
        if (mTalkNpcStateEvent->tryStartReactionWithoutTrample(message, other, self))
            return true;
    } else {
        if (mTalkNpcStateEvent->tryStartReaction(message, other, self))
            return true;
    }

    return rs::receiveMsgNpcCommonNoReaction(message, other, self);
}

// NON_MATCHING: compiler defers watchParam load to join point via address computation,
// target loads eagerly in each branch — 5 cycles attempted, no progress
bool WorldTravelingNpc::receiveEvent(const al::EventFlowEventData* data) {
    if (TalkNpcFunction::receiveEventChangeWaitAction(mTalkNpcActionAnimInfo, data,
                                                      mTalkNpcParam))
        return true;

    if (al::isEventName(data, "UpdateWorldTravelingProgress")) {
        const char* progress = al::getEventDataParamString(data, "ProgressName");
        GameDataFunction::saveWorldTravelingStatus(this, progress);
        return true;
    }

    const TalkNpcParam* watchParam;
    al::LiveActor* watchActor = this;

    if (al::isEventName(data, "PartnerWait")) {
        mPartner->startWait();
        rs::resetEventCharacterName(mEventFlowExecutor);
        al::resetTalkSubActorName(mEventFlowExecutor);
        watchParam = mTalkNpcParam;
    } else if (al::isEventName(data, "PartnerTalk")) {
        mPartner->startTalk();
        rs::swapEventCharacterName(mEventFlowExecutor, &mCharacterName);
        al::setTalkSubActorName(mEventFlowExecutor, u8"世界旅行の相棒");
        watchActor = mPartner;
        watchParam = mPartner->mTalkNpcParam;
    } else {
        return false;
    }

    sead::Vector3f watchTrans = {0.0f, 0.0f, 0.0f};
    rs::calcPlayerWatchTrans(&watchTrans, watchActor, watchParam);
    rs::validateWatchTarget(watchActor, watchTrans);
    return true;
}

void WorldTravelingNpc::exeEvent() {
    al::updateNerveState(this);
}
