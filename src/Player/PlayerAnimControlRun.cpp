#include "Player/PlayerAnimControlRun.h"

#include <prim/seadSafeString.h>

#include "Library/Math/MathUtil.h"

#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerEffect.h"
#include "Util/JudgeUtil.h"

PlayerAnimControlRun::PlayerAnimControlRun(PlayerAnimator* animator, const PlayerConst* playerConst,
                                           const IJudge* judge, PlayerEffect* effect, bool isMoon)
    : mAnimator(animator), mEffect(effect), mPlayerConst(playerConst), mJudge(judge),
      mIsRunStart(false), mRunStartBlendRate(0.0f), mPrevSpeed(0.0f), mBlendWeight(0.0f),
      mIsMoon(isMoon), mMoveAnimName(isMoon ? "MoveMoon" : "Move") {}

void PlayerAnimControlRun::reset(f32 speed, bool forceReset) {
    mIsRunStart = !forceReset && mPlayerConst->getNormalMaxSpeed() * 0.9f > speed;
    mRunStartBlendRate = 0.0f;
    mPrevSpeed = 0.0f;
    mBlendWeight = 0.0f;
}

bool PlayerAnimControlRun::isAnimDashFast() const {
    PlayerAnimator* animator = mAnimator;
    const sead::SafeString anim(mMoveAnimName);
    return animator->isAnim(anim) && mAnimator->getBlendWeight(3) > 0.5f;
}

// NON_MATCHING: first faithful source attempt from corpus; exact instruction ordering pending.
void PlayerAnimControlRun::update(f32 speed, const sead::Vector3f& velocity) {
    const f32 runStartFrameMax =
        mAnimator->getRunStartAnimFrameMax() * mPlayerConst->getRunStartPlayFrameScale();
    const s32 runStartFrameMaxInt = runStartFrameMax;
    const f32 velocityLength = velocity.length();

    if (velocityLength >= 0.95f && mIsRunStart && mPlayerConst->getDashJudgeSpeed() > speed &&
        mRunStartBlendRate < runStartFrameMaxInt) {
        mPrevSpeed = al::converge(mPrevSpeed, mPlayerConst->getRunStartBlendFrame(), 1);
    } else {
        mPrevSpeed = al::converge(mPrevSpeed, 0, 1);
    }

    mRunStartBlendRate =
        mRunStartBlendRate + 1 <= runStartFrameMaxInt ? mRunStartBlendRate + 1 : runStartFrameMaxInt;

    const sead::SafeString moveAnim(mMoveAnimName);
    if (!mAnimator->isAnim(moveAnim))
        mAnimator->startAnim(moveAnim);
    if (!mAnimator->isAnim(moveAnim))
        return;

    const f32 runStartAnimRate = mPlayerConst->getAnimFrameRateRunStart();
    const f32 runStartBlendRate =
        al::calcRate01((f32)mPrevSpeed, 0.0f, (f32)mPlayerConst->getRunStartBlendFrame());

    bool updateRunEffect = false;
    if (runStartBlendRate > 0.5f) {
        mEffect->tryStartRunEffectRunStart();
    } else if (mJudge && rs::isJudge(mJudge)) {
        mEffect->tryStartRunEffectDashWaterSurface();
    } else {
        updateRunEffect = true;
    }

    const f32 moveWeight = 1.0f - runStartBlendRate;
    const f32 dashFastBorder = mPlayerConst->getDashFastBorderSpeed();
    const f32 dashFastBlendRange = mPlayerConst->getDashFastBlendRange();
    const f32 dashBorder = mPlayerConst->getDashBorderSpeed();
    const f32 dashBlendRange = mPlayerConst->getDashBlendRange();
    const f32 runBorder = mPlayerConst->getRunBorderSpeed();
    const f32 runBlendRange = mPlayerConst->getRunBlendRange();

    if (dashFastBorder - dashFastBlendRange < speed) {
        mBlendWeight = 0.0f;
        f32 blend = 0.0f;
        if (!al::isNearZero(dashFastBlendRange, 0.001f)) {
            blend = (dashFastBlendRange + speed - dashFastBorder) / (dashFastBlendRange * 2.0f);
            blend = sead::Mathf::clamp(blend, 0.0f, 1.0f);
        }
        mAnimator->setBlendWeight(mBlendWeight, 0.0f, moveWeight * (1.0f - blend),
                                  moveWeight * blend, runStartBlendRate, 0.0f);
        const f32 rate = al::lerpValue(
            al::lerpValue(mPlayerConst->getAnimFrameRateMaxDash(),
                          mPlayerConst->getAnimFrameRateMaxDashFast(),
                          al::calcRate01(speed, dashFastBorder - dashFastBlendRange,
                                         mPlayerConst->getAnimFrameRateSpeedMax())),
            runStartAnimRate, runStartBlendRate);
        mAnimator->setAnimRate(rate);
        if (updateRunEffect)
            mEffect->tryStartRunEffectDashFast();
    } else if (velocityLength >= 0.95f || dashBorder - dashBlendRange < speed) {
        mBlendWeight = 0.0f;
        f32 blend = 0.0f;
        if (!al::isNearZero(dashBlendRange, 0.001f)) {
            blend = (dashBlendRange + speed - dashBorder) / (dashBlendRange * 2.0f);
            blend = sead::Mathf::clamp(blend, 0.0f, 1.0f);
        }
        mAnimator->setBlendWeight(mBlendWeight, moveWeight * (1.0f - blend), moveWeight * blend,
                                  0.0f, runStartBlendRate, 0.0f);
        const f32 rate = al::lerpValue(
            al::lerpValue(mPlayerConst->getAnimFrameRateMaxRun(),
                          mPlayerConst->getAnimFrameRateMaxDash(),
                          al::calcRate01(speed, dashBorder - dashBlendRange,
                                         dashFastBorder - dashFastBlendRange)),
            runStartAnimRate, runStartBlendRate);
        mAnimator->setAnimRate(rate);
        if (updateRunEffect) {
            if (blend > 0.5f)
                mEffect->tryStartRunEffectDash();
            else
                mEffect->tryStartRunEffectRun();
        }
    } else if (runBorder - runBlendRange < speed) {
        f32 blend = 0.0f;
        if (!al::isNearZero(runBlendRange, 0.001f)) {
            blend = (runBlendRange + speed - runBorder) / (runBlendRange * 2.0f);
            blend = sead::Mathf::clamp(blend, 0.0f, 1.0f);
        }
        mBlendWeight = moveWeight * (1.0f - blend);
        mAnimator->setBlendWeight(mBlendWeight, moveWeight * blend, 0.0f, 0.0f,
                                  runStartBlendRate, 0.0f);
        const f32 rate = al::lerpValue(
            al::lerpValue(mPlayerConst->getAnimFrameRateMinRun(),
                          mPlayerConst->getAnimFrameRateMaxRun(),
                          al::calcRate01(speed, runBorder - runBlendRange,
                                         dashBorder - dashBlendRange)),
            runStartAnimRate, runStartBlendRate);
        mAnimator->setAnimRate(rate);
        if (updateRunEffect) {
            if (blend > 0.5f)
                mEffect->tryStartRunEffectRun();
            else
                mEffect->clearRunEffect();
        }
    } else {
        mBlendWeight = moveWeight;
        mAnimator->setBlendWeight(mBlendWeight, 0.0f, 0.0f, 0.0f, runStartBlendRate, 0.0f);
        const f32 rate = al::lerpValue(
            al::lerpValue(1.0f, mPlayerConst->getAnimFrameRateMinRun(),
                          al::calcRate01(speed, mPlayerConst->getAnimFrameRateSpeedMin(),
                                         runBorder - runBlendRange)),
            runStartAnimRate, runStartBlendRate);
        mAnimator->setAnimRate(rate);
        if (updateRunEffect)
            mEffect->clearRunEffect();
    }

    mAnimator->recordRunStartAnimRate(runStartBlendRate);
}
