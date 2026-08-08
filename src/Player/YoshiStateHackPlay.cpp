#include "Player/YoshiStateHackPlay.h"

#include "Library/Effect/EffectSystemInfo.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/HackerJudgeStartRun.h"
#include "Player/HackerJudgeStartJump.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerCounterForceRun.h"
#include "Player/PlayerEyeSensorHitHolder.h"
#include "Player/PlayerJumpMessageRequest.h"
#include "Player/PlayerTrigger.h"
#include "Player/PlayerWallActionHistory.h"
#include "Player/YoshiActionTongueAttack.h"
#include "Player/YoshiJudgeFallFromGround.h"
#include "Player/YoshiJudgeStartJumpFlap.h"
#include "Player/YoshiJudgeStartTongueShrink.h"
#include "Player/YoshiJudgeWallCling.h"
#include "Player/YoshiJudgeWallHitDown.h"
#include "Player/YoshiStateHackDown.h"
#include "Player/YoshiStateHackJump.h"
#include "Player/YoshiStateHackRun.h"
#include "Player/YoshiStateHackTongueShrink.h"
#include "Player/YoshiStateHackWait.h"
#include "Player/YoshiStateHackWallAir.h"
#include "Player/YoshiStateHackWallPopUp.h"
#include "Player/YoshiTongue.h"
#include "Util/JudgeUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackFunction.h"
#include "Util/SensorMsgFunction.h"
#include "Util/YoshiUtil.h"

namespace {
NERVE_IMPL(YoshiStateHackPlay, Wait)
NERVE_IMPL(YoshiStateHackPlay, Jump)
NERVE_IMPL(YoshiStateHackPlay, Flap)
NERVE_IMPL(YoshiStateHackPlay, WallAir)
NERVE_IMPL(YoshiStateHackPlay, WallPopUp)
NERVE_IMPL(YoshiStateHackPlay, TongueShrink)
NERVE_IMPL(YoshiStateHackPlay, Fall)
NERVE_IMPL(YoshiStateHackPlay, Run)
NERVE_IMPL(YoshiStateHackPlay, Down)

NERVES_MAKE_STRUCT(YoshiStateHackPlay, Wait, Jump, Flap, WallAir, WallPopUp, TongueShrink, Fall,
                   Run, Down)

void updateForceRun(PlayerCounterForceRun* counter, al::IUseNerve* nerveUser,
                    const al::LiveActor* actor, IUsePlayerCollision* collision,
                    YoshiStateHackTongueShrink* state);
}  // namespace

void YoshiStateHackPlay::appear() {
    mWallActionHistory->reset();
    HackerStateBase::appear();
    setNerveOnGround();
}

void YoshiStateHackPlay::setNerveOnGround() {
    if (!rs::isOnGroundRunAngle(mActor, mCollision, mConst)) {
        al::setNerve(this, &NrvYoshiStateHackPlay.Fall);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeStartRun))
        al::setNerve(this, &NrvYoshiStateHackPlay.Run);
    else
        al::setNerve(this, &NrvYoshiStateHackPlay.Wait);
}

void YoshiStateHackPlay::kill() {
    prepareEndHack();
    HackerStateBase::kill();
}

bool YoshiStateHackPlay::update() {
    rs::updateJudge(mJudgeStartJumpFlap);
    if (mTongueAttack->tryStartEatFinish() && al::isNerve(this, &NrvYoshiStateHackPlay.Wait))
        mStateWait->endTongueEat();
    mTongueAttack->update();
    executePreUpdateNerveChange();
    return HackerStateBase::update();
}

void YoshiStateHackPlay::executePreUpdateNerveChange() {
    const al::LiveActor* actor = mActor;
    if (!rs::isTouchJumpCode(actor, mCollision))
        return;

    rs::requestReactionJumpCode(actor, mCollision, al::getHitSensor(actor, "Body"));
    PlayerJumpMessageRequest* request = mJumpMessageRequest;
    const f32 jumpPower = mConst->getTrampleJumpCodePower();
    request->jumpType = static_cast<PlayerJumpType>(1);
    request->jumpPower = jumpPower;
    request->extendFrame = 0;
    al::setNerve(this, &NrvYoshiStateHackPlay.Jump);
}

void YoshiStateHackPlay::updatePrevMovement() {
    mEyeSensorHitHolder->clear();
}

void YoshiStateHackPlay::updateAfterMovement() {
    mWallActionHistory->update(mCollision);
    mCounterForceRun->update();
    mTongueAttack->syncEatVis();
}

void YoshiStateHackPlay::prepareEndHack() {
    mTongue->endHack();
}

// NON_MATCHING: target/current are 84/84; the s32 wall-damage helper return restores target size, but current normalizes with CMP/CSET where target tests bit 0 and branches to literal false/true returns. Next find a natural caller/source idiom that preserves the target TBZ branch instead of boolean canonicalization.
bool YoshiStateHackPlay::reactionCollidedCollisionCodeDamage() const {
    if (!al::isNerve(this, &NrvYoshiStateHackPlay.WallAir))
        return false;
    if (!mStateWallAir->isCling())
        return false;
    return rs::isCollidedDamageCodeAnyWallHit(mCollision);
}

bool YoshiStateHackPlay::isActiveHeadCorrection() const {
    if (isDead())
        return false;
    if (mTongue->isEnableStartAttack())
        return false;
    if (al::isNerve(this, &NrvYoshiStateHackPlay.Jump))
        return !mStateJump->isJumpTurn();
    if (!al::isNerve(this, &NrvYoshiStateHackPlay.TongueShrink))
        return true;
    return !mStateTongueShrink->isJumpRolling();
}

bool YoshiStateHackPlay::isCollisionShapeTongueJump() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvYoshiStateHackPlay.TongueShrink))
        return mStateTongueShrink->isCollisionShapeJump();
    if (!al::isNerve(this, &NrvYoshiStateHackPlay.WallAir) || mStateWallAir->isDead())
        return false;
    return mStateWallAir->isAir();
}

bool YoshiStateHackPlay::tryGetLookAtTonguePos(sead::Vector3f* position) const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvYoshiStateHackPlay.TongueShrink) &&
        !mStateTongueShrink->isEnableTongueKeepAction())
        return false;
    if (!mTongue->isEnableLookAtTip())
        return false;
    mTongue->calcTongueTipPos(position);
    return true;
}

bool YoshiStateHackPlay::tryCalcTonguePullForce(f32* force, sead::Vector3f* direction) const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvYoshiStateHackPlay.TongueShrink) &&
        !mStateTongueShrink->isEnablePullForce())
        return false;
    if (al::isNerve(this, &NrvYoshiStateHackPlay.WallAir) && mStateWallAir->isCling())
        return false;
    return mTongue->tryCalcTonguePullForce(force, direction);
}

void YoshiStateHackPlay::calcGroundPoseRate(f32* frontRate, f32* sideRate) const {
    *frontRate = 0.0f;
    *sideRate = 0.0f;

    if (al::isNerve(this, &NrvYoshiStateHackPlay.Wait)) {
        *frontRate = 1.0f;
        *sideRate = 1.0f;
        return;
    }

    if (al::isNerve(this, &NrvYoshiStateHackPlay.Run)) {
        *frontRate = mStateRun->getPoseRate();
        *sideRate = 0.0f;
    }
}

void YoshiStateHackPlay::startFruitShineGetDemo() {
    if (mAnimator->isUpperBodyAnimAttached()) {
        if (mTongueAttack->isUseSubParts())
            mAnimator->startUpperBodyAnimSubParts("EatEnd");
        else
            mAnimator->startUpperBodyAnim("EatEnd");

        al::LiveActor* modelActor = _28;
        const f32 frameMax = al::getSklAnimFrameMax(modelActor, "EatEnd");
        al::setPartialSklAnimFrame(modelActor, 0, frameMax);
        al::resetPosition(_28);
        mAnimator->clearUpperBodyAnim();
    }

    if (mAnimator->isAnim("EatEnd"))
        mAnimator->startAnim("Wait");

    al::tryKillEmitterAndParticleAll(mActor);
    al::tryKillEmitterAndParticleAll(mTongue);
}

void YoshiStateHackPlay::exeWait() {
    if (al::isFirstStep(this))
        rs::resetJudge(mJudgeFallFromGround);

    al::updateNerveState(this);
    const al::LiveActor* actor = mActor;
    const IUsePlayerCollision* collision = mCollision;
    YoshiActionTongueAttack* tongueAttack = mTongueAttack;
    sead::Vector3f groundNormal = {0.0f, 0.0f, 0.0f};
    rs::calcGroundNormalOrUpDir(&groundNormal, actor, collision);
    if (tongueAttack->tryStartAttackGround(groundNormal))
        mStateWait->startTongueEat();
    tryChangeNerveOnGround();
}

bool YoshiStateHackPlay::tryChangeNerveOnGround() {
    if (rs::updateJudgeAndResult(mJudgeStartTongueShrink)) {
        setNerveTongueShrink();
        return true;
    }

    if (rs::updateJudgeAndResult(mJudgeStartJump)) {
        sead::Vector3f turnAngle = {0.0f, 0.0f, 0.0f};
        if (mStateRun->tryTurnJump(&turnAngle)) {
            PlayerJumpMessageRequest* request = mJumpMessageRequest;
            request->jumpType = static_cast<PlayerJumpType>(0x10);
            request->turnJumpAngle = turnAngle;
        }
        al::setNerve(this, &NrvYoshiStateHackPlay.Jump);
        return true;
    }

    if (!al::isNerve(this, &NrvYoshiStateHackPlay.Run) &&
        rs::updateJudgeAndResult(mJudgeStartRun)) {
        al::setNerve(this, &NrvYoshiStateHackPlay.Run);
        return true;
    }

    if (!rs::updateJudgeAndResult(mJudgeFallFromGround))
        return false;

    al::setNerve(this, &NrvYoshiStateHackPlay.Fall);
    return true;
}

void YoshiStateHackPlay::exeJump() {
    if (al::updateNerveState(this)) {
        setNerveOnGround();
        return;
    }

    YoshiActionTongueAttack* tongueAttack = mTongueAttack;
    sead::Vector3f up = -al::getGravity(mActor);
    tongueAttack->tryStartAttack(up);
    tryChangeNerveAir();
}

bool YoshiStateHackPlay::tryChangeNerveAir() {
    if (rs::updateJudgeAndResult(mJudgeStartTongueShrink)) {
        setNerveTongueShrink();
        return true;
    }

    const al::LiveActor* actor = mActor;
    const IUsePlayerCollision* collision = mCollision;
    const f32 angle = mConst->getStandAngleMin();
    if (rs::isOnGroundLessAngle(actor, collision, angle)) {
        if (!rs::isOnGroundRunAngle(mActor, mCollision, mConst)) {
            al::setNerve(this, &NrvYoshiStateHackPlay.Fall);
        } else if (rs::updateJudgeAndResult(mJudgeStartRun)) {
            al::setNerve(this, &NrvYoshiStateHackPlay.Run);
        } else {
            al::setNerve(this, &NrvYoshiStateHackPlay.Wait);
        }
        return true;
    }

    if (!rs::isJudge(mJudgeStartJumpFlap))
        return false;

    al::setNerve(this, &NrvYoshiStateHackPlay.Flap);
    return true;
}

void YoshiStateHackPlay::exeFlap() {
    if (al::isFirstStep(this))
        mJudgeStartJumpFlap->start();

    if (al::updateNerveState(this)) {
        setNerveOnGround();
        return;
    }

    YoshiActionTongueAttack* tongueAttack = mTongueAttack;
    sead::Vector3f up = -al::getGravity(mActor);
    tongueAttack->tryStartAttack(up);
    tryChangeNerveAir();
}

void YoshiStateHackPlay::exeWallAir() {
    if (al::isFirstStep(this)) {
        mJudgeStartJumpFlap->start();
        mCounterForceRun->reset();
    }

    if (al::updateNerveState(this)) {
        setNerveOnGround();
        return;
    }

    if (mStateWallAir->isCling()) {
        YoshiActionTongueAttack* tongueAttack = mTongueAttack;
        al::LiveActor* actor = mActor;
        IUsePlayerCollision* collision = mCollision;
        sead::Vector3f wallNormal = {0.0f, 0.0f, 0.0f};
        rs::calcWallNormalOrReverseFrontDir(&wallNormal, actor, collision);
        sead::Vector3f up = {0.0f, 0.0f, 0.0f};
        al::calcUpDir(&up, actor);
        tongueAttack->tryStartAttackFromWall(up, wallNormal);
        if (rs::updateJudgeAndResult(mJudgeStartTongueShrink)) {
            mStateWallAir->startShrink();
            setNerveTongueShrink();
            return;
        }
    }

    if (mStateWallAir->isAir()) {
        YoshiActionTongueAttack* tongueAttack = mTongueAttack;
        al::LiveActor* actor = mActor;
        sead::Vector3f front = {0.0f, 0.0f, 0.0f};
        al::calcFrontDir(&front, actor);
        sead::Vector3f up = {0.0f, 0.0f, 0.0f};
        al::calcUpDir(&up, actor);
        tongueAttack->tryStartAttackFromWallAir(up, front);
        tryChangeNerveAir();
    }
}

void YoshiStateHackPlay::setNerveTongueShrink() {
    if (mJudgeStartTongueShrink->isConnectGround()) {
        YoshiStateHackTongueShrink* state = mStateTongueShrink;
        YoshiJudgeStartTongueShrink* judge = mJudgeStartTongueShrink;
        state->setupGroundSnap(judge->getCollisionParts(), judge->getConnectPos(),
                               judge->getConnectDir(), judge->getConnectNormal());
    } else if (mJudgeStartTongueShrink->isConnectWall()) {
        YoshiStateHackTongueShrink* state = mStateTongueShrink;
        YoshiJudgeStartTongueShrink* judge = mJudgeStartTongueShrink;
        state->setupWallSnap(judge->getCollisionParts(), judge->getConnectPos(),
                             judge->getTongueTipPos(), judge->getConnectNormal(),
                             judge->isWallClimb());
    }

    al::setNerve(this, &NrvYoshiStateHackPlay.TongueShrink);
    updateForceRun(mCounterForceRun, this, mActor, mCollision, mStateTongueShrink);
}

void YoshiStateHackPlay::exeWallPopUp() {
    if (al::updateNerveState(this))
        setNerveOnGround();
}

void YoshiStateHackPlay::exeTongueShrink() {
    updateForceRun(mCounterForceRun, this, mActor, mCollision, mStateTongueShrink);

    const bool isEnd = al::updateNerveState(this);
    if (isEnd) {
        if (mStateTongueShrink->isEndCancelForceRun())
            mCounterForceRun->reset();
        setNerveOnGround();
        return;
    }

    if (mStateTongueShrink->isGroundConnectRun() && rs::updateJudgeAndResult(mJudgeWallHitDown)) {
        al::setNerve(this, &NrvYoshiStateHackPlay.Down);
        return;
    }

    if (mStateTongueShrink->isEnableWallCling()) {
        if (rs::updateJudgeAndResult(mJudgeWallCling)) {
            YoshiJudgeWallCling* judge = mJudgeWallCling;
            const al::CollisionParts* collisionParts = judge->getCollisionParts();
            if (judge->isWallPopUp()) {
                YoshiStateHackWallPopUp* wallPopUp = mStateWallPopUp;
                sead::Vector3f reverseNormal = -judge->getWallNormal();
                wallPopUp->setupSnap(collisionParts, judge->getWallPos(), reverseNormal,
                                     judge->getPopUpDir());
                al::setNerve(this, &NrvYoshiStateHackPlay.WallPopUp);
                return;
            }

            mStateWallAir->setupCling(collisionParts, judge->getWallPos(), judge->getWallNormal());
            al::setNerve(this, &NrvYoshiStateHackPlay.WallAir);
            return;
        }

        if (mJudgeWallCling->isDamageWall()) {
            setNerveOnGround();
            return;
        }
    }

    if (mStateTongueShrink->isEnableShrinkEndJump() && rs::updateJudgeAndResult(mJudgeStartJump)) {
        mStateTongueShrink->endShrinkAndJump();
        al::setNerve(this, &NrvYoshiStateHackPlay.Jump);
    }
}

namespace {
void updateForceRun(PlayerCounterForceRun* counter, al::IUseNerve* nerveUser,
                    const al::LiveActor* actor, IUsePlayerCollision* collision,
                    YoshiStateHackTongueShrink* state) {
    sead::Vector3f groundNormal = {0.0f, 0.0f, 0.0f};
    rs::calcGroundNormalOrGravityDir(&groundNormal, actor, collision);

    const f32 moveSpeed = al::calcSpeedExceptDir(actor, groundNormal);
    const f32 forceRunSpeed = counter->isForceRun() ? counter->getSpeed() : 0.0f;
    const f32 speed = moveSpeed > forceRunSpeed ? moveSpeed : forceRunSpeed;

    if (al::isNerve(nerveUser, &NrvYoshiStateHackPlay.TongueShrink) &&
        state->isEnableAccelForceRun()) {
        const bool isOnGround = rs::isOnGround(actor, collision);
        f32 accel = speed > 30.0f ? 0.2f : 0.5f;
        if (!isOnGround)
            accel = -0.5f;

        const f32 speedMax = static_cast<f32>(state->getLoopRunCount()) * 3.0f + 30.0f;
        const f32 nextSpeed = speed + accel;
        f32 resultSpeed = 15.0f;
        if (nextSpeed < 15.0f) {
        } else {
            resultSpeed = nextSpeed;
            if (nextSpeed > speedMax)
                resultSpeed = speedMax;
        }
        counter->setupForceRun(2, resultSpeed);
    } else if (counter->isForceRun()) {
        const f32 nextSpeed = speed - 0.5f;
        f32 resultSpeed = 15.0f;
        if (nextSpeed < 15.0f) {
        } else {
            resultSpeed = nextSpeed;
            if (nextSpeed > 30.0f)
                resultSpeed = 30.0f;
        }
        counter->setSpeed(resultSpeed);
    }
}
}  // namespace

void YoshiStateHackPlay::exeFall() {
    if (al::updateNerveState(this)) {
        setNerveOnGround();
        return;
    }

    YoshiActionTongueAttack* tongueAttack = mTongueAttack;
    sead::Vector3f up = -al::getGravity(mActor);
    tongueAttack->tryStartAttack(up);
    tryChangeNerveAir();
}

// NON_MATCHING: exact-size 184; first ongoing-path difference is load/zero-store scheduling around target 0x49B114: target loads actor/collision before zeroing groundNormal while current source zeros before those member loads. Next recover the original vector-initialization/argument-evaluation shape that schedules those loads without hoisting them across updateNerveState().
void YoshiStateHackPlay::exeRun() {
    if (al::updateNerveState(this)) {
        setNerveOnGround();
        return;
    }

    sead::Vector3f groundNormal = {0.0f, 0.0f, 0.0f};
    YoshiActionTongueAttack* tongueAttack = mTongueAttack;
    rs::calcGroundNormalOrUpDir(&groundNormal, mActor, mCollision);
    tongueAttack->tryStartAttackGround(groundNormal);
    tryChangeNerveOnGround();
}

void YoshiStateHackPlay::exeDown() {
    if (al::updateNerveState(this)) {
        if (rs::isOnGroundRunAngle(mActor, mCollision, mConst)) {
            if (rs::updateJudgeAndResult(mJudgeStartRun))
                al::setNerve(this, &NrvYoshiStateHackPlay.Run);
            else
                al::setNerve(this, &NrvYoshiStateHackPlay.Wait);
        } else {
            al::setNerve(this, &NrvYoshiStateHackPlay.Fall);
        }
        return;
    }

    if (mStateDown->isLand() && rs::updateJudgeAndResult(mJudgeFallFromGround)) {
        al::setNerve(this, &NrvYoshiStateHackPlay.Fall);
        return;
    }

    if (mStateDown->isEnableCancel() && rs::updateJudgeAndResult(mJudgeStartJump))
        al::setNerve(this, &NrvYoshiStateHackPlay.Jump);
}

bool YoshiStateHackPlay::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (isDead())
        return false;

    const al::LiveActor* actor = mActor;
    if (al::isSensorNpc(self) && rs::sendMsgPlayerItemGetAll(other, self))
        return true;

    if (al::isSensorPlayerEye(self)) {
        if (mEyeSensorHitHolder->isEnableRecordLookAt(other, self)) {
            mEyeSensorHitHolder->tryRecordLookAtTarget(other, self);
            if (!al::isSensorLookAt(other)) {
                mEyeSensorHitHolder->tryRecordHomingAttack(other, self);
                mEyeSensorHitHolder->tryRecordEatTarget(other, self);
            }
        }
        return true;
    }

    if (al::isSensorNpc(self) && rs::sendMsgYoshiDirectEat(other, self)) {
        mTongueAttack->tryEatDirect();
        return true;
    }

    if (al::isSensorPlayerFoot(self) && rs::isEnableSendTrampleMsg(actor, self, other) &&
        (al::sendMsgPlayerAttackTrample(other, self, nullptr) ||
         al::sendMsgPlayerTrampleReflect(other, self, nullptr))) {
        mJumpMessageRequest->jumpType = static_cast<PlayerJumpType>(4);
        al::setNerve(this, &NrvYoshiStateHackPlay.Jump);
        return true;
    }

    if (al::isSensorNpc(self) && rs::isEnableSendUpperPunchMsg(actor, self, other) &&
        al::getGravity(actor).dot(al::getVelocity(actor)) < 0.0f &&
        al::sendMsgPlayerObjUpperPunch(other, self)) {
        mTrigger->set(PlayerTrigger::EAttackSensorTrigger_val4);
        return true;
    }

    if (!al::isSensorPlayerFoot(self) || !al::sendMsgPlayerKick(other, self))
        return false;
    if (!mAnimator->isSubAnimPlaying())
        mAnimator->startSubAnim("Kick");
    return true;
}

bool YoshiStateHackPlay::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                                    al::HitSensor* self) {
    if (rs::isSensorTypeYoshiMsgReceivable(self) && rs::isMsgHackerDamageAndCancel(message))
        return rs::requestDamage(*mPlayerHack);
    return false;
}

YoshiStateHackPlay::~YoshiStateHackPlay() = default;

