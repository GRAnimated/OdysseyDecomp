#include "Npc/Yukimaru.h"

#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "Library/Effect/EffectSystemInfo.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Shadow/ActorShadowUtil.h"

#include "Npc/NpcStateReaction.h"
#include "Npc/NpcStateReactionParam.h"
#include "Npc/NpcStateTurnParam.h"
#include "Npc/NpcStateWait.h"
#include "Npc/NpcStateWaitParam.h"
#include "Npc/YukimaruStateHack.h"
#include "Npc/YukimaruStateMove.h"
#include "Util/Hack.h"
#include "Util/ItemUtil.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(Yukimaru, Event);
NERVE_IMPL(Yukimaru, Reaction);
NERVE_IMPL(Yukimaru, Hack);
NERVE_IMPL(Yukimaru, HackAfter);
NERVES_MAKE_NOSTRUCT(Yukimaru, Event, Reaction, Hack, HackAfter);

NpcStateTurnParam sTurnParam(20.0f, 0.0f, 0.0f, 1000.0f, false, false, 20);
}  // namespace

Yukimaru::Yukimaru(const char* name) : al::LiveActor(name) {}

void Yukimaru::init(const al::ActorInitInfo& initInfo) {
    al::initActorWithArchiveName(this, initInfo, "SnowManRacer", "Hack");
    mQuat = al::getQuat(this);
    al::initJointControllerKeeper(this, 1);
    al::initJointGlobalQuatController(this, &mQuat, "Rotate");
    al::createAndSetColliderSpecialPurpose(this, "MoveLimit");
    mCapTargetInfo = rs::createCapTargetInfo(this, nullptr);

    mStateHack = new YukimaruStateHack(this);
    mEventFlowExecutor = rs::initEventFlowFromPlacementInfo(this, initInfo, "TalkNpc");
    al::initEventReceiver(mEventFlowExecutor, this);

    if (rs::isDefinedEventCamera(mEventFlowExecutor, "Default"))
        rs::initEventCameraTalk(mEventFlowExecutor, initInfo, "Default", -1.0f);

    mStateWaitParam = new NpcStateWaitParam("GuardWait", nullptr, "Turn", "GuardReaction", nullptr,
                                            "GuardReaction", false, nullptr, false);
    mStateWait = new NpcStateWait(this, initInfo, mStateWaitParam, &sTurnParam, nullptr);
    mReactionParam = new NpcStateReactionParam("GuardReaction", "GuardReaction");
    mStateReaction = NpcStateReaction::createForHackActor(this, mReactionParam);

    al::startVisAnim(this, "Off");
    al::onDepthShadowModel(this);
    al::invalidateDepthShadowMap(this);

    al::initNerve(this, &Event, 4);
    al::initNerveState(this, mStateWait, &Event, u8"イベント");
    al::initNerveState(this, mStateReaction, &Reaction, u8"リアクション");
    al::initNerveState(this, mStateHack, &Hack, u8"憑依");

    bool isHackStart = false;
    al::tryGetArg(&isHackStart, initInfo, "IsHackStart");
    if (isHackStart) {
        al::HitSensor* bodySensor = al::getHitSensor(this, "Body");
        rs::requestStageStartHack(this, bodySensor, mCapTargetInfo, nullptr);
        rs::tryInitItem(this, 10, initInfo, false);
    }

    makeActorAlive();
}

void Yukimaru::initAfterPlacement() {
    rs::startEventFlow(mEventFlowExecutor, "Init");
}

void Yukimaru::movement() {
    al::updateMaterialCodeAll(this);
    al::LiveActor::movement();
}

void Yukimaru::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isNerve(this, &Hack)) {
        mStateHack->attackSensor(self, other);
        return;
    }

    if (al::isNerve(this, &HackAfter) &&
        YukimaruMovement::attackSensor(this, self, other)) {
        mStateHack->mStateMove->startScaleBoundReaction(al::getVelocity(this).y);
    }

    rs::attackSensorNpcCommon(self, other);
}

bool Yukimaru::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) {
    if (rs::tryReceiveMsgInitCapTargetAndSetCapTargetInfo(msg, mCapTargetInfo))
        return true;

    if (rs::isMsgPlayerDisregardHomingAttack(msg)) {
        if (!isEnableHack())
            return true;
    }

    if (isEnableHack()) {
        if (mStateHack->tryStartDirect(msg)) {
            al::setNerve(this, &Hack);
            return true;
        }
    }

    if (isEnableHack()) {
        if ((rs::isMsgCapEnableLockOn(msg) || rs::isMsgCapCancelLockOn(msg)) &&
            !al::isNerve(this, &Hack))
            return true;
    }

    if (al::isNerve(this, &Hack)) {
        if (!mStateHack->receiveMsg(msg, other, self))
            return false;

        if (mStateHack->isEndCancel()) {
            afterHack();
            return true;
        }

        if (mStateHack->isEndKill()) {
            makeActorDead();
            return true;
        }

        return true;
    }

    if (rs::isMsgKillByShineGet(msg)) {
        al::tryKillEmitterAndParticleAll(this);
        makeActorDead();
        return true;
    }

    if (isEnableHack()) {
        if (rs::isMsgStartHack(msg) && !al::isNerve(this, &Hack)) {
            al::setVelocityZero(this);
            mStateHack->start(other, self, _159);
            al::setNerve(this, &Hack);
            return true;
        }
    }

    if (!al::isNerve(this, &Hack)) {
        if (mStateReaction->receiveMsg(msg, other, self)) {
            if (al::isNerve(this, &Event))
                al::setNerve(this, &Reaction);

            if (al::isNerve(this, &HackAfter)) {
                al::startHitReaction(this, u8"踏まれ");
                mStateHack->mStateMove->startScaleBoundReaction(al::getVelocity(this).y);
            }

            return true;
        }

        if (mStateReaction->receiveMsgNoReaction(msg, other, self))
            return true;
    }

    if (rs::isMsgPushToPlayer(msg)) {
        al::pushAndAddVelocityH(this, other, self, 1.3f);
        return true;
    }

    return false;
}

bool Yukimaru::isEnableHack() const {
    if (al::isNerve(this, &HackAfter))
        return al::isGreaterEqualStep(this, 30);
    return mIsEnableHack;
}

void Yukimaru::afterHack() {
    mStateWaitParam->mAction = "RunEnemy";
    mReactionParam->mReactionAnim = "ReactionRun";
    mReactionParam->mReactionEndAnim = "ReactionRun";
    _159 = true;
    al::scaleVelocity(this, 0.6f);
    al::setNerve(this, &HackAfter);
}

bool Yukimaru::isEnableReaction() const {
    return !al::isNerve(this, &Hack);
}

bool Yukimaru::receiveEvent(const al::EventFlowEventData* event) {
    if (al::isEventName(event, "HackDisable")) {
        mStateReaction->setCapReaction(false);
        mIsEnableHack = false;
        return true;
    }

    if (al::isEventName(event, "HackEnable")) {
        mStateWaitParam->mAction = "NoGuardWait";
        mReactionParam->mReactionAnim = "NoGuardReaction";
        mReactionParam->mReactionEndAnim = "NoGuardReaction";
        mStateReaction->setCapReaction(true);
        mIsEnableHack = true;
        return true;
    }

    return false;
}

void Yukimaru::disableHack() {
    mStateReaction->setCapReaction(false);
    mIsEnableHack = false;
}

void Yukimaru::enableHack() {
    mStateReaction->setCapReaction(true);
    mIsEnableHack = true;
}

void Yukimaru::exeEvent() {
    al::setVelocityZero(this);
    mStateHack->updateScale();
    al::updateNerveState(this);
    rs::updateEventFlow(mEventFlowExecutor);
}

void Yukimaru::exeReaction() {
    al::setVelocityZero(this);
    mStateHack->updateScale();
    al::updateNerveStateAndNextNerve(this, &Event);
}

void Yukimaru::exeHack() {
    if (al::updateNerveState(this))
        afterHack();
}

void Yukimaru::exeHackAfter() {
    if (al::isFirstStep(this))
        al::startAction(this, "HackEnd");

    mStateHack->updateMoveNoInput();

    if (al::isGreaterEqualStep(this, 90) && al::isOnGroundNoVelocity(this, 0) &&
        al::isVelocitySlowH(this, 1.0f)) {
        mStateHack->resetMoveEffect();
        al::setVelocityZero(this);
        al::setNerve(this, &Event);
    }
}
