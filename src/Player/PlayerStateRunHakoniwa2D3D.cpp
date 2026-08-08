#include "Player/PlayerStateRunHakoniwa2D3D.h"

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerConst.h"
#include "Player/PlayerCounterForceRun.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerJudgeStartGroundSpin.h"
#include "Player/PlayerJudgeWaterSurfaceRun.h"
#include "Player/PlayerStateGroundSpin.h"
#include "Player/PlayerStateRun2D.h"
#include "Player/PlayerStateRunHakoniwa.h"
#include "Util/ActorDimensionUtil.h"
#include "Util/JudgeUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(PlayerStateRunHakoniwa2D3D, Run3D)
NERVE_IMPL(PlayerStateRunHakoniwa2D3D, GroundSpin3D)
NERVE_IMPL(PlayerStateRunHakoniwa2D3D, Run2D)
NERVES_MAKE_STRUCT(PlayerStateRunHakoniwa2D3D, Run3D, GroundSpin3D, Run2D)
}  // namespace

// NON_MATCHING: target/current are 0x1f4 bytes; strings, nerves, and owned-state wiring are
// recovered but initializer ordering gives different parameter/register lifetimes; next
// source-level hypothesis is to group owned-state construction in target call order before member stores.
PlayerStateRunHakoniwa2D3D::PlayerStateRunHakoniwa2D3D(
    al::LiveActor* player, const PlayerConst* pConst, const IUseDimension* dimension,
    const PlayerInput* input, const IUsePlayerCollision* collision,
    const al::WaterSurfaceFinder* waterSurfaceFinder,
    const PlayerCounterForceRun* counterForceRun,
    const PlayerCounterQuickTurnJump* counterQuickTurnJump, PlayerTrigger* trigger,
    PlayerAnimator* animator, PlayerEffect* effect,
    PlayerJointParamCenterDynamics* centerDynamics, bool isMoon)
    : al::ActorStateBase("走り[2D3D]", player), mConst(pConst), mInput(input),
      mDimension(dimension), mCollision(collision), mWaterSurfaceFinder(waterSurfaceFinder),
      mCounterForceRun(counterForceRun), mRun3D(nullptr), mGroundSpin3D(nullptr), mRun2D(nullptr),
      mJudgeStartGroundSpin(nullptr), mJudgeWaterSurfaceRun(nullptr) {
    mJudgeStartGroundSpin = new PlayerJudgeStartGroundSpin(player, collision, input);
    mJudgeWaterSurfaceRun =
        new PlayerJudgeWaterSurfaceRun(player, pConst, waterSurfaceFinder, counterForceRun);
    initNerve(&NrvPlayerStateRunHakoniwa2D3D.Run3D, 3);
    mRun3D = new PlayerStateRunHakoniwa(player, pConst, input, collision, counterForceRun,
                                        counterQuickTurnJump, trigger, animator, effect,
                                        centerDynamics, mJudgeWaterSurfaceRun, isMoon);
    mGroundSpin3D = new PlayerStateGroundSpin(player, collision, input, pConst, animator);
    mRun2D = new PlayerStateRun2D(player, pConst, input, collision, animator);
    al::initNerveState(this, mRun3D, &NrvPlayerStateRunHakoniwa2D3D.Run3D, "走り3D");
    al::initNerveState(this, mGroundSpin3D, &NrvPlayerStateRunHakoniwa2D3D.GroundSpin3D,
                       "地上スピン");
    al::initNerveState(this, mRun2D, &NrvPlayerStateRunHakoniwa2D3D.Run2D, "走り2D");
}

void PlayerStateRunHakoniwa2D3D::appear() {
    al::ActorStateBase::appear();
    mJudgeWaterSurfaceRun->mIsEnable = mCounterForceRun->getCounter() > 0;
    if (rs::is2D(mDimension))
        al::setNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run2D);
    else if (rs::updateJudgeAndResult(mJudgeStartGroundSpin))
        al::setNerve(this, &NrvPlayerStateRunHakoniwa2D3D.GroundSpin3D);
    else
        al::setNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run3D);
}

bool PlayerStateRunHakoniwa2D3D::update() {
    if (!mJudgeWaterSurfaceRun->mIsEnable && mCounterForceRun->getCounter() >= 1)
        mJudgeWaterSurfaceRun->mIsEnable = true;
    rs::updateJudge(mJudgeWaterSurfaceRun);

    if (al::isNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run2D)) {
        if (al::isNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run2D) &&
            !rs::is2D(mDimension)) {
            if (rs::updateJudgeAndResult(mJudgeStartGroundSpin))
                al::setNerve(this, &NrvPlayerStateRunHakoniwa2D3D.GroundSpin3D);
            else
                al::setNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run3D);
        }
    } else if (rs::is2D(mDimension)) {
        al::setNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run2D);
    }
    return al::NerveStateBase::update();
}

bool PlayerStateRunHakoniwa2D3D::isEnableLookAt() const {
    return al::isNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run3D) && mRun3D->isEnableLookAt();
}

bool PlayerStateRunHakoniwa2D3D::isGroundSpin() const {
    return al::isNerve(this, &NrvPlayerStateRunHakoniwa2D3D.GroundSpin3D);
}

bool PlayerStateRunHakoniwa2D3D::isSpinClockwise() const {
    return mGroundSpin3D->isSpinClockwise();
}

bool PlayerStateRunHakoniwa2D3D::isRunDashFast() const {
    return al::isNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run3D) && mRun3D->isRunDashFast();
}

bool PlayerStateRunHakoniwa2D3D::isRunWaterSurface() const {
    return al::isNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run3D) && rs::isJudge(mJudgeWaterSurfaceRun);
}

bool PlayerStateRunHakoniwa2D3D::isBrake2D() const {
    return al::isNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run2D) && mRun2D->isBrake();
}

bool PlayerStateRunHakoniwa2D3D::tryTurnJump(IJudge* judge, sead::Vector3f* direction) {
    return al::isNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run3D) && rs::isJudge(judge) && mRun3D->tryTurnJump(direction);
}

f32 PlayerStateRunHakoniwa2D3D::getTurnTiltRate() const {
    return al::isNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run3D) ? mRun3D->getTurnTiltRate() : 0.0f;
}

f32 PlayerStateRunHakoniwa2D3D::getCenterTiltRate() const {
    return al::isNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run3D) ? mRun3D->getCenterTiltRateMax() : 0.0f;
}

f32 PlayerStateRunHakoniwa2D3D::getInverseKinematicsRate() const {
    return al::isNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run3D) ? mRun3D->getInverseKinematicsRate() : 0.0f;
}

f32 PlayerStateRunHakoniwa2D3D::getCapDynamicsRate() const {
    return al::isNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run3D) ? mRun3D->getCapDynamicsRate() : 0.0f;
}

void PlayerStateRunHakoniwa2D3D::exeRun3D() {
    if (al::updateNerveState(this)) {
        kill();
    } else if (rs::isJudge(mJudgeWaterSurfaceRun)) {
        f32 approachRate = mConst->getSwimRunSurfaceApproachRate();
        if (mCounterForceRun->getCounter() <= 0) {
            f32 speedH = al::calcSpeedH(mActor);
            f32 brakeRate = al::easeOut(al::calcRate01(
                speedH, mJudgeWaterSurfaceRun->getBorderSpeedH(),
                mConst->getSwimRunSurfaceBrakeBorder()));
            f32 approachRateScale = al::easeOut(
                al::calcRate01(speedH, mConst->getSwimRunSurfaceApproachBorderMin(),
                               mConst->getSwimRunSurfaceApproachBorderMax()));
            al::LiveActor* actor = mActor;
            f32 velocityScale =
                al::lerpValue(mConst->getSwimRunSurfaceBrakeH(), 1.0f, brakeRate);
            al::scaleVelocityHV(actor, velocityScale, 1.0f);
            approachRate = al::lerpValue(mConst->getSwimRunSurfaceApproachRateMin(), approachRate,
                                         approachRateScale);
        }
        al::approachWaterSurfaceRate(mActor, mWaterSurfaceFinder,
                                     mConst->getSwimRunSurfaceBaseHeight(),
                                     mConst->getSwimRunSurfaceApproachLimit(), approachRate);
    } else if (rs::updateJudgeAndResult(mJudgeStartGroundSpin)) {
        al::setNerve(this, &NrvPlayerStateRunHakoniwa2D3D.GroundSpin3D);
    }
}

void PlayerStateRunHakoniwa2D3D::exeGroundSpin3D() {
    if (!al::updateNerveState(this))
        return;

    if (mInput->isMove())
        al::setNerve(this, &NrvPlayerStateRunHakoniwa2D3D.Run3D);
    else
        kill();
}

void PlayerStateRunHakoniwa2D3D::exeRun2D() {
    bool isStateEnd = al::updateNerveState(this);
    if (rs::isCollidedGround(mCollision)) {
        sead::Vector3f up = {0.0f, 0.0f, 0.0f};
        al::calcUpDir(&up, mActor);
        sead::Vector3f lockDirection = {0.0f, 0.0f, 0.0f};
        rs::calcLockDirection(&lockDirection, mDimension);
        al::verticalizeVec(&up, lockDirection, up);
        if (al::tryNormalizeOrZero(&up)) {
            sead::Vector3f front = {0.0f, 0.0f, 0.0f};
            al::calcFrontDir(&front, mActor);
            sead::Quatf quat = sead::Quatf::unit;
            al::makeQuatUpFront(&quat, up, front);
            al::updatePoseQuat(mActor, quat);
        }
    }

    if (isStateEnd)
        kill();
}

PlayerStateRunHakoniwa2D3D::~PlayerStateRunHakoniwa2D3D() = default;
