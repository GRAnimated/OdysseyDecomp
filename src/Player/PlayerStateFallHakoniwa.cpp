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
PlayerStateFallHakoniwa::PlayerStateFallHakoniwa(
    al::LiveActor* player, const PlayerConst* pConst, const IUsePlayerCollision* collision,
    const PlayerInput* input, const PlayerTrigger* trigger, const PlayerAreaChecker* areaChecker,
    const IUseDimension* dimension, PlayerAnimator* animator)
    : al::ActorStateBase("落下", player), mConst(pConst), mCollision(collision), mTrigger(trigger),
      mDimension(dimension), mAnimator(animator) {
    mJudgeInvalidateInputFall =
        new PlayerJudgeInvalidateInputFall(player, areaChecker, collision);
    mAirMoveControl = new PlayerActionAirMoveControl(player, pConst, input, mCollision, false);
    mAirMoveControl->setupCollideWallScaleVelocity(mConst->getFallWallScaleVelocity(), 0.0f,
                                                   mConst->getNormalMaxSpeed());
    initNerve(&NrvPlayerStateFallHakoniwa.Fall);
}

void PlayerStateFallHakoniwa::appear() {
    al::NerveStateBase::appear();
    _58 = 0;
    _5c = 0;
    mWallCatchReferenceFront.set(0.0f, 0.0f, 0.0f);
    mIsInvalidInputFall = false;
    mIsRunFall = false;

    if (mTrigger) {
        mIsInvalidInputFall = mTrigger->isOn(PlayerTrigger::ECollisionTrigger_val7);
        mIsRunFall = mTrigger->isOn(PlayerTrigger::EActionTrigger_val14);
        if (mTrigger->isOn(PlayerTrigger::EActionTrigger_val32)) {
            _58 = 8;
            _5c = 15;
            al::calcFrontDir(&mWallCatchReferenceFront, mActor);
        }
    }

    const f32 speed = al::calcSpeedH(mActor);
    mFallSpeed = sead::Mathf::clamp(speed, mConst->getNormalMaxSpeed(),
                                    mConst->getJumpMoveSpeedMax());
    mFallTargetArea = nullptr;
    updateFallTargetArea();
    mAirMoveControl->setIsPlayer2D(rs::isPlayer2D(mActor));
    al::setNerve(this, &NrvPlayerStateFallHakoniwa.Fall);
}

// NON_MATCHING: exact 580/580 size with all 10 semantic calls aligned; first remaining diff is FP
// register/scheduling after separateVectorParallelVertical (target keeps direction.z in S7 and
// materializes 5.0f later). Next test a natural scalar/vector lifetime around the 0.75f/5.0f selection.
void PlayerStateFallHakoniwa::updateFallTargetArea() {
    if (!mFallTargetArea) {
        if (!rs::updateJudgeAndResult(mJudgeInvalidateInputFall))
            return;
        mFallTargetArea = mJudgeInvalidateInputFall->getArea();
    }

    al::LiveActor* actor = mActor;
    const sead::Matrix34f& areaMtx = al::getAreaObjBaseMtx(mFallTargetArea);
    sead::Vector3f areaUp;
    areaUp.set(0.0f, 0.0f, 0.0f);
    sead::Vector3f areaPos;
    areaPos.set(0.0f, 0.0f, 0.0f);
    areaMtx.getBase(areaUp, 1);
    areaMtx.getTranslation(areaPos);

    sead::Vector3f toArea;
    toArea.set(0.0f, 0.0f, 0.0f);
    toArea.setSub(areaPos, al::getTrans(actor));
    al::verticalizeVec(&toArea, areaUp, toArea);
    const f32 distance = toArea.length();

    sead::Vector3f velocityH;
    velocityH.set(0.0f, 0.0f, 0.0f);
    sead::Vector3f velocityV;
    velocityV.set(0.0f, 0.0f, 0.0f);
    al::separateVectorHV(&velocityH, &velocityV, areaUp, al::getVelocity(actor));

    sead::Vector3f direction;
    direction.set(0.0f, 0.0f, 0.0f);
    if (al::tryNormalizeOrZero(&direction, toArea)) {
        sead::Vector3f parallel;
    parallel.set(0.0f, 0.0f, 0.0f);
        sead::Vector3f vertical;
    vertical.set(0.0f, 0.0f, 0.0f);
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
        nextSpeed = sead::Mathf::clampMax(nextSpeed, sead::Mathf::min(distance, mFallSpeed));
        velocityH = vertical + direction * nextSpeed;
    } else {
        velocityH.set(0.0f, 0.0f, 0.0f);
    }

    al::setVelocity(mActor, velocityH + velocityV);
}

bool PlayerStateFallHakoniwa::isValidWallCatch() const {
    if (isDead())
        return true;
    if (!_5c || al::isGreaterStep(this, _5c))
        return true;
    sead::Vector3f front;
    front.set(0.0f, 0.0f, 0.0f);
    al::calcFrontDir(&front, mActor);
    return front.dot(mWallCatchReferenceFront) < 0.70711f;
}

void PlayerStateFallHakoniwa::exeFall() {
    updateFallTargetArea();
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("Fall");
        PlayerActionAirMoveControl* airMoveControl = mAirMoveControl;
        const f32 moveSpeedMax = mConst->getJumpMoveSpeedMax();
        const f32 moveSpeedMin = mConst->getJumpMoveSpeedMin();
        const f32 speedV = al::calcSpeedV(mActor);
        const f32 gravity = mConst->getJumpGravity();
        airMoveControl->setup(moveSpeedMax, moveSpeedMin, 0, speedV, gravity, _58, 0.0f);
    }
    if (rs::isPlayer2D(mActor) && rs::isIn2DArea(mDimension)) {
        sead::Vector3f lockDirection;
        lockDirection.set(0.0f, 0.0f, 0.0f);
        rs::calcLockDirection(&lockDirection, mDimension);
        mAirMoveControl->verticalizeStartMoveDir(lockDirection);
    }
    mAirMoveControl->update();
}

PlayerStateFallHakoniwa::~PlayerStateFallHakoniwa() = default;

