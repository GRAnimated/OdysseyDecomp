#include "Npc/HintNpc.h"

#include "Library/Base/StringUtil.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"

#include "Npc/ActorStateReactionBase.h"
#include "Npc/EventActorMovementTurn.h"
#include "Npc/TalkNpcActionAnimInfo.h"
#include "Npc/TalkNpcCap.h"
#include "Npc/TalkNpcStateEvent.h"
#include "System/GameDataFunction.h"
#include "System/GameDataUtil.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"

namespace {
NERVE_IMPL(HintNpc, Event);
NERVE_IMPL(HintNpc, Reaction);
NERVES_MAKE_NOSTRUCT(HintNpc, Event, Reaction);
}  // namespace

HintNpc::HintNpc(const char* name) : al::LiveActor(name) {}

void HintNpc::init(const al::ActorInitInfo& initInfo) {
    mActionAnimInfo = new TalkNpcActionAnimInfo();
    mActionAnimInfo->mWaitActionName = "Wait";
    al::initActor(this, initInfo);
    al::initNerve(this, &Event, 2);

    mNpcCap = TalkNpcCap::createForHintNpc(this, initInfo);
    al::initSubActorKeeperNoFile(this, initInfo, 1);
    al::registerSubActorSyncClipping(this, mNpcCap);
    al::onSyncAlphaMaskSubActorAll(this);

    mParam = rs::initTalkNpcParam(this, nullptr);
    mActionAnimInfo->init(this, initInfo, mParam, nullptr);

    mEventFlowExecutor = rs::initEventFlow(this, initInfo, nullptr, nullptr);
    rs::initEventActionNameConverter(mEventFlowExecutor, mActionAnimInfo);
    rs::initEventCharacterName(mEventFlowExecutor, initInfo, "KinopioMemberHint");
    rs::initEventCameraTalk(mEventFlowExecutor, initInfo, "Default", -1.0f);
    al::initEventReceiver(mEventFlowExecutor, this);
    rs::initEventQueryJudge(mEventFlowExecutor, this);

    mMovementTurn = new EventActorMovementTurn(this, mParam, mActionAnimInfo);
    rs::initEventMovement(mEventFlowExecutor, mMovementTurn, initInfo);

    mStateEvent = new TalkNpcStateEvent(u8"イベント", this, mParam, mEventFlowExecutor, -1);
    mReaction = rs::createNpcStateReaction(this, mParam, nullptr);
    al::initNerveState(this, mStateEvent, &Event, u8"イベント");
    al::initNerveState(this, mReaction, &Reaction, u8"リアクション");

    bool isExistTargetShine = false;
    if (al::tryGetArg(&isExistTargetShine, initInfo, "IsExistTargetShine") && isExistTargetShine) {
        auto* p = static_cast<const char**>(operator new(sizeof(const char*) * 2));
        p[0] = nullptr;
        p[1] = nullptr;
        mShineInfo = p;
        al::tryGetStringArg(&mShineInfo[0], initInfo, "ShineStageName");
        al::tryGetStringArg(&mShineInfo[1], initInfo, "ShinePlacementId");
    }

    GameDataFunction::setHintNpcTrans(this);
    makeActorAlive();
}

void HintNpc::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isSensorEye(self))
        rs::sendMsgEventFlowScareCheck(other, self, mEventFlowExecutor);
    else
        rs::attackSensorNpcCommon(self, other);
}

bool HintNpc::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) {
    if (rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;

    if (mReaction->receiveMsg(msg, other, self)) {
        if (!al::isNerve(this, &Reaction))
            al::setNerve(this, &Reaction);
        return true;
    }

    return mReaction->receiveMsgNoReaction(msg, other, self);
}

// NON_MATCHING: compiler collapses if/else bool store into AND+STRB
bool HintNpc::receiveEvent(const al::EventFlowEventData* data) {
    if (TalkNpcFunction::receiveEventChangeWaitAction(mActionAnimInfo, data, mParam)) {
        if (al::isEqualString(mActionAnimInfo->_38, "WaitMap"))
            reinterpret_cast<u8*>(mMovementTurn)[0x70] = true;
        else
            reinterpret_cast<u8*>(mMovementTurn)[0x70] = false;
        return true;
    }

    if (al::isEventName(data, "AfterFirstTalkHintNpc")) {
        rs::onFlagHintNpcFirstTalk(this);
        return true;
    }

    return false;
}

const char* HintNpc::judgeQuery(const char* query) const {
    if (!rs::isOnFlagHintNpcFirstTalk(this))
        return "First";

    GameDataHolderAccessor accessor(this);
    if (!GameDataFunction::isEnableUnlockHint(accessor))
        return "Deactive";

    if (mShineInfo && mShineInfo[0] && !al::isEqualString(mShineInfo[0], "None") && mShineInfo[1] &&
        !al::isEqualString(mShineInfo[1], "None"))
        return "WaitHappy";

    return "Wait";
}

void HintNpc::exeEvent() {
    al::updateNerveState(this);
}

void HintNpc::exeReaction() {
    al::updateNerveStateAndNextNerve(this, &Event);
}
