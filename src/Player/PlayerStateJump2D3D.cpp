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

PlayerStateJump2D3D::PlayerStateJump2D3D(al::LiveActor* player, const PlayerConst* pConst,
                                         const IUsePlayerCollision* collision,
                                         const IPlayerModelChanger* modelChanger,
                                         const IUseDimension* dimension,
                                         const PlayerInput* input, PlayerAnimator* animator)
    : al::ActorStateBase("ジャンプ", player), mConst(pConst), mCollision(collision),
      mModelChanger(modelChanger), mDimension(dimension), mAnimator(animator),
      mContinuousJump(nullptr), mAirMoveControl(nullptr), mTrigger(nullptr), mJumpPower(0.0f),
      mIsJumpPowerOverridden(false), mOverrideJumpPower(0.0f), mContinuousJumpCount(0),
      mIsCanCountContinuousJump(false), mCounterCapCatch(nullptr) {
    mAirMoveControl = new PlayerActionAirMoveControl(player, pConst, input, collision, false);
    initNerve(&Jump, 0);
}

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

// NON_MATCHING: the continuous-jump prefix now follows target control flow, but Clang inlines calcJumpPowerMax where the target calls it out-of-line; next source-level hypothesis is a caller/helper source shape that changes the natural inlining decision without attributes.
void PlayerStateJump2D3D::appear() {
    PlayerContinuousJump* continuousJump = mContinuousJump;
    al::LiveActor* actor = mActor;
    const PlayerConst* pConst = mConst;
    bool isJumpPowerOverridden = mIsJumpPowerOverridden;
    bool isMini = mModelChanger->isMini();
    bool canCountContinuousJump = false;
    mContinuousJumpCount = 0;
    if (continuousJump) {
        if (isJumpPowerOverridden || isMini ||
            (continuousJump->mCount >= 2 &&
             al::calcSpeedH(actor) < pConst->getDashMaxSpeed2D()))
            continuousJump->clear();
        else
            mContinuousJumpCount = continuousJump->mCount;
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

// NON_MATCHING: exact size; target compares 2 then 1 then CBNZ, while current switch hoists the zero case first; next source-level hypothesis is a selector shape that keeps zero/default after the shared 1/2 case.
f32 PlayerStateJump2D3D::calcJumpPowerMin() const {
    if (mModelChanger->is2DModel() && rs::isIn2DArea(mDimension))
        return mConst->getJumpPowerMin2DArea();

    if (mCounterCapCatch && mCounterCapCatch->isCapCatch())
        return mConst->getJumpPowerCapCatch();

    switch (mContinuousJumpCount) {
    case 1:
        return mConst->getContinuousJumpPowerMin();
    case 2:
        return mConst->getContinuousJumpPowerMin();
    case 0:
        return mConst->getJumpPowerMin();
    default:
        return mConst->getJumpPowerMax();
    }
}

// NON_MATCHING: exact size and 2-then-1 comparison order; second comparison currently falls through to the 1-case instead of branching to it; next source-level hypothesis is a branch-local selector shape that places default before the 2/1 getter blocks.
f32 PlayerStateJump2D3D::calcJumpPowerMax() const {
    if (mModelChanger->is2DModel() && rs::isIn2DArea(mDimension))
        return mConst->getJumpPowerMax2DArea();

    if (mCounterCapCatch && mCounterCapCatch->isCapCatch())
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

// NON_MATCHING: first-step setup, upper-punch reaction, ceiling response, movement update, and landing termination are recovered; helper lowering and actor register lifetime differ; next source-level hypothesis is preserving branch-local actor/member pointers in the corpus call order.
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

// NON_MATCHING: exact size and 2-then-1 comparison order; second comparison currently falls through to the 1-case instead of branching to it; next source-level hypothesis is a branch-local selector shape that places default before the 2/1 getter blocks.
f32 PlayerStateJump2D3D::calcJumpGravity() const {
    if (mCounterCapCatch && mCounterCapCatch->isCapCatch())
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

PlayerStateJump2D3D::~PlayerStateJump2D3D() = default;
