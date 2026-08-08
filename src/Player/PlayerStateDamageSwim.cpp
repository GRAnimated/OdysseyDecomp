#include "Player/PlayerStateDamageSwim.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseKeeper.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nature/WaterSurfaceFinder.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Se/SeFunction.h"

#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerEffect.h"
#include "Player/PlayerFunction.h"
#include "Player/PlayerTrigger.h"
#include "Util/JudgeUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerUtil.h"

namespace {
NERVE_IMPL(PlayerStateDamageSwim, DamageSwim);
NERVE_IMPL(PlayerStateDamageSwim, Dead);
NERVE_IMPL(PlayerStateDamageSwim, DamageLandWater);
NERVE_IMPL(PlayerStateDamageSwim, DamageSurface);
NERVE_IMPL(PlayerStateDamageSwim, EndGround);
NERVE_IMPL(PlayerStateDamageSwim, EndOutOfWater);

NERVES_MAKE_NOSTRUCT(PlayerStateDamageSwim, DamageSwim, Dead, DamageLandWater, DamageSurface,
                     EndGround, EndOutOfWater);
}  // namespace
PlayerStateDamageSwim::PlayerStateDamageSwim(
    al::LiveActor* player, const PlayerConst* pConst, const IUsePlayerCollision* collision,
    const PlayerInput* input, const PlayerTrigger* trigger, PlayerAnimator* animator,
    const al::WaterSurfaceFinder* waterSurfaceFinder, PlayerEffect* effect, IJudge* judgeInWater,
    IJudge* judgeOutInWater)
    : al::ActorStateBase("水中・水面ダメージ", player), mConst(pConst), mCollision(collision),
      mTrigger(trigger), mAnimator(animator), mWaterSurfaceFinder(waterSurfaceFinder),
      mEffect(effect), mJudgeInWater(judgeInWater), mJudgeOutInWater(judgeOutInWater) {
    mAirMoveControl = new PlayerActionAirMoveControl(player, pConst, input, collision, true);
    initNerve(&DamageSwim, 0);
}

void PlayerStateDamageSwim::appear() {
    al::ActorStateBase::appear();
    _68 = false;
    _69 = false;
    mIsNoDamageDown = mTrigger->isOn(PlayerTrigger::EActionTrigger_val10);

    if (PlayerFunction::isPlayerDeadStatus(mActor)) {
        al::setVelocityZero(mActor);
        _68 = mTrigger->isOn(PlayerTrigger::EActionTrigger_val7);
        _69 = mTrigger->isOn(PlayerTrigger::EActionTrigger_val4);
        al::setNerve(this, &Dead);
        return;
    }

    if (mTrigger->isOn(PlayerTrigger::EActionTrigger_val5)) {
        tryReactionWaterIn();
        al::LiveActor* player = mActor;
        const sead::Vector3f& gravity = al::getGravity(player);
        f32 velocityGravity = gravity.dot(al::getVelocity(player));
        if (velocityGravity < 0.0f)
            *al::getVelocityPtr(player) -= gravity * (velocityGravity + velocityGravity);
        al::setNerve(this, &DamageLandWater);
        return;
    }

    if (mWaterSurfaceFinder->isFoundSurface() &&
        mWaterSurfaceFinder->getDistance() < mConst->getSwimSurfaceStartDist()) {
        sead::Matrix34f effectMtx = sead::Matrix34f::ident;
        f32 waterHeight = 0.0f;
        if (rs::tryCalcEffectMtxDoubleCheckWaterOut(
                &effectMtx, &waterHeight, mActor, mWaterSurfaceFinder,
                rs::isCollidedGround(mCollision), mConst->getSwimCenterOffset())) {
            al::startHitReaction(mActor, "出水");
            mEffect->emitEffectWaterInOut(effectMtx, false);
        }
        al::setNerve(this, &DamageSurface);
        return;
    }

    al::setNerve(this, &DamageSwim);
}

bool PlayerStateDamageSwim::tryReactionWaterIn() {
    const al::LiveActor* player = mActor;
    sead::Matrix34f effectMtx = sead::Matrix34f::ident;
    f32 waterHeight = 0.0f;
    if (!rs::tryCalcEffectMtxDoubleCheckWaterIn(
            &effectMtx, &waterHeight, player, mWaterSurfaceFinder,
            rs::isCollidedGround(mCollision), mConst->getSwimCenterOffset()))
        return false;

    mEffect->emitEffectWaterInOut(effectMtx, true);
    al::startSe(mActor, "PlWaterIn");
    return true;
}

bool PlayerStateDamageSwim::tryReactionWaterOut() {
    sead::Matrix34f effectMtx = sead::Matrix34f::ident;
    f32 waterHeight = 0.0f;
    if (!rs::tryCalcEffectMtxDoubleCheckWaterOut(
            &effectMtx, &waterHeight, mActor, mWaterSurfaceFinder,
            rs::isCollidedGround(mCollision), mConst->getSwimCenterOffset()))
        return false;

    al::startHitReaction(mActor, "出水");
    mEffect->emitEffectWaterInOut(effectMtx, false);
    return true;
}

bool PlayerStateDamageSwim::isReduceOxygen() const {
    return !isDead();
}

bool PlayerStateDamageSwim::isNoDamageDown() const {
    return !isDead() && mIsNoDamageDown;
}

bool PlayerStateDamageSwim::isEnableCancel() const {
    if (al::isNerve(this, &DamageSwim))
        return al::isGreaterStep(this, mConst->getDamageSwimCancelFrame());
    if (al::isNerve(this, &DamageLandWater))
        return al::isGreaterStep(this, mConst->getDamageSwimSurfaceCancelFrame());
    return false;
}

bool PlayerStateDamageSwim::isEndGround() const {
    return al::isNerve(this, &EndGround);
}

bool PlayerStateDamageSwim::isEndInWater() const {
    return !al::isNerve(this, &EndOutOfWater) && !al::isNerve(this, &DamageSurface);
}

void PlayerStateDamageSwim::exeDamageSwim() {
    sead::Vector3f velocityH;
    sead::Vector3f velocityV;
    sead::Vector3f velocity;

    if (al::isFirstStep(this)) {
        if (mIsNoDamageDown)
            mAnimator->startAnim("NoDamageDownSwim");
        else
            mAnimator->startAnim("DamageDownSwim");

        velocityH.set(0.0f, 0.0f, 0.0f);
        al::calcFrontDir(&velocityH, mActor);
        al::setVelocity(mActor, -velocityH * mConst->getDamageSwimPushPower());
    }

    al::LiveActor* player = mActor;
    const PlayerConst* pConst = mConst;
    const IUsePlayerCollision* collision = mCollision;
    velocityH.set(0.0f, 0.0f, 0.0f);
    velocityV.set(0.0f, 0.0f, 0.0f);
    al::separateVelocityHV(&velocityH, &velocityV, player);
    const sead::Vector3f gravityDir = -al::getGravity(player);
    f32 speedV = velocityV.dot(gravityDir);
    if (rs::isOnGround(player, collision) && speedV < 0.0f)
        speedV = 0.0f;
    speedV -= pConst->getDamageSwimGravity();
    speedV = sead::Mathf::max(-pConst->getSwimFallSpeedMax(), speedV);
    velocityV.setScale(gravityDir, speedV);
    velocityH *= pConst->getSwimBrakeRateH();
    velocity = velocityH + velocityV;
    al::setVelocity(player, velocity);

    if (!rs::updateJudgeAndResult(mJudgeInWater)) {
        al::setNerve(this, &EndOutOfWater);
        kill();
    }
    if (mAnimator->isAnimEnd())
        kill();
}

void PlayerStateDamageSwim::exeDamageSurface() {
    if (al::isFirstStep(this)) {
        if (mIsNoDamageDown)
            mAnimator->startAnim("NoDamageDownSwimSurface");
        else
            mAnimator->startAnim("DamageDownSwimSurface");

        sead::Vector3f front = {0.0f, 0.0f, 0.0f};
        al::calcFrontDir(&front, mActor);
        sead::Vector3f* velocity = al::getVelocityPtr(mActor);
        al::verticalizeVec(velocity, front, *velocity);
        al::addVelocityToDirection(mActor, -front, mConst->getDamageSwimPushPower());
        mAirMoveControl->setup(mConst->getJumpMoveSpeedMax(),
                               mConst->getDamageSwimSurfacePushPower(), 0,
                               mConst->getDamageSwimSurfaceHopPower(),
                               mConst->getDamageSwimSurfaceGravity(), 9999, 1.0f);
        rs::resetJudge(mJudgeOutInWater);
    }

    mAirMoveControl->update();
    if (rs::isOnGround(mActor, mCollision)) {
        al::setNerve(this, &EndGround);
        kill();
    } else if (rs::updateJudgeAndResult(mJudgeOutInWater)) {
        tryReactionWaterIn();
        al::setNerve(this, &DamageLandWater);
    }
}

void PlayerStateDamageSwim::exeDamageLandWater() {
    al::LiveActor* player = mActor;
    const sead::Vector3f& gravity = al::getGravity(player);
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("DamageSwimSurfaceLand");
        al::limitVelocityDir(player, gravity, mConst->getDamageSwimSurfaceLandSpeed());
    }

    rs::moveDiveSwim(player, mConst->getSwimBrakeRateH(), 1.0f,
                     mConst->getDamageSwimSurfaceLandBrake());
    if (rs::isOnGround(player, mCollision)) {
        sead::Vector3f* velocity = al::getVelocityPtr(player);
        al::verticalizeVec(velocity, gravity, *velocity);
    }

    if (!rs::updateJudgeAndResult(mJudgeInWater)) {
        tryReactionWaterOut();
        al::setNerve(this, &EndOutOfWater);
        kill();
    }
    if (mAnimator->isAnimEnd())
        kill();
}

void PlayerStateDamageSwim::exeDead() {
    if (al::isFirstStep(this)) {
        if (mAnimator->isSubAnimPlaying())
            mAnimator->endSubAnim();

        if (_69)
            mAnimator->startAnim("DeadIce");
        else if (_68)
            mAnimator->startAnim("DeadWallow");
        else if (rs::isPlayerInWater(mActor))
            mAnimator->startAnim("DeadWater");
        else
            mAnimator->startAnimDead();

        rs::faceToCamera(mActor);
    }

    if (mAnimator->isAnimEnd())
        kill();
}

void PlayerStateDamageSwim::exeEndGround() {}

void PlayerStateDamageSwim::exeEndOutOfWater() {}

PlayerStateDamageSwim::~PlayerStateDamageSwim() = default;

