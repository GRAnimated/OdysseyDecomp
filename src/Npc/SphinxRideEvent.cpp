#include "Npc/SphinxRideEvent.h"

#include "Library/Area/AreaObjUtil.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementInfo.h"
#include "Library/Stage/StageSwitchUtil.h"

#include "Npc/SphinxRide.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/DemoUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerDemoUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(SphinxRideEvent, CheckInArea);
NERVE_IMPL(SphinxRideEvent, EventPyramid);
NERVE_IMPL(SphinxRideEvent, EventSphinxStone);
NERVES_MAKE_NOSTRUCT(SphinxRideEvent, CheckInArea, EventPyramid, EventSphinxStone);
}  // namespace

SphinxRideEvent::SphinxRideEvent(SphinxRide* sphinxRide)
    : al::LiveActor("乗れるスフィンクスイベント"), mSphinxRide(sphinxRide) {}

// NON_MATCHING: basic block reordering in mEventType if-else chain
void SphinxRideEvent::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, "SphinxRideEvent", nullptr);

    if (!al::isExistLinkChild(info, "StartEventPosPlayer", 0) ||
        !al::isExistLinkChild(info, "StartEventPosSphinxRide", 0)) {
        makeActorDead();
        return;
    }

    al::getLinkTR(&mSphinxRideTrans, &mSphinxRideRotate, info, "StartEventPosSphinxRide");

    al::PlacementInfo placementInfo;
    al::getLinksInfoByIndex(&placementInfo, info, "StartEventPosPlayer", 0);
    al::tryGetTrans(&mPlayerTrans, placementInfo);
    al::tryGetFront(&mPlayerFront, placementInfo);

    bool isExistPyramid = al::isExistLinkChild(info, "SwitchAreaOnEventPyramid", 0);
    bool isExistSphinxStone = al::isExistLinkChild(info, "SwitchAreaOnEventSphinxStone", 0);

    if (isExistPyramid)
        mAreaObjGroup =
            al::createLinkAreaGroup(this, info, "SwitchAreaOnEventPyramid",
                                    "帽子メッセージエリアグループ", "帽子メッセージエリア");

    if (isExistSphinxStone)
        mAreaObjGroup =
            al::createLinkAreaGroup(this, info, "SwitchAreaOnEventSphinxStone",
                                    "帽子メッセージエリアグループ", "帽子メッセージエリア");

    if (isExistPyramid)
        mEventType = 0;

    if (isExistSphinxStone)
        mEventType = 1;

    if (mEventType == 1) {
        mEventFlowExecutor = rs::initEventFlow(this, info, "SphinxRideEventStone", nullptr);
    } else if (mEventType == 0) {
        mEventFlowExecutor = rs::initEventFlow(this, info, "SphinxRideEventPyramid", nullptr);
        rs::initEventCharacterName(mEventFlowExecutor, info, "SphinxRide");
    } else {
        makeActorDead();
        return;
    }

    al::initEventReceiver(mEventFlowExecutor, this);
    rs::initEventQueryJudge(mEventFlowExecutor, this);
    rs::initEventCameraObject(mEventFlowExecutor, info, "Default");
    al::initNerve(this, &CheckInArea, 0);
    makeActorAlive();

    s32 shineIndex = GameDataFunction::tryFindLinkedShineIndexByLinkName(this, info, "GetAutoMoon");
    GameDataHolderAccessor accessor(this);
    if (GameDataFunction::isGotShine(accessor, shineIndex))
        al::tryOnStageSwitch(this, "SwitchShineGetOn");
}

void SphinxRideEvent::exeCheckInArea() {
    if (al::isInAreaObj(mAreaObjGroup, al::getTrans(mSphinxRide))) {
        if (mEventType == 0)
            al::setNerve(this, &EventPyramid);
        else if (mEventType == 1)
            al::setNerve(this, &EventSphinxStone);
    }
}

void SphinxRideEvent::exeEventPyramid() {
    if (al::isFirstStep(this)) {
        mSphinxRide->setupEventFade();
        rs::startEventFlow(mEventFlowExecutor, "Event");
    }

    if (rs::updateEventFlow(mEventFlowExecutor)) {
        mSphinxRide->endEventWait(0.0f);
        kill();
    }
}

void SphinxRideEvent::exeEventSphinxStone() {
    if (al::isFirstStep(this)) {
        mSphinxRide->setupEventFade();
        rs::startEventFlow(mEventFlowExecutor, "Event");
    }

    if (rs::updateEventFlow(mEventFlowExecutor)) {
        mSphinxRide->endEventWait(0.0f);
        kill();
    }
}

bool SphinxRideEvent::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                 al::HitSensor* self) {
    if (al::isMsgPlayerDisregard(msg) || rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;

    return rs::isMsgPlayerDisregardTargetMarker(msg);
}

bool SphinxRideEvent::receiveEvent(const al::EventFlowEventData* event) {
    if (!al::isEventName(event, "SetPosPlayerAndSphinx"))
        return false;

    rs::addDemoActor(mSphinxRide, false);
    al::resetRotatePosition(mSphinxRide, mSphinxRideRotate, mSphinxRideTrans);
    mSphinxRide->startEventWait();

    sead::Quatf quat;
    al::makeQuatFrontNoSupport(&quat, mPlayerFront);
    rs::replaceDemoPlayer(this, mPlayerTrans, quat);
    rs::startActionDemoPlayer(this, "Wait");
    al::tryOnStageSwitch(this, "SwitchShineGetOn");
    return true;
}

const char* SphinxRideEvent::judgeQuery(const char* query) const {
    return nullptr;
}
