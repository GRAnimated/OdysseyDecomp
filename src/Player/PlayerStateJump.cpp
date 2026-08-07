#include "Player/PlayerStateJump.h"

#include "Library/Base/StringUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"

#include "Player/IPlayerModelChanger.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerCounterForceRun.h"
#include "Util/ActorDimensionUtil.h"
#include "Util/ObjUtil.h"

f32 PlayerStateJump::calcJumpPowerBorderSpeedMin() const {
    const IUseDimension* dimension = mDimension;
    bool use2DBorder = mModelChanger->is2DModel();
    if (use2DBorder)
        use2DBorder = rs::isIn2DArea(dimension);
    if (use2DBorder)
        return mConst->getJumpPowerMinBorder2D();
    return mConst->getNormalMinSpeed();
}

f32 PlayerStateJump::calcJumpPowerBorderSpeedMax() const {
    const IUseDimension* dimension = mDimension;
    bool use2DBorder = mModelChanger->is2DModel();
    if (use2DBorder)
        use2DBorder = rs::isIn2DArea(dimension);
    if (use2DBorder)
        return mConst->getJumpPowerMaxBorder2D();
    return mConst->getNormalMaxSpeed();
}

// NON_MATCHING: behavior is recovered; switch lowering and shared epilogue placement differ.
f32 PlayerStateJump::calcJumpPowerMin() const {
    const IUseDimension* dimension = mDimension;
    if (mModelChanger->is2DModel() && rs::isIn2DArea(dimension))
        return mConst->getJumpPowerMin2DArea();
    if (mCounterForceRun->isForceRun())
        return mConst->getJumpPowerForceRun();

    switch (_b0) {
    case 2:
    case 1:
        return mConst->getContinuousJumpPowerMin();
    case 0:
        return mConst->getJumpPowerMin();
    default:
        return mConst->getJumpPowerMax();
    }
}

// NON_MATCHING: behavior is recovered; branch-target layout places the shared epilogue too early.
f32 PlayerStateJump::calcJumpPowerMax() const {
    const IUseDimension* dimension = mDimension;
    if (mModelChanger->is2DModel() && rs::isIn2DArea(dimension))
        return mConst->getJumpPowerMax2DArea();
    if (mCounterForceRun->isForceRun())
        return mConst->getJumpPowerForceRun();

    switch (_b0) {
    case 2:
        return mConst->getJumpPowerMax3rd();
    case 1:
        return mConst->getJumpPowerMax2nd();
    default:
        return mConst->getJumpPowerMax();
    }
}

// NON_MATCHING: behavior is recovered; the compiler hoists the PlayerConst load before the second mode comparison.
f32 PlayerStateJump::calcJumpGravity() const {
    if (mCounterForceRun->isForceRun())
        return mConst->getJumpGravityForceRun();

    if (_b0 != 2) {
        if (_b0 == 1)
            return mConst->getJumpGravity2nd();
        return mConst->getJumpGravity();
    }
    return mConst->getJumpGravity3rd();
}



// NON_MATCHING: strings and decisions are recovered; mode-branch and final conditional-select lowering differ.
const char* PlayerStateJump::calcJumpAnimName() const {
    if (mModelChanger->is2DModel())
        return _d0 ? _d0 : "Jump";

    if (_c8)
        return _c8;

    if (_b0 == 2)
        return "Jump3";
    if (_b0 == 1)
        return "Jump2";
    if (_b0 != 0)
        return "Jump";

    const bool isSubAnimPlaying = mAnimator->isSubAnimPlaying();
    const bool isFront = rs::isPlayerSideFaceToCameraZ(mActor);
    if (isSubAnimPlaying)
        return isFront ? "JumpInterp" : "JumpReverseInterp";
    return isFront ? "Jump" : "JumpReverse";
}

bool PlayerStateJump::trySubAnimJumpReaction() {
    PlayerAnimator* animator = mAnimator;
    if (!animator->isSubAnimPlaying())
        return false;
    if (_b8) {
        animator->endSubAnim();
        return false;
    }
    al::startHitReaction(mActor, "アクションジャンプ");
    return true;
}

bool PlayerStateJump::isJumpSpinGroundClockwise() const {
    return al::isEqualString(_c8, "SpinJumpR");
}
