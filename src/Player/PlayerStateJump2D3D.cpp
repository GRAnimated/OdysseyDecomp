#include "Player/PlayerStateJump2D3D.h"

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/IPlayerModelChanger.h"
#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerActionFunction.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerContinuousJump.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerCounterAfterCapCatch.h"
#include "Player/PlayerTrigger.h"
#include "Util/ActorDimensionUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(PlayerStateJump2D3D, Jump)
NERVES_MAKE_NOSTRUCT(PlayerStateJump2D3D, Jump)
}  // namespace

// NON_MATCHING: the 0x80-byte layout and initialization are recovered; remaining differences are packed-field initialization and constructor register allocation.
PlayerStateJump2D3D::PlayerStateJump2D3D(al::LiveActor* player, const PlayerConst* pConst,
                                         const IUsePlayerCollision* collision,
                                         const IPlayerModelChanger* modelChanger,
                                         const IUseDimension* dimension,
                                         const PlayerInput* input, PlayerAnimator* animator)
    : al::ActorStateBase("ジャンプ", player), mConst(pConst), mCollision(collision),
      mModelChanger(modelChanger), mDimension(dimension), mAnimator(animator),
      mContinuousJump(nullptr), mAirMoveControl(nullptr), mTrigger(nullptr), mJumpPower(0.0f),
      mIsJumpPowerOverridden(false), _65{}, mOverrideJumpPower(0.0f), mContinuousJumpCount(0),
      mIsCanCountContinuousJump(false), _71{}, mCounterCapCatch(nullptr) {
    mAirMoveControl = new PlayerActionAirMoveControl(player, pConst, input, collision, false);
    initNerve(&Jump, 0);
}

PlayerStateJump2D3D::~PlayerStateJump2D3D() = default;

void PlayerStateJump2D3D::initContinuousJump(PlayerContinuousJump* continuousJump) {
    mContinuousJump = continuousJump;
}

void PlayerStateJump2D3D::initUseTriggerCheck(PlayerTrigger* trigger) {
    mTrigger = trigger;
}

void PlayerStateJump2D3D::initUseCounterCapCatch(
    const PlayerCounterAfterCapCatch* counterCapCatch) {
    mCounterCapCatch = counterCapCatch;
}

// NON_MATCHING: jump setup, continuous-jump handling, override cleanup, and nerve transition are recovered; helper inlining and local lifetime still differ.
void PlayerStateJump2D3D::appear() {
    PlayerContinuousJump* continuousJump = mContinuousJump;
    al::LiveActor* actor = mActor;
    const PlayerConst* pConst = mConst;
    bool isJumpPowerOverridden = mIsJumpPowerOverridden;
    bool isMini = mModelChanger->isMini();
    bool canCountContinuousJump = false;
    mContinuousJumpCount = 0;
    if (continuousJump) {
        bool shouldClearContinuousJump = true;
        if (!isJumpPowerOverridden && !isMini) {
            u32 count = continuousJump->mCount;
            if (count < 2 || al::calcSpeedH(actor) >= pConst->getDashMaxSpeed2D()) {
                mContinuousJumpCount = count;
                shouldClearContinuousJump = false;
            }
        }
        if (shouldClearContinuousJump)
            continuousJump->clear();
        canCountContinuousJump = true;
    }
    mIsCanCountContinuousJump = canCountContinuousJump;

    f32 speed = al::calcSpeedH(mActor);
    f32 borderMin = calcJumpPowerBorderSpeedMin();
    f32 borderMax = calcJumpPowerBorderSpeedMax();
    f32 jumpPowerMin = calcJumpPowerMin();
    f32 jumpPowerMax = calcJumpPowerMax();
    f32 jumpPower =
        PlayerActionFunction::calcJumpSpeed(speed, borderMin, borderMax, jumpPowerMin, jumpPowerMax);
    if (mIsJumpPowerOverridden)
        jumpPower = mOverrideJumpPower;

    mIsJumpPowerOverridden = false;
    mOverrideJumpPower = 0.0f;
    mJumpPower = jumpPower;
    al::ActorStateBase::appear();
    al::setNerve(this, &Jump);
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

// NON_MATCHING: all selector values and conditions are recovered; remaining mismatch is branch ordering/source shape.
f32 PlayerStateJump2D3D::calcJumpPowerMin() const {
    if (mModelChanger->is2DModel() && rs::isIn2DArea(mDimension))
        return mConst->getJumpPowerMin2DArea();

    if (mCounterCapCatch && mCounterCapCatch->isCapCatch())
        return mConst->getJumpPowerCapCatch();

    if (mContinuousJumpCount == 2)
        return mConst->getContinuousJumpPowerMin();
    if (mContinuousJumpCount == 1)
        return mConst->getContinuousJumpPowerMin();
    if (mContinuousJumpCount == 0)
        return mConst->getJumpPowerMin();
    return mConst->getJumpPowerMax();
}

// NON_MATCHING: all selector values and conditions are recovered; remaining mismatch is branch ordering/source shape.
f32 PlayerStateJump2D3D::calcJumpPowerMax() const {
    if (mModelChanger->is2DModel() && rs::isIn2DArea(mDimension))
        return mConst->getJumpPowerMax2DArea();

    if (mCounterCapCatch && mCounterCapCatch->isCapCatch())
        return mConst->getJumpPowerCapCatch();

    if (mContinuousJumpCount != 2) {
        if (mContinuousJumpCount != 1)
            return mConst->getJumpPowerMax();
        return mConst->getJumpPowerMax2nd();
    }
    return mConst->getJumpPowerMax3rd();
}

void PlayerStateJump2D3D::kill() {
    if (isDead()) {
        mIsJumpPowerOverridden = false;
        mOverrideJumpPower = 0.0f;
    }
    al::ActorStateBase::kill();
}

void PlayerStateJump2D3D::tryCountUpContinuousJump(PlayerContinuousJump* continuousJump) {
    if (!mIsCanCountContinuousJump || mModelChanger->is2DModel()) {
        continuousJump->clear();
        return;
    }

    if (continuousJump->mCount == 0 ||
        mJumpPower >= mConst->getJumpPowerMax() * 0.99f) {
        continuousJump->countUp(mAirMoveControl->getStartMoveDir());
        return;
    }

    continuousJump->clear();
}

// NON_MATCHING: first-step setup, upper-punch reaction, ceiling response, movement update, and landing termination are recovered; helper lowering and actor register lifetime differ.
void PlayerStateJump2D3D::exeJump() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        mAnimator->startAnim(calcJumpAnimName());
        s32 extendFrame = mConst->getExtendFrame();
        PlayerActionAirMoveControl* airMoveControl = mAirMoveControl;
        f32 moveSpeedMax = mConst->getJumpMoveSpeedMax();
        f32 normalMaxSpeed2D = mConst->getNormalMaxSpeed2D();
        f32 jumpPower = mJumpPower;
        f32 jumpGravity = calcJumpGravity();
        f32 jumpInertiaRate = mConst->getJumpInertiaRate();
        airMoveControl->setup(moveSpeedMax, normalMaxSpeed2D, extendFrame, jumpPower, jumpGravity,
                              0, jumpInertiaRate);
    }

    if (rs::sendPlayerCollisionUpperPunchMsg(actor, al::getHitSensor(actor, "Body"), mCollision,
                                             mModelChanger)) {
        if (mTrigger)
            mTrigger->set(PlayerTrigger::EActionTrigger_val0);
        const sead::Vector3f& gravity = al::getGravity(actor);
        sead::Vector3f* velocity = al::getVelocityPtr(actor);
        al::verticalizeVec(velocity, gravity, *velocity);
        al::addVelocity(actor, gravity * 17.0f);
        mIsCanCountContinuousJump = false;
    }

    if (rs::isCollidedCeiling(mCollision)) {
        const sead::Vector3f& gravity = al::getGravity(actor);
        sead::Vector3f* velocity = al::getVelocityPtr(actor);
        al::verticalizeVec(velocity, gravity, *velocity);
        al::addVelocity(actor, gravity * 0.0f);
        mAirMoveControl->setExtendFrame(0);
        mIsCanCountContinuousJump = false;
    }

    mAirMoveControl->update();
    if (rs::isOnGround(actor, mCollision))
        kill();
}

const char* PlayerStateJump2D3D::calcJumpAnimName() const {
    if (mCounterCapCatch && mCounterCapCatch->isCapCatch()) {
        if (mAnimator->isSubAnimPlaying())
            mAnimator->endSubAnim();
        if (mAnimator->isUpperBodyAnimAttached())
            mAnimator->clearUpperBodyAnim();
        return "JumpCapCatch";
    }

    static const char* const jumpAnimNames[] = {"Jump", "Jump2", "Jump3"};
    if (static_cast<u32>(mContinuousJumpCount) <= 2)
        return jumpAnimNames[mContinuousJumpCount];
    return "Jump";
}

// NON_MATCHING: gravity selector values and conditions are recovered; remaining mismatch is control-flow ordering.
f32 PlayerStateJump2D3D::calcJumpGravity() const {
    if (mCounterCapCatch && mCounterCapCatch->isCapCatch())
        return mConst->getJumpGravityCapCatch();

    if (mContinuousJumpCount != 2) {
        if (mContinuousJumpCount != 1)
            return mConst->getJumpGravity();
        return mConst->getJumpGravity2nd();
    }
    return mConst->getJumpGravity3rd();
}
