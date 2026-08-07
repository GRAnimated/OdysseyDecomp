#include "Player/PlayerStateFallHakoniwa.h"

#include <math/seadMathCalcCommon.h>

#include "Library/Area/AreaObjUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerJudgeInvalidateInputFall.h"
#include "Player/PlayerTrigger.h"
#include "Util/ActorDimensionUtil.h"
#include "Util/JudgeUtil.h"
#include "Util/PlayerUtil.h"

namespace {
NERVE_IMPL(PlayerStateFallHakoniwa, Fall)
NERVES_MAKE_STRUCT(PlayerStateFallHakoniwa, Fall)
}  // namespace

// NON_MATCHING: generated constructor is four bytes smaller than the target; next compare zero-initializer store grouping.
PlayerStateFallHakoniwa::PlayerStateFallHakoniwa(
    al::LiveActor* player, const PlayerConst* pConst, const IUsePlayerCollision* collision,
    const PlayerInput* input, const PlayerTrigger* trigger, const PlayerAreaChecker* areaChecker,
    const IUseDimension* dimension, PlayerAnimator* animator)
    : al::ActorStateBase("落下", player), mConst(pConst), mCollision(collision), mTrigger(trigger),
      mDimension(dimension), mAnimator(animator), mJudgeInvalidateInputFall(nullptr),
      mAirMoveControl(nullptr), _58(0), _5c(0), _60{}, mIsInvalidInputFall(false),
      mIsRunFall(false), _6e{}, mFallTargetArea(nullptr), mFallSpeed(0.0f), _7c{} {
    mJudgeInvalidateInputFall =
        new PlayerJudgeInvalidateInputFall(player, areaChecker, collision);
    mAirMoveControl = new PlayerActionAirMoveControl(player, pConst, input, collision, false);
    mAirMoveControl->setupCollideWallScaleVelocity(pConst->getFallSpeedMax(), 0.0f,
                                                   pConst->getJumpPowerMaxBorder2D());
    initNerve(&NrvPlayerStateFallHakoniwa.Fall, 0);
}

PlayerStateFallHakoniwa::~PlayerStateFallHakoniwa() = default;

// NON_MATCHING: corpus behavior is implemented; next compare trigger initialization and clamp lowering.
void PlayerStateFallHakoniwa::appear() {
    al::NerveStateBase::appear();
    _58 = 0;
    _5c = 0;
    _60 = {};
    mIsInvalidInputFall = false;
    mIsRunFall = false;

    if (mTrigger) {
        mIsInvalidInputFall = mTrigger->isOn(PlayerTrigger::ECollisionTrigger_val7);
        mIsRunFall = mTrigger->isOn(PlayerTrigger::EActionTrigger_val14);
        if (mTrigger->isOn(PlayerTrigger::EActionTrigger_val32)) {
            _58 = 8;
            _5c = 15;
            al::calcFrontDir(&_60, mActor);
        }
    }

    const f32 speed = al::calcSpeedH(mActor);
    mFallSpeed = sead::Mathf::clamp(speed, mConst->getJumpPowerMaxBorder2D(),
                                    mConst->getJumpBaseSpeedMax());
    mFallTargetArea = nullptr;
    updateFallTargetArea();
    mAirMoveControl->setIsPlayer2D(rs::isPlayer2D(mActor));
    al::setNerve(this, &NrvPlayerStateFallHakoniwa.Fall);
}

// NON_MATCHING: corpus vector behavior is implemented; next compare temporary ordering and min/max lowering.
void PlayerStateFallHakoniwa::updateFallTargetArea() {
    if (!mFallTargetArea) {
        if (!rs::updateJudgeAndResult(mJudgeInvalidateInputFall))
            return;
        mFallTargetArea = mJudgeInvalidateInputFall->getArea();
    }

    sead::Vector3f areaUp = {};
    sead::Vector3f areaPos = {};
    const sead::Matrix34f& areaMtx = al::getAreaObjBaseMtx(mFallTargetArea);
    areaMtx.getBase(areaUp, 1);
    areaMtx.getTranslation(areaPos);

    sead::Vector3f toArea = areaPos - al::getTrans(mActor);
    al::verticalizeVec(&toArea, areaUp, toArea);
    const f32 distance = toArea.length();

    sead::Vector3f velocityH = {};
    sead::Vector3f velocityV = {};
    al::separateVectorHV(&velocityH, &velocityV, areaUp, al::getVelocity(mActor));

    sead::Vector3f adjustedH = {};
    sead::Vector3f direction = {};
    if (al::tryNormalizeOrZero(&direction, toArea)) {
        sead::Vector3f parallel = {};
        sead::Vector3f vertical = {};
        al::separateVectorParallelVertical(&parallel, &vertical, direction, velocityH);
        const f32 towardSpeed = parallel.dot(direction);
        vertical *= 0.75f;

        f32 nextSpeed = towardSpeed * 0.75f;
        if (towardSpeed >= 0.0f) {
            nextSpeed = towardSpeed;
            if (towardSpeed * 5.0f > distance)
                nextSpeed = towardSpeed * 0.75f;
        }
        nextSpeed += 0.1f;
        nextSpeed = sead::Mathf::clampMax(nextSpeed, sead::Mathf::clampMax(mFallSpeed, distance));
        adjustedH = vertical + direction * nextSpeed;
    }

    al::setVelocity(mActor, adjustedH + velocityV);
}

// NON_MATCHING: target uses a shared true-return epilogue before the vector block; next preserve the positive validity condition.
bool PlayerStateFallHakoniwa::isValidWallCatch() const {
    if (!isDead() && _5c && !al::isGreaterStep(this, _5c)) {
        sead::Vector3f front = {};
        al::calcFrontDir(&front, mActor);
        return front.dot(_60) < 0.70711f;
    }
    return true;
}

// NON_MATCHING: corpus behavior is implemented and generated code is eight bytes smaller than the target; next compare setup argument evaluation order.
void PlayerStateFallHakoniwa::exeFall() {
    updateFallTargetArea();
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("Fall");
        PlayerActionAirMoveControl* airMoveControl = mAirMoveControl;
        const f32 baseSpeedMax = mConst->getJumpBaseSpeedMax();
        const f32 gravity = mConst->getJumpGravity();
        const f32 speedV = al::calcSpeedV(mActor);
        const f32 jumpPowerMin = mConst->getJumpPowerMin();
        airMoveControl->setup(baseSpeedMax, gravity, 0, speedV, jumpPowerMin, _58, 0.0f);
    }
    if (rs::isPlayer2D(mActor) && rs::isIn2DArea(mDimension)) {
        sead::Vector3f lockDirection = {};
        rs::calcLockDirection(&lockDirection, mDimension);
        mAirMoveControl->verticalizeStartMoveDir(lockDirection);
    }
    mAirMoveControl->update();
}
