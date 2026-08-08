#include "Player/PlayerStateSwim.h"

#include "Library/Controller/PadRumbleFunction.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Nature/WaterSurfaceFinder.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionTurnControl.h"
#include "Player/PlayerAnimControlSwimWalk.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerCarryKeeper.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerEffect.h"
#include "Player/PlayerJointParamSwim.h"
#include "Player/PlayerJudgeReduceOxygen.h"
#include "Player/PlayerJudgeStartSwimJump.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerTrigger.h"
#include "Player/PlayerSpinCapAttack.h"
#include "Util/JudgeUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
void updateSwimJointParams(PlayerJointParamSwim* joint, al::LiveActor* actor,
                           const PlayerConst* pConst, PlayerActionTurnControl* turnControl);
void updateSwimHeadSliding(s32* counter, al::LiveActor* actor, const PlayerConst* pConst,
                           const al::WaterSurfaceFinder* waterSurfaceFinder);
void updateSwimTurn(al::LiveActor* actor, PlayerActionTurnControl* turnControl,
                    const PlayerInput* input, const PlayerConst* pConst, s32 step, f32 scale);

NERVE_IMPL(PlayerStateSwim, SwimMove);
NERVE_IMPL(PlayerStateSwim, SwimDive);
NERVE_IMPL(PlayerStateSwim, SwimPaddle);
NERVE_IMPL(PlayerStateSwim, SwimDiveIn);
NERVE_IMPL(PlayerStateSwim, SwimHeadIn);
NERVE_IMPL(PlayerStateSwim, SwimFallIn);
NERVE_IMPL(PlayerStateSwim, SwimSurface);
NERVE_IMPL(PlayerStateSwim, SwimWalk);
NERVE_IMPL(PlayerStateSwim, SwimHipDropLand);
NERVE_IMPL(PlayerStateSwim, EndSwimJump);
NERVE_IMPL(PlayerStateSwim, EndSwimJumpPop);
NERVE_IMPL(PlayerStateSwim, EndSwimJumpHipDrop);
NERVE_IMPL(PlayerStateSwim, SwimHipDropLoop);
NERVE_IMPL(PlayerStateSwim, SwimTrample);
NERVE_IMPL(PlayerStateSwim, SwimLand);
NERVE_IMPL(PlayerStateSwim, SwimHipDropHeadSlidingEnd);
NERVE_IMPL(PlayerStateSwim, SwimReflectDownFall);
NERVE_IMPL(PlayerStateSwim, SwimHipDropStart);
NERVE_IMPL(PlayerStateSwim, SwimHipDropHeadSliding);
NERVE_IMPL(PlayerStateSwim, SwimSpinCapSurface);
NERVE_IMPL(PlayerStateSwim, SwimSpinCap);
NERVE_IMPL(PlayerStateSwim, SwimHipDropJump);

NERVES_MAKE_NOSTRUCT(PlayerStateSwim, SwimSpinCapSurface, SwimSpinCap);
NERVES_MAKE_STRUCT(PlayerStateSwim, SwimMove, SwimDive, SwimPaddle, SwimDiveIn, SwimHeadIn,
                   SwimFallIn, SwimSurface, SwimWalk, SwimHipDropLand, EndSwimJump,
                   EndSwimJumpPop, EndSwimJumpHipDrop, SwimHipDropLoop, SwimTrample, SwimLand,
                   SwimHipDropHeadSlidingEnd, SwimReflectDownFall, SwimHipDropStart,
                   SwimHipDropHeadSliding, SwimHipDropJump);
}  // namespace

PlayerStateSwim::PlayerStateSwim(
    al::LiveActor* player, const PlayerConst* pConst, const IUsePlayerCollision* collision,
    const PlayerInput* input, const PlayerTrigger* trigger,
    const PlayerCarryKeeper* carryKeeper, const PlayerModelHolder* modelHolder,
    const PlayerExternalVelocity* externalVelocity, PlayerAnimator* animator,
    PlayerSpinCapAttack* spinCapAttack, const al::WaterSurfaceFinder* waterSurfaceFinder,
    PlayerEffect* effect, PlayerJointParamSwim* swimJoint)
    : al::ActorStateBase("泳ぎ", player), mConst(pConst), mCollision(collision), mInput(input),
      mTrigger(trigger), mCarryKeeper(carryKeeper), mModelHolder(modelHolder),
      mExternalVelocity(externalVelocity), mAnimator(animator), mEffect(effect),
      mSwimJoint(swimJoint), mSpinCapAttack(spinCapAttack), mTurnControl(nullptr),
      mAnimControlSwimWalk(new PlayerAnimControlSwimWalk(animator, pConst)),
      mWaterSurfaceFinder(waterSurfaceFinder),
      mJudgeReduceOxygen(new PlayerJudgeReduceOxygen(pConst, waterSurfaceFinder)),
      mJudgeStartSwimJump(new PlayerJudgeStartSwimJump(input, pConst, waterSurfaceFinder)), _a0(0),
      _a4(1.0f), _a8(false), _ac(0), _b0(0.0f), _b4(0), _b8(0), _bc(0), _c0{0.0f, 0.0f, 0.0f},
      _cc(0), _d0{}, _d8{0.0f, 0.0f, 0.0f}, _e4{0.0f, 0.0f, 0.0f}, _f0(0), _f4(0), _f8(0),
      _fc(0), _100(0), _104(false), _108(0), _10c(0), _110(false) {
    PlayerActionTurnControl* turnControl = new PlayerActionTurnControl(player);
    mTurnControl = turnControl;
    turnControl->set_88(true);
    const PlayerConst* turnConst = mConst;
    f32 rotSpeedMax = turnConst->getSwimRotSpeedMax();
    turnControl->setup(turnConst->getSwimRotStartAngle(), turnConst->getSwimRotFastAngle(),
                       rotSpeedMax, rotSpeedMax, turnConst->getSwimRotAccelFrame(),
                       turnConst->getSwimRotAccelFrameFast(), turnConst->getSwimRotBrakeFrame());
    initNerve(&NrvPlayerStateSwim.SwimMove, 0);
}

void PlayerStateSwim::kill() {
    *mSwimJoint = {};
    mSpinCapAttack->tryCancelCapState(mAnimator);
    al::NerveStateBase::kill();
    alPadRumbleFunction::stopPadRumbleLoop(mActor, "OjHugeMetalBallRotateLv_amfm_2xl", &al::getTrans(mActor));
    _110 = false;
}

void PlayerStateSwim::control() {
    if (_a0 + 1 <= 9999)
        _a0++;
    else
        _a0 = 9999;
    _100 = al::converge(_100, 0, 1);
    rs::updateJudge(mJudgeStartSwimJump);
    if (_10c - 1 >= 0)
        _10c--;

    f32 rate = 0.001f;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimSurface) ||
        al::isNerve(this, &NrvPlayerStateSwim.SwimWalk))
        rate = 0.0003f;
    _110 = rs::updateCollidedWallVibration(mActor, mCollision, _110,
                                           "OjHugeMetalBallRotateLv_amfm_2xl", rate, 0.017f);
}

bool PlayerStateSwim::attackHipDropKnockDown(al::HitSensor* self, al::HitSensor* other) {
    if (!isDead() && al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropLand) &&
        al::isFirstStep(this))
        return al::sendMsgPlayerHipDropKnockDown(other, self);
    return false;
}

f32 PlayerStateSwim::calcEyeHeadTiltRate() const {
    if (isDead())
        return 0.0f;
    f32 rate = mSwimJoint->_c * 1.75f / mConst->getSwimBentSpineMax();
    return al::easeIn(sead::Mathf::clamp(rate, -1.0f, 1.0f));
}

bool PlayerStateSwim::isEndSwimJump() const {
    return al::isNerve(this, &NrvPlayerStateSwim.EndSwimJump);
}

bool PlayerStateSwim::isEndSwimJumpPop() const {
    return al::isNerve(this, &NrvPlayerStateSwim.EndSwimJumpPop);
}

bool PlayerStateSwim::isEndSwimJumpHipDrop() const {
    return al::isNerve(this, &NrvPlayerStateSwim.EndSwimJumpHipDrop);
}

bool PlayerStateSwim::isEnableHipDropAttack() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimDive))
        return true;
    return al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropLoop);
}

bool PlayerStateSwim::isEnableCapThrow() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimMove))
        return true;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimPaddle))
        return true;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimTrample))
        return true;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimLand))
        return true;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimWalk))
        return true;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropHeadSlidingEnd))
        return true;
    const PlayerConst* pConst = mConst;
    if (!al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropLand))
        return false;
    return al::isGreaterEqualStep(this, pConst->getSwimDiveLandCancelFrame());
}

bool PlayerStateSwim::isEnableCapThrowSurface() const {
    if (isDead() || !isSurface())
        return false;
    return al::isNerve(this, &NrvPlayerStateSwim.SwimSurface) ||
           al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropHeadSlidingEnd);
}

bool PlayerStateSwim::isSurface() const {
    bool isSurfaceNerve = al::isNerve(this, &NrvPlayerStateSwim.SwimSurface);
    bool isFoundSurface = mWaterSurfaceFinder->isFoundSurface();
    if (isSurfaceNerve)
        return isFoundSurface;
    if (!isFoundSurface)
        return false;
    return mWaterSurfaceFinder->getDistance() <= mConst->getSwimSurfaceStartDist();
}

bool PlayerStateSwim::isEnableTalkSwim() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimSurface) ||
        al::isNerve(this, &NrvPlayerStateSwim.SwimMove) ||
        al::isNerve(this, &NrvPlayerStateSwim.SwimPaddle))
        return true;
    return al::isNerve(this, &NrvPlayerStateSwim.SwimTrample);
}

bool PlayerStateSwim::isEnableLookAt() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimSurface) ||
        al::isNerve(this, &NrvPlayerStateSwim.SwimMove) ||
        al::isNerve(this, &NrvPlayerStateSwim.SwimPaddle) ||
        al::isNerve(this, &NrvPlayerStateSwim.SwimLand) ||
        al::isNerve(this, &NrvPlayerStateSwim.SwimWalk) ||
        al::isNerve(this, &NrvPlayerStateSwim.SwimTrample))
        return true;
    return al::isNerve(this, &NrvPlayerStateSwim.SwimReflectDownFall);
}

bool PlayerStateSwim::isEnableReactionCapCatch() const {
    return !isDead();
}

bool PlayerStateSwim::isEnableCarryAction() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimDive))
        return false;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimFallIn))
        return false;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimHeadIn))
        return false;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimDiveIn))
        return false;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropStart))
        return false;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropLoop))
        return false;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropLand))
        return false;
    return !al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropHeadSliding) &&
           !al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropHeadSlidingEnd);
}

bool PlayerStateSwim::isEnableWallHitDown() const {
    if (!isDead()) {
        if (al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropHeadSliding))
            return true;
        if (al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropHeadSlidingEnd))
            return true;
    }
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimDiveIn))
        return _104;
    return false;
}

bool PlayerStateSwim::isHeadSliding() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropHeadSliding))
        return true;
    return al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropHeadSlidingEnd);
}

bool PlayerStateSwim::isEnableSurfaceShadow() const {
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimDive))
        return true;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimFallIn))
        return true;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimHeadIn))
        return al::calcSpeedV(mActor) < 0.0f;
    return al::isNerve(this, &NrvPlayerStateSwim.SwimDiveIn);
}

bool PlayerStateSwim::isEnableRecoveryLife() const {
    if (isDead() || !al::isNerve(this, &NrvPlayerStateSwim.SwimWalk))
        return false;
    return mAnimator->isAnim("SwimStandWait");
}

bool PlayerStateSwim::isSpinCapSurface() const {
    return !isDead() && al::isNerve(this, &SwimSpinCapSurface);
}

bool PlayerStateSwim::isReduceOxygen() const {
    if (isDead() || al::isNerve(this, &NrvPlayerStateSwim.SwimFallIn) ||
        al::isNerve(this, &NrvPlayerStateSwim.SwimDiveIn) ||
        (al::isNerve(this, &NrvPlayerStateSwim.SwimSurface) &&
         !rs::isCollidedGround(mCollision)))
        return false;
    return rs::updateJudgeAndResult(mJudgeReduceOxygen);
}

bool PlayerStateSwim::isWaitGround() const {
    if (isDead() || !rs::isCollidedGround(mCollision))
        return false;
    return al::calcSpeedExceptDir(mActor, rs::getCollidedGroundNormal(mCollision)) < 2.0f;
}

bool PlayerStateSwim::isEnableTrample() const {
    if (isDead())
        return false;
    return !al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropHeadSliding) &&
           !al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropHeadSlidingEnd);
}

bool PlayerStateSwim::isEnableCapTouchJump() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimPaddle))
        return true;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimTrample))
        return true;
    return isSurface();
}

void PlayerStateSwim::startCapThrow() {
    mSpinCapAttack->setupAttackInfo();
    al::setNerve(this, &SwimSpinCap);
}

void PlayerStateSwim::startCapThrowSurface() {
    mSpinCapAttack->setupAttackInfo();
    al::setNerve(this, &SwimSpinCapSurface);
}

bool PlayerStateSwim::tryReactionWaterOut() {
    const al::LiveActor* player = mActor;
    sead::Matrix34f effectMtx = sead::Matrix34f::ident;
    f32 waterHeight = 0.0f;
    if (!rs::tryCalcEffectMtxDoubleCheckWaterOut(
            &effectMtx, &waterHeight, player, mWaterSurfaceFinder,
            rs::isCollidedGround(mCollision), mConst->getSwimCenterOffset()))
        return false;

    al::startHitReaction(player, "出水");
    mEffect->emitEffectWaterInOut(effectMtx, false);
    alPadRumbleFunction::stopPadRumbleOneTime(mActor, "炭酸水イン1", -1);
    alPadRumbleFunction::stopPadRumbleOneTime(mActor, "炭酸水イン2", -1);
    alPadRumbleFunction::stopPadRumbleOneTime(mActor, "SE入水1", -1);
    alPadRumbleFunction::stopPadRumbleOneTime(mActor, "SE入水2", -1);
    alPadRumbleFunction::stopPadRumbleOneTime(mActor, "SE入水弱1", -1);
    alPadRumbleFunction::stopPadRumbleOneTime(mActor, "SE入水弱2", -1);
    return true;
}


namespace {
void updateSwimTurn(al::LiveActor* actor, PlayerActionTurnControl* turnControl,
                    const PlayerInput* input, const PlayerConst* pConst, s32 step, f32 scale) {
    f32 speedRate = al::calcRate01(al::calcSpeedH(actor), pConst->getSwimRotSpeedChangeStart(),
                                  pConst->getSwimLowSpeedMaxH());
    f32 turnRate = al::easeIn(speedRate) *
                   al::calcRate01(static_cast<f32>(step), 0.0f,
                                  static_cast<f32>(static_cast<u32>(pConst->getSwimHighAccelPermitFrame())));
    f32 turnSpeed =
        al::lerpValue(pConst->getSwimRotSpeedMax(), pConst->getSwimRotSpeedForward(), turnRate) *
        scale;
    turnControl->setup(pConst->getSwimRotStartAngle(), pConst->getSwimRotFastAngle(), turnSpeed,
                       turnSpeed, pConst->getSwimRotAccelFrame(),
                       pConst->getSwimRotAccelFrameFast(), pConst->getSwimRotBrakeFrame());

    sead::Vector3f up = -al::getGravity(actor);
    sead::Vector3f move = {0.0f, 0.0f, 0.0f};
    input->calcMoveInput(&move, up);
    turnControl->update(move, up);

    sead::Vector3f front = turnControl->get_5c();
    sead::Quatf quat = sead::Quatf::unit;
    al::makeQuatFrontUp(&quat, front, up);
    al::updatePoseQuat(actor, quat);
}
}  // namespace

// NON_MATCHING: current 788 bytes versus target 804; the leading swim-velocity helper is absent. Next hypothesis is restoring that helper call before tuning branch/codegen shape.
void PlayerStateSwim::exeSwimMove() {
    if (al::isFirstStep(this))
        _ac = 0;

    updateSwimTurn(mActor, mTurnControl, mInput, mConst, _a0, 1.0f);
    updateSwimJointParams(mSwimJoint, mActor, mConst, mTurnControl);

    if (mWaterSurfaceFinder->isFoundSurface() &&
        mWaterSurfaceFinder->getDistance() < mConst->getSwimSurfaceStartDist()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimSurface);
        return;
    }
    if (mTrigger->isOn(PlayerTrigger::EAttackSensorTrigger_val2)) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimDive);
        return;
    }
    if (mInput->isTriggerPaddle()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimPaddle);
        return;
    }
    if (mInput->isTriggerHipDrop()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimHipDropStart);
        return;
    }

    bool isOnGround = rs::isOnGround(mActor, mCollision);
    bool isMove = mInput->isMove();
    if (!isOnGround) {
        const char* animName;
        if (isMove) {
            _ac = 15;
            animName = "SwimStandMove";
        } else {
            if (mAnimator->isAnim("SwimStand")) {
                if (mAnimator->isAnimEnd())
                    mAnimator->startAnim("SwimStandWait");
                _ac = al::converge(_ac, 0, 1);
                return;
            }
            if (_ac > 0) {
                _ac = al::converge(_ac, 0, 1);
                return;
            }
            animName = "SwimStandWait";
        }
        if (!mAnimator->isAnim(animName))
            mAnimator->startAnim(animName);
        _ac = al::converge(_ac, 0, 1);
        return;
    }

    if (!isMove) {
        rs::waitGround(mActor, mCollision, mConst->getSwimGravity(),
                       mConst->getSwimFallSpeedMax(), 0.0f, 0.0f);
        al::setNerve(this, &NrvPlayerStateSwim.SwimLand);
        return;
    }

    sead::Vector3f groundNormal = sead::Vector3f::zero;
    rs::calcGroundNormalOrGravityDir(&groundNormal, mActor, mCollision);
    sead::Vector3f velocity = sead::Vector3f::zero;
    al::verticalizeVec(&velocity, groundNormal, al::getVelocity(mActor));
    f32 gravityWalk = mConst->getSwimGravityWalk();
    velocity -= groundNormal * gravityWalk;
    al::setVelocity(mActor, velocity);
    al::setNerve(this, &NrvPlayerStateSwim.SwimWalk);
}

namespace {
void updateSwimJointParams(PlayerJointParamSwim* joint, al::LiveActor* actor,
                           const PlayerConst* pConst, PlayerActionTurnControl* turnControl) {
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    al::calcFrontDir(&front, actor);
    f32 speed = front.dot(al::getVelocity(actor));
    f32 speedRate = al::calcRate01(speed, 0.0f, pConst->getSwimLowSpeedMaxH());

    f32 bentForward = speedRate * pConst->getSwimBentForwardMax();
    f32 forwardBlend = pConst->getSwimBentForwardBlendRate();
    joint->_0 = al::lerpValue(joint->_0, bentForward, forwardBlend);

    sead::Vector3f up = -al::getGravity(actor);
    f32 turnRate = speedRate * turnControl->calcTurnPowerRate(up);
    f32 bentSide = turnRate * pConst->getSwimBentSideMax();
    f32 sideBlend = pConst->getSwimBentSideBlendRate();
    joint->_4 = al::lerpValue(joint->_4, bentSide, sideBlend);
    f32 bentSpine = turnRate * pConst->getSwimBentSpineMax();
    f32 spineBlend = pConst->getSwimBentSideBlendRate();
    joint->_c = al::lerpValue(joint->_c, bentSpine, spineBlend);
    f32 neutralBlend = pConst->getSwimBentSideBlendRate();
    joint->_8 = al::lerpValue(joint->_8, 0.0f, neutralBlend);
}
}  // namespace

void PlayerStateSwim::exeSwimLand() {
    sead::Vector3f up;
    sead::Vector3f velocity;
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("SwimLand");
        _f0 = 0;
    }

    rs::calcGroundNormalOrGravityDir(&_d8, mActor, mCollision);
    rs::waitGround(mActor, mCollision, mConst->getSwimGravity(),
                   mConst->getSwimFallSpeedMax(), 0.0f, 0.0f);
    updateSwimTurn(mActor, mTurnControl, mInput, mConst, _a0, 0.75f);

    const PlayerConst* pConst = mConst;
    PlayerJointParamSwim* joint = mSwimJoint;
    f32 forwardBlend = pConst->getSwimBentForwardBlendRate();
    joint->_0 = al::lerpValue(joint->_0, 0.0f, forwardBlend);
    f32 sideBlend = pConst->getSwimBentSideBlendRate();
    joint->_4 = al::lerpValue(joint->_4, 0.0f, sideBlend);
    f32 sideBlend2 = pConst->getSwimBentSideBlendRate();
    joint->_8 = al::lerpValue(joint->_8, 0.0f, sideBlend2);
    f32 frontBlend = pConst->getSwimBentFrontBlendRate();
    joint->_c = al::lerpValue(joint->_c, 0.0f, frontBlend);

    if (mWaterSurfaceFinder->isFoundSurface() &&
        mWaterSurfaceFinder->getDistance() < mConst->getSwimSurfaceStartDist()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimSurface);
        return;
    }
    if (mTrigger->isOn(PlayerTrigger::EAttackSensorTrigger_val2)) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimTrample);
        return;
    }
    if (mTrigger->isOn(PlayerTrigger::EAttackSensorTrigger_val1) || mInput->isTriggerPaddle()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimPaddle);
        return;
    }

    if (rs::isCollidedGround(mCollision)) {
        _f0 = 0;
    } else if (_f0++ >= 2) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimMove);
        return;
    }

    if (!mInput->isMove() && !mAnimator->isAnimEnd())
        return;

    al::LiveActor* actor = mActor;
    const IUsePlayerCollision* collision = mCollision;
    const PlayerConst* pConst2 = mConst;
    up = {0.0f, 0.0f, 0.0f};
    rs::calcGroundNormalOrGravityDir(&up, actor, collision);
    velocity = {0.0f, 0.0f, 0.0f};
    al::verticalizeVec(&velocity, up, al::getVelocity(actor));
    velocity -= up * pConst2->getSwimGravityWalk();
    al::setVelocity(actor, velocity);
    al::setNerve(this, &NrvPlayerStateSwim.SwimWalk);
}



void PlayerStateSwim::exeSwimDive() {
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("SwimDive");
        al::LiveActor* player = mActor;
        al::limitVelocityDir(player, al::getGravity(player), mConst->getSwimDiveStartSpeed());
        *mSwimJoint = {};
    }
    updateNerveDownFall();
}

void PlayerStateSwim::updateNerveDownFall() {
    if (al::isFirstStep(this)) {
        _b4 = mConst->getSwimDiveButtonValidFrame();
        _b8 = 0;
        _bc = 0;
    }

    rs::calcGroundNormalOrUpDir(&_d8, mActor, mCollision);
    if (al::isGreaterEqualStep(this, mConst->getSwimDiveNoBrakeFrame())) {
        if (!mInput->isHoldHipDrop()) {
            _b4 = 0;
        } else {
            s32 next = _b4 - 1;
            if (next < 0)
                next = 0;
            _b4 = next;
        }
    }

    f32 brake = 0.0f;
    if (_b4 == 0)
        brake = mConst->getSwimDiveBrake();
    rs::moveDiveSwim(mActor, 1.0f, 1.0f, brake);

    if (mTrigger->isOn(PlayerTrigger::EAttackSensorTrigger_val3) ||
        mTrigger->isOn(PlayerTrigger::EAttackSensorTrigger_val1)) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimReflectDownFall);
        return;
    }

    if (tryChangeHipDropLand(&_b8, &_bc))
        return;

    const PlayerInput* input = mInput;
    if (!mCarryKeeper->isCarry() && input->isTriggerHeadSliding()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimHipDropHeadSliding);
        return;
    }

    f32 speed = -al::calcSpeedV(mActor);
    if (mConst->getSwimDiveEndSpeed() > speed) {
        u32 step = ++_bc;
        if (step >= static_cast<u32>(mConst->getSwimDiveEndFrame())) {
            if (mWaterSurfaceFinder->isFoundSurface() &&
                mWaterSurfaceFinder->getDistance() < mConst->getSwimSurfaceStartDist())
                al::setNerve(this, &NrvPlayerStateSwim.SwimSurface);
            else
                al::setNerve(this, &NrvPlayerStateSwim.SwimMove);
        }
    }
}

void PlayerStateSwim::exeSwimHipDropStart() {
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("SwimHipDropStart");
        al::setVelocityZero(mActor);
        *mSwimJoint = {};
        if (mWaterSurfaceFinder->isFoundSurface())
            _b0 = mWaterSurfaceFinder->getDistance();
        else
            _b0 = mConst->getTall();
    }

    al::keepWaterSurfaceHeight(mActor, mWaterSurfaceFinder, _b0);
    rs::calcGroundNormalOrUpDir(&_d8, mActor, mCollision);
    const PlayerInput* input = mInput;
    if (!mCarryKeeper->isCarry() && input->isTriggerHeadSliding()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimHipDropHeadSliding);
        return;
    }
    if (!mAnimator->isAnimEnd())
        return;

    al::addVelocityToGravity(mActor, mConst->getSwimDiveStartSpeed());
    al::setNerve(this, &NrvPlayerStateSwim.SwimHipDropLoop);
}

void PlayerStateSwim::exeSwimHipDropLoop() {
    if (al::isFirstStep(this))
        mAnimator->startAnim("SwimHipDrop");
    updateNerveDownFall();
}


void PlayerStateSwim::exeSwimHipDropLand() {
    sead::Vector3f groundNormal;
    sead::Vector3f velocity;
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("SwimHipDropLand");
        al::setVelocityZero(mActor);
        rs::startHitReactionHipDropLand(mActor, false);
    }

    rs::calcGroundNormalOrUpDir(&_d8, mActor, mCollision);
    rs::waitGround(mActor, mCollision, mConst->getSwimGravity(),
                   mConst->getSwimFallSpeedMax(), 0.0f, 0.0f);

    if (al::isInRangeStep(this, 0, mConst->getJumpHipDropPermitEndFrame()) &&
        mInput->isTriggerJump()) {
        if (mWaterSurfaceFinder->isFoundSurface() &&
            mWaterSurfaceFinder->getDistance() <= mConst->getSwimSurfaceEnableJumpHeight()) {
            al::setNerve(this, &NrvPlayerStateSwim.EndSwimJumpHipDrop);
            kill();
            return;
        }
        al::setNerve(this, &NrvPlayerStateSwim.SwimHipDropJump);
        return;
    }

    const PlayerConst* pConstGate = mConst;
    if (al::isNerve(this, &NrvPlayerStateSwim.SwimHipDropLand) &&
        al::isGreaterEqualStep(this, pConstGate->getSwimDiveLandCancelFrame())) {
        if (mWaterSurfaceFinder->isFoundSurface() &&
            mWaterSurfaceFinder->getDistance() < mConst->getSwimSurfaceStartDist()) {
            al::setNerve(this, &NrvPlayerStateSwim.SwimSurface);
            return;
        }
        if (mInput->isTriggerPaddle()) {
            al::setNerve(this, &NrvPlayerStateSwim.SwimPaddle);
            return;
        }
        if (mInput->isMove()) {
            al::setNerve(this, &NrvPlayerStateSwim.SwimMove);
            return;
        }
    }

    if (!rs::isCollidedGround(mCollision)) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimMove);
        return;
    }
    if (!mAnimator->isAnimEnd())
        return;

    al::LiveActor* actor = mActor;
    const IUsePlayerCollision* collision = mCollision;
    const PlayerConst* pConst = mConst;
    groundNormal = {0.0f, 0.0f, 0.0f};
    rs::calcGroundNormalOrGravityDir(&groundNormal, actor, collision);
    velocity = {0.0f, 0.0f, 0.0f};
    al::verticalizeVec(&velocity, groundNormal, al::getVelocity(actor));
    velocity -= groundNormal * pConst->getSwimGravityWalk();
    al::setVelocity(actor, velocity);
    al::setNerve(this, &NrvPlayerStateSwim.SwimWalk);
}

void PlayerStateSwim::exeSwimHipDropHeadSliding() {
    al::LiveActor* actor = mActor;
    const sead::Vector3f& gravity = al::getGravity(actor);
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("SwimHeadSlidingStart");
        sead::Vector3f velH = {0.0f, 0.0f, 0.0f};
        sead::Vector3f velV = {0.0f, 0.0f, 0.0f};
        al::separateVectorParallelVertical(&velV, &velH, gravity, al::getVelocity(actor));
        if (!al::tryNormalizeOrZero(&velH))
            al::calcFrontDir(&velH, actor);
        if (velV.dot(gravity) > 0.0f)
            velV = {0.0f, 0.0f, 0.0f};
        velV.setScaleAdd(mConst->getSwimHeadSlidingJump(), gravity, velV);
        al::setVelocity(actor, mConst->getSwimHeadSlidingSpeed() * velH + velV);
        _f8 = 0;
    }

    f32 speedEnd = mConst->getSwimHeadSlidingSpeedEnd();
    sead::Vector3f moveInput = {0.0f, 0.0f, 0.0f};
    mInput->calcMoveInput(&moveInput, -gravity);
    {
        al::LiveActor* sideActor = mActor;
        const PlayerConst* pConst = mConst;
        PlayerJointParamSwim* joint = mSwimJoint;
        sead::Vector3f side = {0.0f, 0.0f, 0.0f};
        al::calcSideDir(&side, sideActor);
        f32 sidePower = side.dot(moveInput);
        f32 bent = -(al::easeIn(sead::Mathf::abs(sidePower)) * al::sign(sidePower) *
                      pConst->getSwimBentFrontMax());
        f32 bentBlend = pConst->getSwimBentFrontBlendRate();
        joint->_8 = al::lerpValue(joint->_8, bent, bentBlend);
    }

    updateSwimHeadSliding(&_f8, actor, mConst, mWaterSurfaceFinder);
    rs::moveDivingJump(actor, moveInput, 0.0f, mConst->getSwimHeadSlidingBrake(),
                       mConst->getSwimHeadSlidingSpeed(), speedEnd,
                       mConst->getSwimHeadSlidingSideAccel(),
                       mConst->getSwimHeadSlidingGravity(), mConst->getSwimFallSpeedMax(),
                       mConst->getSlerpQuatGrav());

    if (al::isGreaterEqualStep(this, mConst->getSwimHeadSlidingFrame())) {
        s32 brakeFrame = mConst->getSwimHeadSlidingBrakeFrame();
        f32 brake = 0.0f;
        if (brakeFrame >= 1)
            brake = (mConst->getSwimHeadSlidingSpeed() - speedEnd) /
                    static_cast<f32>(brakeFrame);
        al::subVelocityExceptDirectionLimit(actor, gravity, brake, speedEnd);
    }
    if (mAnimator->isAnim("SwimHeadSlidingStart") && mAnimator->isAnimEnd())
        mAnimator->startAnim("SwimHeadSliding");
    if (al::isNearZeroOrLess(al::calcSpeedExceptDir(actor, gravity) - speedEnd, 0.001f))
        al::setNerve(this, &NrvPlayerStateSwim.SwimHipDropHeadSlidingEnd);
}

namespace {
// NON_MATCHING: current 520 bytes versus target 532; target keeps a packed 64-bit surface-found/distance snapshot. Next hypothesis is a single aggregate/local snapshot spanning the counter branch.
void updateSwimHeadSliding(s32* counter, al::LiveActor* actor, const PlayerConst* pConst,
                           const al::WaterSurfaceFinder* waterSurfaceFinder) {
    {
        bool isFoundSurface = waterSurfaceFinder->isFoundSurface();
        f32 surfaceDistance = waterSurfaceFinder->getDistance();
        if (*counter != 0) {
            if (!isFoundSurface || surfaceDistance >= pConst->getSwimSurfaceEndDist()) {
                *counter = 0;
                return;
            }
            (*counter)++;
        } else if (!isFoundSurface ||
                   surfaceDistance > pConst->getSwimSurfaceStartDist()) {
            return;
        }
    }

    sead::Vector3f vertical = {0.0f, 0.0f, 0.0f};
    sead::Vector3f horizontal = {0.0f, 0.0f, 0.0f};
    al::separateVelocityHV(&horizontal, &vertical, actor);
    sead::Vector3f up = -al::getGravity(actor);
    f32 speed = vertical.dot(up);
    (void)pConst->getSwimSurfaceDamperStart();
    (void)pConst->getSwimSurfaceDamperFrame();
    if (waterSurfaceFinder->isFoundSurface()) {
        speed *= pConst->getSwimSurfaceMoveDamper();
        speed += (waterSurfaceFinder->getDistance() - pConst->getSwimSurfaceMoveBaseHeight()) *
                 pConst->getSwimSurfaceMoveSpring();
        f32 speedMax = pConst->getSwimRiseSpeedMax();
        if (speed > speedMax)
            speed = speedMax;
    }
    sead::Vector3f velocity = horizontal + up * speed;
    al::setVelocity(actor, velocity);
    if (waterSurfaceFinder->isFoundSurface()) {
        sead::Vector3f displacement = waterSurfaceFinder->getSurfaceDisplacement();
        velocity = displacement + al::getTrans(actor);
        al::setTrans(actor, velocity);
    }
}
}  // namespace

void PlayerStateSwim::exeSwimHipDropHeadSlidingEnd() {
    al::LiveActor* actor = mActor;
    const sead::Vector3f& gravity = al::getGravity(actor);
    if (al::isFirstStep(this))
        mAnimator->startAnim("SwimHeadSlidingEnd");

    updateSwimHeadSliding(&_f8, actor, mConst, mWaterSurfaceFinder);
    f32 speedMin = static_cast<f32>(mConst->getSwimHeadSlidingEndSpeedMin());
    s32 brakeFrame = mConst->getSwimHeadSlidingEndBrakeFrame();
    f32 brake = 0.0f;
    if (brakeFrame >= 1)
        brake = (mConst->getSwimHeadSlidingSpeedEnd() - speedMin) /
                static_cast<f32>(brakeFrame);
    al::subVelocityExceptDirectionLimit(actor, gravity, brake, speedMin);

    const PlayerConst* pConst = mConst;
    PlayerJointParamSwim* joint = mSwimJoint;
    f32 forwardBlend = pConst->getSwimBentForwardBlendRate();
    joint->_0 = al::lerpValue(joint->_0, 0.0f, forwardBlend);
    f32 sideBlend = pConst->getSwimBentSideBlendRate();
    joint->_4 = al::lerpValue(joint->_4, 0.0f, sideBlend);
    f32 sideBlend2 = pConst->getSwimBentSideBlendRate();
    joint->_8 = al::lerpValue(joint->_8, 0.0f, sideBlend2);
    f32 frontBlend = pConst->getSwimBentFrontBlendRate();
    joint->_c = al::lerpValue(joint->_c, 0.0f, frontBlend);

    if (_f8 == 0 && mInput->isTriggerPaddle()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimPaddle);
        return;
    }
    if (mInput->isTriggerHipDrop()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimHipDropStart);
        return;
    }
    if (al::isNearZeroOrLess(al::calcSpeedExceptDir(actor, gravity) - speedMin, 0.001f))
        al::setNerve(this, &NrvPlayerStateSwim.SwimMove);
}

// NON_MATCHING: current 724 bytes versus target 744; the leading swim-velocity helper is absent. Next hypothesis is restoring that helper call before tuning branch/codegen shape.
void PlayerStateSwim::exeSwimTrample() {
    if (al::isFirstStep(this)) {
        f32 intervalRate =
            1.0f - al::calcRate01(static_cast<f32>(_a0),
                                  static_cast<f32>(mConst->getSwimPaddleAnimRateIntervalMin()),
                                  static_cast<f32>(mConst->getSwimPaddleAnimRateIntervalMax()));
        intervalRate = sead::Mathf::clamp(intervalRate, 0.0f, 1.0f);
        _a0 = 0;
        _a8 = false;
        _a4 = al::lerpValue(1.0f, mConst->getSwimPaddleAnimMaxRate(), intervalRate);
        mAnimator->startAnim("SwimTrample");
    }

    updateSwimTurn(mActor, mTurnControl, mInput, mConst, _a0, 1.0f);
    updateSwimJointParams(mSwimJoint, mActor, mConst, mTurnControl);

    if (mWaterSurfaceFinder->isFoundSurface() &&
        mWaterSurfaceFinder->getDistance() < mConst->getSwimSurfaceStartDist()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimSurface);
        return;
    }
    if (mInput->isTriggerPaddle()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimPaddle);
        return;
    }
    if (mInput->isTriggerHipDrop()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimHipDropStart);
        return;
    }

    if (rs::isOnGround(mActor, mCollision)) {
        if (mInput->isMove()) {
            sead::Vector3f groundNormal = {0.0f, 0.0f, 0.0f};
            rs::calcGroundNormalOrGravityDir(&groundNormal, mActor, mCollision);
            sead::Vector3f velocity = {0.0f, 0.0f, 0.0f};
            al::verticalizeVec(&velocity, groundNormal, al::getVelocity(mActor));
            velocity -= groundNormal * mConst->getSwimGravityWalk();
            al::setVelocity(mActor, velocity);
            al::setNerve(this, &NrvPlayerStateSwim.SwimWalk);
        } else {
            rs::waitGround(mActor, mCollision, mConst->getSwimGravity(),
                           mConst->getSwimFallSpeedMax(), 0.0f, 0.0f);
            al::setNerve(this, &NrvPlayerStateSwim.SwimLand);
        }
        return;
    }

    if (mAnimator->isAnimEnd() || al::calcSpeedV(mActor) < 0.0f)
        al::setNerve(this, &NrvPlayerStateSwim.SwimMove);
}



// NON_MATCHING: current 468 bytes versus target 448; compiler preserves this+0x20 in X21 across the first-step branch. Next hypothesis is shortening the actor/local lifetime across first-step setup.
void PlayerStateSwim::exeSwimReflectDownFall() {
    al::LiveActor* player = mActor;
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("SwimTrample");
        sead::Vector3f gravity = al::getGravity(player);
        f32 x = gravity.x;
        f32 y = gravity.y;
        f32 z = gravity.z;
        const PlayerConst* pConst = mConst;
        f32 power = pConst->getDiveTramplePower();
        sead::Vector3f velocity = {-x * power, -y * power, -z * power};
        al::setVelocity(player, velocity);
    }

    al::addVelocityToGravity(player, mConst->getSwimGravity());
    updateSwimJointParams(mSwimJoint, mActor, mConst, mTurnControl);

    if (mWaterSurfaceFinder->isFoundSurface() &&
        mWaterSurfaceFinder->getDistance() < mConst->getSwimSurfaceStartDist()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimSurface);
        return;
    }
    if (mTrigger->isOn(PlayerTrigger::EAttackSensorTrigger_val2)) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimTrample);
        return;
    }
    if (al::isGreaterStep(this, mConst->getDiveTrampleCancelFrame())) {
        if (mInput->isTriggerPaddle()) {
            al::setNerve(this, &NrvPlayerStateSwim.SwimPaddle);
            return;
        }
        if (mInput->isTriggerHipDrop()) {
            al::setNerve(this, &NrvPlayerStateSwim.SwimHipDropStart);
            return;
        }
        if (mInput->isMove()) {
            al::setNerve(this, &NrvPlayerStateSwim.SwimMove);
            return;
        }
    }
    if (mAnimator->isAnimEnd() || al::calcSpeedV(player) < 0.0f)
        al::setNerve(this, &NrvPlayerStateSwim.SwimMove);
}
void PlayerStateSwim::exeEndSwimJump() {}

void PlayerStateSwim::exeEndSwimJumpPop() {}

void PlayerStateSwim::exeEndSwimJumpHipDrop() {}

bool PlayerStateSwim::tryChangeHipDropLand(s32* state, s32* step) {
    if (!rs::isOnGround(mActor, mCollision) ||
        (mTrigger->isOn(PlayerTrigger::ECollisionTrigger_val1) && mInput->isHoldHipDrop()))
        return false;

    (*state)++;
    (*step)++;
    rs::waitGround(mActor, mCollision, mConst->getSwimGravity(), mConst->getSwimGravity(), 1.0f,
                   0.0f);
    if (static_cast<u32>(*state) > static_cast<u32>(mConst->getSwimDiveLandCount()))
        al::setNerve(this, &NrvPlayerStateSwim.SwimHipDropLand);
    return true;
}

PlayerStateSwim::~PlayerStateSwim() = default;
