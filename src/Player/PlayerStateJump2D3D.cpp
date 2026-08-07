#include "Player/PlayerStateJump2D3D.h"

#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/IPlayerModelChanger.h"
#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerActionFunction.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerContinuousJump.h"
#include "Player/PlayerCounterAfterCapCatch.h"
#include "Player/PlayerTrigger.h"
#include "Util/ActorDimensionUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
const char* const cJumpAnimNames[] = {"Jump", "Jump2", "Jump3"};

NERVE_IMPL(PlayerStateJump2D3D, Jump)
NERVES_MAKE_STRUCT(PlayerStateJump2D3D, Jump)
}  // namespace

PlayerStateJump2D3D::PlayerStateJump2D3D(al::LiveActor* player, const PlayerConst* pConst,
                                         const IUsePlayerCollision* collision,
                                         const IPlayerModelChanger* modelChanger,
                                         const IUseDimension* dimension, const PlayerInput* input,
                                         PlayerAnimator* animator)
    : al::ActorStateBase("ジャンプ", player), mConst(pConst), mCollision(collision),
      mModelChanger(modelChanger), mDimension(dimension), mAnimator(animator) {
    mAirMoveControl = new PlayerActionAirMoveControl(player, pConst, input, collision, false);
    initNerve(&NrvPlayerStateJump2D3D.Jump, 0);
}

PlayerStateJump2D3D::~PlayerStateJump2D3D() = default;

void PlayerStateJump2D3D::initContinuousJump(PlayerContinuousJump* continuousJump) {
    mContinuousJump = continuousJump;
}

void PlayerStateJump2D3D::initUseTriggerCheck(PlayerTrigger* trigger) {
    mTrigger = trigger;
}

void PlayerStateJump2D3D::initUseCounterCapCatch(const PlayerCounterAfterCapCatch* counter) {
    mCounterAfterCapCatch = counter;
}

// NON_MATCHING: corpus behavior is implemented; next compare continuous-jump local ordering.
void PlayerStateJump2D3D::appear() {
    mContinuousJumpCount = 0;
    mIsEnableContinuousJump = false;
    if (mContinuousJump) {
        if (mIsForceJump || mModelChanger->isMini() ||
            (mContinuousJump->getCount() >= 2 &&
             al::calcSpeedH(mActor) < mConst->getDashMaxSpeed2D())) {
            mContinuousJump->clear();
        } else {
            mContinuousJumpCount = mContinuousJump->getCount();
        }
        mIsEnableContinuousJump = true;
    }

    f32 jumpPower = PlayerActionFunction::calcJumpSpeed(
        al::calcSpeedH(mActor), calcJumpPowerBorderSpeedMin(), calcJumpPowerBorderSpeedMax(),
        calcJumpPowerMin(), calcJumpPowerMax());
    if (mIsForceJump)
        jumpPower = mForceJumpPower;
    mIsForceJump = false;
    mForceJumpPower = 0.0f;
    mJumpPower = jumpPower;
    al::NerveStateBase::appear();
    al::setNerve(this, &NrvPlayerStateJump2D3D.Jump);
}

f32 PlayerStateJump2D3D::calcJumpPowerBorderSpeedMin() const {
    if (mModelChanger->is2DModel() && rs::isIn2DArea(mDimension))
        return mConst->getNormalMaxSpeed2D();
    return mConst->getNormalMinSpeed();
}

f32 PlayerStateJump2D3D::calcJumpPowerBorderSpeedMax() const {
    if (mModelChanger->is2DModel())
        rs::isIn2DArea(mDimension);
    return mConst->getDashMaxSpeed2D();
}

// NON_MATCHING: current lowering is eight bytes smaller than the target; next preserve distinct count checks and getter calls.
f32 PlayerStateJump2D3D::calcJumpPowerMin() const {
    if (mModelChanger->is2DModel() && rs::isIn2DArea(mDimension))
        return mConst->getJumpPowerMin2DArea();
    if (mCounterAfterCapCatch && mCounterAfterCapCatch->isCapCatch())
        return mConst->getJumpPowerCapCatch();
    if (mContinuousJumpCount == 1)
        return mConst->getContinuousJumpPowerMin();
    if (mContinuousJumpCount == 2)
        return mConst->getContinuousJumpPowerMin();
    if (mContinuousJumpCount == 0)
        return mConst->getJumpPowerMin();
    return mConst->getJumpPowerMax();
}

// NON_MATCHING: exact size; first branch destination differs within the selected getter switch.
f32 PlayerStateJump2D3D::calcJumpPowerMax() const {
    if (mModelChanger->is2DModel() && rs::isIn2DArea(mDimension))
        return mConst->getJumpPowerMax2DArea();
    if (mCounterAfterCapCatch && mCounterAfterCapCatch->isCapCatch())
        return mConst->getJumpPowerCapCatch();
    switch (mContinuousJumpCount) {
    case 1:
        return mConst->getJumpPowerMax2nd();
    case 2:
        return mConst->getJumpPowerMax3rd();
    default:
        return mConst->getJumpPowerMax();
    }
}

void PlayerStateJump2D3D::kill() {
    if (isDead()) {
        mIsForceJump = false;
        mForceJumpPower = 0.0f;
    }
    al::NerveStateBase::kill();
}

// NON_MATCHING: exact size; floating comparison branches with PL instead of target GE; next test an ordered comparison source form without splitting the shared clear path.
void PlayerStateJump2D3D::tryCountUpContinuousJump(PlayerContinuousJump* continuousJump) {
    if (!mIsEnableContinuousJump || mModelChanger->is2DModel() ||
        (continuousJump->getCount() &&
         mJumpPower < mConst->getJumpPowerMax() * 0.99f)) {
        continuousJump->clear();
        return;
    }
    continuousJump->countUp(mAirMoveControl->getStartMoveDir());
}

// NON_MATCHING: full corpus behavior is implemented; next compare first-step animation/setup ordering.
void PlayerStateJump2D3D::exeJump() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        mAnimator->startAnim(calcJumpAnimName());
        mAirMoveControl->setup(mConst->getJumpMoveSpeedMin(), mConst->getNormalMaxSpeed2D(),
                               mConst->getExtendFrame(), mJumpPower, calcJumpGravity(), 0,
                               mConst->getJumpInertiaRate());
    }

    al::HitSensor* body = al::getHitSensor(actor, "Body");
    if (rs::sendPlayerCollisionUpperPunchMsg(actor, body, mCollision, mModelChanger)) {
        if (mTrigger)
            mTrigger->set(PlayerTrigger::EAttackSensorTrigger_val0);
        const sead::Vector3f& gravity = al::getGravity(actor);
        sead::Vector3f* velocity = al::getVelocityPtr(actor);
        al::verticalizeVec(velocity, gravity, *velocity);
        al::addVelocity(actor, gravity * 17.0f);
        mIsEnableContinuousJump = false;
    }

    if (rs::isCollidedCeiling(mCollision)) {
        const sead::Vector3f& gravity = al::getGravity(actor);
        sead::Vector3f* velocity = al::getVelocityPtr(actor);
        al::verticalizeVec(velocity, gravity, *velocity);
        al::addVelocity(actor, gravity * 0.0f);
        mAirMoveControl->setExtendFrame(0);
        mIsEnableContinuousJump = false;
    }

    mAirMoveControl->update();
    if (rs::isOnGround(actor, mCollision))
        kill();
}

const char* PlayerStateJump2D3D::calcJumpAnimName() const {
    if (mCounterAfterCapCatch && mCounterAfterCapCatch->isCapCatch()) {
        if (mAnimator->isSubAnimPlaying())
            mAnimator->endSubAnim();
        if (mAnimator->isUpperBodyAnimAttached())
            mAnimator->clearUpperBodyAnim();
        return "JumpCapCatch";
    }

    s32 count = mContinuousJumpCount;
    if (static_cast<u32>(count) <= 2)
        return cJumpAnimNames[count];
    return "Jump";
}

// NON_MATCHING: exact size; first branch destination differs within the selected getter switch.
f32 PlayerStateJump2D3D::calcJumpGravity() const {
    if (mCounterAfterCapCatch && mCounterAfterCapCatch->isCapCatch())
        return mConst->getJumpGravityCapCatch();
    switch (mContinuousJumpCount) {
    case 1:
        return mConst->getJumpGravity2nd();
    case 2:
        return mConst->getJumpGravity3rd();
    default:
        return mConst->getJumpGravity();
    }
}
