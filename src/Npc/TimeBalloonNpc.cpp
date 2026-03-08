#include "Npc/TimeBalloonNpc.h"

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Collision/PartsConnectorUtil.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/Joint/JointSpringControllerHolder.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"

#include "Npc/NpcEventStateScare.h"
#include "Npc/NpcStateReaction.h"
#include "Npc/NpcStateReactionParam.h"
#include "Npc/RandomActionUpdater.h"
#include "Npc/TalkNpcActionAnimInfo.h"
#include "Npc/TalkNpcParam.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(TimeBalloonNpc, Event);
NERVE_IMPL(TimeBalloonNpc, Reaction);
NERVE_IMPL(TimeBalloonNpc, Scared);
NERVES_MAKE_NOSTRUCT(TimeBalloonNpc, Event, Reaction, Scared);

static NpcStateReactionParam sReactionParam("Reaction", "ReactionCap");
static NpcEventStateScareActionParam sScareParam("Scared");
}  // namespace

void TimeBalloonNpc::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, "Luigi", nullptr);
    al::startAction(this, "Wait");

    mTalkNpcActionAnimInfo = new TalkNpcActionAnimInfo();
    mTalkNpcActionAnimInfo->mWaitActionName = "Wait";
    mTalkNpcParam = rs::initTalkNpcParam(this, nullptr);
    mTalkNpcActionAnimInfo->init(this, info, mTalkNpcParam, nullptr);

    mRandomActionUpdater = new RandomActionUpdater(this, mTalkNpcActionAnimInfo);

    al::initJointControllerKeeper(this, 1);
    mNpcJointLookAtController =
        rs::tryCreateAndAppendNpcJointLookAtController(this, mTalkNpcParam, 2000.0f);

    auto* cache = new NpcFacialAnimCache;
    cache->talkNpcParam = mTalkNpcParam;
    cache->actor = this;
    cache->cachedAnimName = nullptr;
    cache->isEnabled = true;
    cache->cachedIsValid = true;
    mFacialAnimCache = cache;

    mNpcStateReaction = NpcStateReaction::createForHuman(this, &sReactionParam);
    al::initNerve(this, &Event, 4);
    al::initNerveState(this, mNpcStateReaction, &Reaction, u8"リアクション");

    mNpcEventStateScare = new NpcEventStateScare(this, &sScareParam);
    al::initNerveState(this, mNpcEventStateScare, &Scared, u8"怖がり");

    al::tryGetArg(&mScenarioNo, info, "ScenarioNo");

    if (mScenarioNo == -1) {
        mEventFlowExecutor = rs::initEventFlow(this, info, "TimeBalloonNpc", nullptr);
        rs::initEventCharacterName(mEventFlowExecutor, info, "TimeBalloonNpc");
        rs::initEventMovementTurnSeparate(mEventFlowExecutor, info);
        rs::initEventCameraObject(mEventFlowExecutor, info, "TalkCamera");
        rs::initEventCameraObject(mEventFlowExecutor, info, "TalkCameraPlay");
        al::calcLinkChildNum(info, "TutorialBalloon");
        al::tryGetLinksTrans(&mTutorialBalloonTrans, info, "TutorialBalloon");
        rs::initEventCameraObject(mEventFlowExecutor, info, "TalkCameraTutorial");
    } else {
        mEventFlowExecutor = rs::initEventFlow(this, info, "TimeBalloonNpc", "Wait");
        rs::initEventCharacterName(mEventFlowExecutor, info, "TimeBalloonNpc");
        rs::initEventMovementTurnSeparate(mEventFlowExecutor, info);
        rs::initEventCameraObject(mEventFlowExecutor, info, "TalkCamera");
    }

    al::LiveActor* balloon =
        new al::LiveActor(u8"[風船ゲーム]ルイージの風船");
    mBalloonActor = balloon;
    al::initChildActorWithArchiveNameNoPlacementInfo(balloon, info, "LuigiBalloon", nullptr);
    al::startAction(mBalloonActor, "Wait");
    al::initSubActorKeeperNoFile(this, info, 1);
    al::registerSubActorSyncAll(this, mBalloonActor);
    mBalloonActor->makeActorAlive();

    al::updatePoseMtx(mBalloonActor, al::getJointMtxPtr(this, "Bag"));
    al::initJointControllerKeeper(mBalloonActor, 2);

    auto* springHolder = new al::JointSpringControllerHolder();
    mJointSpringControllerHolder = springHolder;
    springHolder->init(mBalloonActor, "InitJointSpringCtrl");

    rs::registerPlayerStartInfoToHolderForTimeBalloon(this, info);
    al::tryGetLinksTrans(&mPlayerRestartTrans, info, "PlayerRestartPos");

    GameDataFunction::setTimeBalloonTrans(GameDataHolderAccessor(this), al::getTrans(this));

    mMtxConnector = al::tryCreateMtxConnector(this, info);

    if (al::isExistLinkChild(info, "KinopioNpc", 0))
        al::createLinksActorFromFactory(info, "KinopioNpc", 0);

    makeActorAlive();
}

void TimeBalloonNpc::initAfterPlacement() {
    if (mMtxConnector)
        al::attachMtxConnectorToCollision(mMtxConnector, this, false);
}

void TimeBalloonNpc::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isSensorEye(self)) {
        rs::sendMsgEventFlowScareCheck(other, self, mEventFlowExecutor);
        return;
    }
    if (!rs::sendMsgPushToPlayer(other, self))
        al::sendMsgPush(other, self);
}

bool TimeBalloonNpc::receiveMsg(const al::SensorMsg* msg, al::HitSensor* self, al::HitSensor* other) {
    if (rs::isMsgPlayerDisregardHomingAttack(msg) ||
        rs::isMsgPlayerDisregardTargetMarker(msg))
        return true;

    if (mTalkNpcParam->isInvalidTrampleSensor(other)) {
        if (al::isMsgPlayerTrampleReflectForCrossoverSensor(msg, self, other))
            return false;
        if (rs::isMsgPlayerAndCapObjHipDropReflectAll(msg))
            return false;
    }

    if (mNpcStateReaction->receiveMsg(msg, self, other)) {
        if (!al::isNerve(this, &Reaction))
            al::setNerve(this, &Reaction);
        return true;
    }

    return mNpcStateReaction->receiveMsgNoReaction(msg, self, other);
}

void TimeBalloonNpc::control() {
    if (mNpcJointLookAtController)
        rs::updateNpcJointLookAtController(mNpcJointLookAtController);

    NpcFacialAnimCache* cache = mFacialAnimCache;
    if (cache->isEnabled) {
        const char* animName = al::getPlayingSklAnimName(cache->actor, 0);
        bool isValid;
        if (cache->cachedAnimName == animName) {
            isValid = cache->cachedIsValid;
        } else {
            isValid = !cache->talkNpcParam->isInvalidJointLookSklAnim(animName);
            cache->cachedIsValid = isValid;
            cache->cachedAnimName = animName;
        }
        if (isValid)
            rs::tryUpdateNpcEyeLineAnim(cache->actor, cache->talkNpcParam);
    } else if (cache->cachedIsValid) {
        cache->cachedIsValid = false;
        cache->cachedAnimName = nullptr;
    }

    sead::Vector3f bagPos;
    sead::Vector3f bagUpDir;
    sead::Quatf jointQuat;
    al::calcJointPos(&bagPos, this, "Bag");
    al::calcJointUpDir(&bagUpDir, this, "Bag");
    al::calcJointQuat(&jointQuat, this, "Bag");

    sead::Quatf rotQuat;
    al::makeQuatRotationRate(&rotQuat, bagUpDir, sead::Vector3f::ey, 1.0f);
    jointQuat = rotQuat * jointQuat;

    al::updatePoseQuat(mBalloonActor, jointQuat);
    al::updatePoseTrans(mBalloonActor, bagPos);
}

void TimeBalloonNpc::exeEvent() {
    if (mMtxConnector)
        al::connectPoseQT(this, mMtxConnector);
    if (al::isFirstStep(this))
        al::offCollide(this);
    mRandomActionUpdater->update();
}

void TimeBalloonNpc::exeReaction() {
    if (al::updateNerveState(this))
        al::setNerve(this, &Event);
}

bool TimeBalloonNpc::isReaction() const {
    return al::isNerve(this, &Reaction);
}

void TimeBalloonNpc::exeScared() {
    if (al::isFirstStep(this)) {
        rs::startEventFlow(mEventFlowExecutor, "Scared");
        rs::stopEventFlow(mEventFlowExecutor);
    }
    rs::updateEventFlow(mEventFlowExecutor);
    if (al::updateNerveState(this))
        al::setNerve(this, &Event);
}
