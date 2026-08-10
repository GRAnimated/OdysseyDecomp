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

#include "Player/PlayerActionFunction.h"
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
void updateSwimVelocityH(sead::Vector3f* velocityH, al::LiveActor* actor,
                         const PlayerConst* pConst, const PlayerInput* input,
                         const IUsePlayerCollision* collision, bool isHighAccel, bool isSurface,
                         bool isSpinCapSurface);
void updateSwimVelocityV(sead::Vector3f* velocityV, const PlayerConst* pConst,
                         const IUsePlayerCollision* collision, const PlayerTrigger* trigger,
                         const sead::Vector3f& normal, u32 step, f32 gravity);
void updateSwimVelocity(al::LiveActor* actor, sead::Vector3f* normal, const PlayerConst* pConst,
                        const PlayerInput* input, const IUsePlayerCollision* collision,
                        const PlayerTrigger* trigger, s32 step);
void updateSwimSurfaceVerticalSpeed(f32* speed, const al::WaterSurfaceFinder* waterSurfaceFinder,
                                    const PlayerConst* pConst, s32 counter, bool isMove);
void updateSwimSurfaceVelocity(al::LiveActor* actor, sead::Vector3f* normal,
                               const PlayerConst* pConst, const PlayerInput* input,
                               const IUsePlayerCollision* collision,
                               const al::WaterSurfaceFinder* waterSurfaceFinder, s32 surfaceCounter,
                               s32 step, bool isSpinCapSurface);
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
    initNerve(&NrvPlayerStateSwim.SwimMove);
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
    return al::easeIn(sead::Mathf::clamp(
        mSwimJoint->_c * 1.75f / mConst->getSwimBentSpineMax(), -1.0f, 1.0f));
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


// NON_MATCHING: target/current are both 1872 bytes with the complete observed call surface and the
// EndSwimJump terminal path corrected to the target +0x28 kill() virtual. Remaining mismatch is the
// 0xB0 target vs 0xA0 current frame/stack coloring plus the still-inlined 0x710048AFE8 helper; next
// hypothesis is recovering that helper as a separate function to restore target temporary lifetimes.
void PlayerStateSwim::exeSwimSurface() {
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("SwimStandSurface");
        _f4 = 0;
        if (mWaterSurfaceFinder->isFoundSurface())
            _e4 = mWaterSurfaceFinder->getSurfacePosition();
        else
            _e4 = al::getTrans(mActor);
    }

    al::LiveActor* actor = mActor;
    const al::WaterSurfaceFinder* waterSurfaceFinder = mWaterSurfaceFinder;
    bool shouldLeaveSurface =
        (!waterSurfaceFinder->isFoundSurface() ||
         waterSurfaceFinder->getDistance() >= mConst->getSwimSurfaceEndDist()) &&
        _fc < 1;

    updateSwimSurfaceVelocity(actor, &_d8, mConst, mInput, mCollision, waterSurfaceFinder, _f4,
                              _a0, false);

    bool isMove = mInput->isMove();
    if (isMove) {
        if (waterSurfaceFinder->isFoundSurface()) {
            sead::Vector3f up = -al::getGravity(actor);
            al::addVelocityToDirection(
                actor, up,
                sead::Mathf::min((waterSurfaceFinder->getSurfacePosition() - _e4).dot(up), 0.0f) *
                    0.1f);
            sead::Vector3f trans =
                waterSurfaceFinder->getSurfaceDisplacement() + al::getTrans(actor);
            al::setTrans(actor, trans);
        }
    } else if (waterSurfaceFinder->isFoundSurface()) {
        sead::Vector3f trans =
            waterSurfaceFinder->getSurfaceDisplacement() + al::getTrans(actor);
        al::setTrans(actor, trans);
    }

    if (waterSurfaceFinder->isFoundSurface())
        _e4 = waterSurfaceFinder->getSurfacePosition();
    else
        _e4 = al::getTrans(actor);

    updateSwimTurn(actor, mTurnControl, mInput, mConst, _a0, 1.0f);

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

    s32 damperStart = mConst->getSwimSurfaceDamperStart();
    s32 damperFrame = mConst->getSwimSurfaceDamperFrame();
    bool canPaddleFromWait = false;
    if (mInput->isMove()) {
        _f4 = mConst->getSwimSurfaceDamperStart();
        if (mCarryKeeper->isCarry()) {
            if (!mAnimator->isAnim("SwimSurfaceCarry")) {
                if (mAnimator->isAnim("SwimSurfaceNormal")) {
                    f32 frame = mAnimator->getAnimFrame();
                    mAnimator->startAnim("SwimSurfaceCarry");
                    mAnimator->setAnimFrame(frame);
                } else {
                    mAnimator->startAnim("SwimSurfaceCarry");
                }
            }
        } else {
            if (!mAnimator->isAnim("SwimSurfaceNormal")) {
                if (mAnimator->isAnim("SwimSurfaceCarry")) {
                    f32 frame = mAnimator->getAnimFrame();
                    mAnimator->startAnim("SwimSurfaceNormal");
                    mAnimator->setAnimFrame(frame);
                } else {
                    mAnimator->startAnim("SwimSurfaceNormal");
                }
            }
        }
    } else {
        if (!mAnimator->isAnim("SwimStandSurface") && !mAnimator->isAnim("SwimStandWait"))
            mAnimator->startAnim("SwimStandSurface");

        if (mAnimator->isAnim("SwimStandSurface")) {
            if (rs::isCollidedGround(mCollision) &&
                (!waterSurfaceFinder->isFoundSurface() ||
                 waterSurfaceFinder->getDistance() >= mConst->getSwimSurfaceStartDist() + 5.0f)) {
                mAnimator->startAnim("SwimStandWait");
            }
        } else if (mAnimator->isAnim("SwimStandWait") &&
                   waterSurfaceFinder->isFoundSurface() &&
                   waterSurfaceFinder->getDistance() < mConst->getSwimSurfaceStartDist()) {
            mAnimator->startAnim("SwimStandSurface");
        }
        canPaddleFromWait = mAnimator->isAnim("SwimStandWait") && _100 == 0;
        _f4 = al::converge(_f4, damperStart + damperFrame, 1);
    }

    if (_100 == 0 && rs::isJudge(mJudgeStartSwimJump)) {
        _fc = 0;
        al::setNerve(this, &NrvPlayerStateSwim.EndSwimJump);
        kill();
        return;
    }
    if (mInput->isTriggerHipDrop()) {
        _fc = 0;
        al::setNerve(this, &NrvPlayerStateSwim.SwimHipDropStart);
        return;
    }

    if (shouldLeaveSurface) {
        if (!mAnimator->isAnim("SwimStandWait") || !rs::isCollidedGround(mCollision)) {
            al::setNerve(this, &NrvPlayerStateSwim.SwimMove);
            return;
        }
        al::setNerve(this, &NrvPlayerStateSwim.SwimWalk);
    }

    if (canPaddleFromWait && mInput->isTriggerPaddle()) {
        _a0 = 0;
        sead::Vector3f* velocity = al::getVelocityPtr(actor);
        sead::Vector3f up = -al::getGravity(actor);
        al::limitVectorOppositeDir(velocity, up, *velocity, velocity->length());
    }

    if (waterSurfaceFinder->isFoundSurface() &&
        waterSurfaceFinder->getDistance() < mConst->getSwimSurfaceStartDist()) {
        _fc = 0;
        return;
    }
    _fc = al::converge(_fc, 0, 1);
}

namespace {
// NON_MATCHING: target is the unlabeled 572-byte helper at 0x71004860CC; current is 820 bytes because
// the target's 364-byte 0x710048AFE8 surface-vertical helper still inlines here. Complete observed
// behavior is recovered; next hypothesis is the original helper factoring/inliner-cost source shape.
void updateSwimSurfaceVelocity(al::LiveActor* actor, sead::Vector3f* normal,
                               const PlayerConst* pConst, const PlayerInput* input,
                               const IUsePlayerCollision* collision,
                               const al::WaterSurfaceFinder* waterSurfaceFinder, s32 surfaceCounter,
                               s32 step, bool isSpinCapSurface) {
    sead::Vector3f previousNormal = *normal;
    sead::Vector3f velocityH = {0.0f, 0.0f, 0.0f};
    sead::Vector3f velocityV = {0.0f, 0.0f, 0.0f};
    al::separateVelocityDirHV(&velocityH, &velocityV, actor, previousNormal);
    rs::calcGroundNormalOrGravityDir(normal, actor, collision);
    al::alongVectorNormalH(&velocityH, velocityH, previousNormal, *normal);

    f32 speedV = previousNormal.dot(velocityV);
    velocityV = *normal * speedV;
    f32 surfaceSpeed = velocityV.dot(*normal);
    updateSwimSurfaceVerticalSpeed(&surfaceSpeed, waterSurfaceFinder, pConst, surfaceCounter,
                                   input->isMove());

    f32 risePower = pConst->getSwimRisePower();
    f32 surfaceGravity = pConst->getSwimSurfaceGravity();
    f32 targetSpeed;
    if (pConst->getSwimHighAccelPermitFrame() <= static_cast<u32>(step)) {
        targetSpeed = surfaceSpeed - surfaceGravity;
        f32 fallSpeed = -pConst->getSwimFallSpeedMax();
        if (targetSpeed < fallSpeed)
            targetSpeed = fallSpeed;
    } else {
        targetSpeed = surfaceSpeed + risePower;
        f32 riseSpeedMax = pConst->getSwimRiseSpeedMax();
        if (targetSpeed > riseSpeedMax)
            targetSpeed = riseSpeedMax;
    }
    velocityV = *normal * targetSpeed;

    updateSwimVelocityH(&velocityH, actor, pConst, input, collision, false, true,
                        isSpinCapSurface);
    sead::Vector3f velocity = velocityH + velocityV;
    al::setVelocity(actor, velocity);
}

void updateSwimTurn(al::LiveActor* actor, PlayerActionTurnControl* turnControl,
                    const PlayerInput* input, const PlayerConst* pConst, s32 step, f32 scale) {
    f32 turnRate =
        al::easeIn(al::calcRate01(al::calcSpeedH(actor), pConst->getSwimRotSpeedChangeStart(),
                                 pConst->getSwimLowSpeedMaxH())) *
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

void PlayerStateSwim::exeSwimMove() {
    if (al::isFirstStep(this))
        _ac = 0;

    updateSwimVelocity(mActor, &_d8, mConst, mInput, mCollision, mTrigger, _a0);
    updateSwimTurn(mActor, mTurnControl, mInput, mConst, _a0, 1.0f);
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
    if (isOnGround) {
        const IUsePlayerCollision* collision = mCollision;
        const PlayerConst* pConst = mConst;
        al::LiveActor* actor = mActor;
        if (isMove) {
            sead::Vector3f groundNormal = {0.0f, 0.0f, 0.0f};
            sead::Vector3f velocity;
            rs::calcGroundNormalOrGravityDir(&groundNormal, actor, collision);
            velocity = {0.0f, 0.0f, 0.0f};
            al::verticalizeVec(&velocity, groundNormal, al::getVelocity(actor));
            velocity -= groundNormal * pConst->getSwimGravityWalk();
            al::setVelocity(actor, velocity);
            al::setNerve(this, &NrvPlayerStateSwim.SwimWalk);
        } else {
            rs::waitGround(actor, collision, pConst->getSwimGravity(),
                           pConst->getSwimFallSpeedMax(), 0.0f, 0.0f);
            al::setNerve(this, &NrvPlayerStateSwim.SwimLand);
        }
        return;
    }

    if (isMove) {
        _ac = 15;
        if (!mAnimator->isAnim("SwimStandMove"))
            mAnimator->startAnim("SwimStandMove");
    } else if (mAnimator->isAnim("SwimStand")) {
        if (mAnimator->isAnimEnd())
            mAnimator->startAnim("SwimStandWait");
    } else if (_ac <= 0 && !mAnimator->isAnim("SwimStandWait")) {
        mAnimator->startAnim("SwimStandWait");
    }
    s32 next = _ac - 1;
    if (next < 0)
        next = 0;
    _ac = next;
    return;
}

namespace {
// NON_MATCHING: target is the unlabeled 512-byte helper at 0x7100486834; source recovers
// its complete observed velocity split/ground-normal/update behavior. Exact symbol selection is
// unavailable because the corpus/file list has no label for this target function.
void updateSwimVelocity(al::LiveActor* actor, sead::Vector3f* normal, const PlayerConst* pConst,
                        const PlayerInput* input, const IUsePlayerCollision* collision,
                        const PlayerTrigger* trigger, s32 step) {
    sead::Vector3f previousNormal = *normal;
    sead::Vector3f velocityV = {0.0f, 0.0f, 0.0f};
    sead::Vector3f velocityH = {0.0f, 0.0f, 0.0f};

    f32 gravity = pConst->getSwimGravity();
    if (rs::calcOnGroundNormalOrGravityDir(normal, actor, collision)) {
        al::separateVelocityDirHV(&velocityH, &velocityV, actor, previousNormal);
        al::alongVectorNormalH(&velocityH, velocityH, previousNormal, *normal);
        f32 speedV = previousNormal.dot(velocityV);
        velocityV = *normal * speedV;
        gravity = pConst->getSwimGravityWalk();
    } else {
        rs::scaleVelocityInertiaWallHit(actor, collision, 0.75f,
                                        pConst->getSwimWallHitSpeedMinH(),
                                        pConst->getSwimHighSpeedMaxH());
        al::separateVelocityDirHV(&velocityH, &velocityV, actor, *normal);
    }

    updateSwimVelocityV(&velocityV, pConst, collision, trigger, *normal, step, gravity);
    bool isHighAccel = pConst->getSwimHighAccelPermitFrame() > static_cast<u32>(step);
    updateSwimVelocityH(&velocityH, actor, pConst, input, collision, isHighAccel, false, false);
    al::setVelocity(actor, velocityH + velocityV);
}

void updateSwimJointParams(PlayerJointParamSwim* joint, al::LiveActor* actor,
                           const PlayerConst* pConst, PlayerActionTurnControl* turnControl) {
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    al::calcFrontDir(&front, actor);
    f32 speedRate =
        al::calcRate01(front.dot(al::getVelocity(actor)), 0.0f, pConst->getSwimLowSpeedMaxH());

    f32 bentForward = speedRate * pConst->getSwimBentForwardMax();
    f32 forwardBlend = pConst->getSwimBentForwardBlendRate();
    joint->_0 = al::lerpValue(joint->_0, bentForward, forwardBlend);

    f32 turnRate = speedRate * turnControl->calcTurnPowerRate(-al::getGravity(actor));
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


// NON_MATCHING: target/current are both 1300 bytes (325 instructions). Behavior and helper factoring
// are recovered; remaining mismatch is the 0xC0 target vs 0xD0 current frame/stack coloring around
// SafeString and velocity temporaries. Next hypothesis is original local scope/lifetime grouping.
void PlayerStateSwim::exeSwimWalk() {
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("SwimWalk");
        _f0 = 0;
    }

    al::LiveActor* actor = mActor;
    const PlayerConst* pConst = mConst;
    const IUsePlayerCollision* collision = mCollision;
    const PlayerInput* input = mInput;
    const PlayerTrigger* trigger = mTrigger;
    s32 step = _a0;
    sead::Vector3f previousNormal = _d8;

    sead::Vector3f velocityH = {0.0f, 0.0f, 0.0f};
    sead::Vector3f velocityV = {0.0f, 0.0f, 0.0f};
    al::separateVelocityDirHV(&velocityH, &velocityV, actor, previousNormal);

    sead::Vector3f snapPosition = {0.0f, 0.0f, 0.0f};
    bool isSnapped = rs::calcSnapGroundNormalOrGravityDir(
        &_d8, &snapPosition, actor, collision, 15.0f, previousNormal);
    al::alongVectorNormalH(&velocityH, velocityH, previousNormal, _d8);
    f32 speedV = previousNormal.dot(velocityV);
    sead::Vector3f velocity;

    f32 gravity = 0.0f;
    if (isSnapped) {
        speedV = sead::Mathf::min(speedV, 0.0f);
        velocityV = _d8 * speedV;

        f32 offset = (snapPosition - al::getTrans(actor)).dot(_d8) - 7.0f;
        velocity = al::getTrans(actor) + _d8 * offset;
        al::setTrans(actor, velocity);
        gravity = pConst->getSwimGravityWalk();
    } else {
        velocityV = _d8 * speedV;
        gravity = pConst->getSwimGravity();
    }

    updateSwimVelocityV(&velocityV, pConst, collision, trigger, _d8, step, gravity);
    updateSwimVelocityH(&velocityH, actor, pConst, input, collision,
                        pConst->getSwimHighAccelPermitFrame() > static_cast<u32>(step), false, false);

    velocity = velocityH + velocityV;
    al::setVelocity(actor, velocity);
    f32 speedH = velocityH.length();

    updateSwimTurn(mActor, mTurnControl, mInput, mConst, _a0, 0.75f);

    const PlayerConst* jointConst = mConst;
    PlayerJointParamSwim* joint = mSwimJoint;
    f32 forwardBlend = jointConst->getSwimBentForwardBlendRate();
    joint->_0 = al::lerpValue(joint->_0, 0.0f, forwardBlend);
    f32 sideBlend = jointConst->getSwimBentSideBlendRate();
    joint->_4 = al::lerpValue(joint->_4, 0.0f, sideBlend);
    f32 sideBlend2 = jointConst->getSwimBentSideBlendRate();
    joint->_8 = al::lerpValue(joint->_8, 0.0f, sideBlend2);
    f32 frontBlend = jointConst->getSwimBentFrontBlendRate();
    joint->_c = al::lerpValue(joint->_c, 0.0f, frontBlend);

    if (mWaterSurfaceFinder->isFoundSurface() &&
        mWaterSurfaceFinder->getDistance() < mConst->getSwimSurfaceStartDist()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimSurface);
        return;
    }
    if (mInput->isTriggerPaddle()) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimPaddle);
        return;
    }
    if (mTrigger->isOn(PlayerTrigger::EAttackSensorTrigger_val2)) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimTrample);
        return;
    }
    if (mTrigger->isOn(PlayerTrigger::EAttackSensorTrigger_val1)) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimPaddle);
        return;
    }

    if (rs::isCollidedGround(mCollision)) {
        _f0 = 0;
    } else if (_f0++ >= 2) {
        al::setNerve(this, &NrvPlayerStateSwim.SwimMove);
        return;
    }

    if (mInput->isMove()) {
        if (!mAnimator->isAnim("SwimWalk"))
            mAnimator->startAnim("SwimWalk");
        mAnimControlSwimWalk->update(speedH);
    } else if (al::calcSpeedExceptDir(mActor, _d8) >= 2.0f) {
        if (mAnimator->isAnim("SwimWalk"))
            mAnimControlSwimWalk->update(speedH);
    } else if (!mAnimator->isAnim("SwimStandWait")) {
        mAnimator->startAnim("SwimStandWait");
    }
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
        if (static_cast<u32>(++_bc) >= static_cast<u32>(mConst->getSwimDiveEndFrame())) {
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
    if (al::isNearZeroOrLess(al::calcSpeedExceptDir(actor, gravity) - speedEnd))
        al::setNerve(this, &NrvPlayerStateSwim.SwimHipDropHeadSlidingEnd);
}

namespace {
void updateSwimHeadSliding(s32* counter, al::LiveActor* actor, const PlayerConst* pConst,
                           const al::WaterSurfaceFinder* waterSurfaceFinder) {
    {
        if (*counter != 0) {
            const al::WaterSurfaceState surface = waterSurfaceFinder->getSurfaceState();
            if (!surface.isFoundSurface ||
                surface.distance >= pConst->getSwimSurfaceEndDist()) {
                *counter = 0;
                return;
            }
            (*counter)++;
        } else {
            const al::WaterSurfaceState surface = waterSurfaceFinder->getSurfaceState();
            if (!surface.isFoundSurface ||
                !(surface.distance < pConst->getSwimSurfaceStartDist())) {
                return;
            }
        }
    }

    sead::Vector3f velocity;
    sead::Vector3f horizontal = {0.0f, 0.0f, 0.0f};
    sead::Vector3f vertical = {0.0f, 0.0f, 0.0f};
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
    velocity = up * speed + horizontal;
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
    if (al::isNearZeroOrLess(al::calcSpeedExceptDir(actor, gravity) - speedMin))
        al::setNerve(this, &NrvPlayerStateSwim.SwimMove);
}


// NON_MATCHING: target/current are both 1124 bytes (281 instructions) with the target 0xF0 frame,
// D8-D10 saves, FMAX clamp, >= surface-end branch, and EndSwimJump kill() virtual recovered. Remaining
// differences are the two -gravity argument-construction schedules around calcCapThrowInput() and
// startCapThrow(); next hypothesis is an original vector-expression form that interleaves callee loads.
void PlayerStateSwim::exeSwimSpinCapSurface() {
    al::LiveActor* actor = mActor;
    const sead::Vector3f& gravity = al::getGravity(actor);
    bool isSeparateSingleSpin = mSpinCapAttack->isSeparateSingleSpin();

    if (al::isFirstStep(this)) {
        if (isSeparateSingleSpin)
            mSpinCapAttack->startSpinSeparateSwimSurface(mAnimator);
        else
            mSpinCapAttack->startCapSpinAttackSwim(mAnimator, mInput);

        {
            sead::Vector3f up = -gravity;
            mInput->calcCapThrowInput(&_c0, up);
        }
        if (!al::tryNormalizeOrZero(&_c0)) {
            sead::Vector3f front = {0.0f, 0.0f, 0.0f};
            al::calcFrontDir(&front, actor);
            al::verticalizeVec(&_c0, gravity, front);
            if (!al::tryNormalizeOrZero(&_c0))
                _c0 = front;
        }
        al::faceToDirection(actor, _c0);

        sead::Matrix34f rootMtx = sead::Matrix34f::ident;
        mAnimator->calcModelJointRootMtx(&rootMtx);
        sead::Vector3f jointUp = {rootMtx.m[0][1], rootMtx.m[1][1], rootMtx.m[2][1]};
        al::normalize(&jointUp);
        sead::Vector3f jointFront = {rootMtx.m[0][0], rootMtx.m[1][0], rootMtx.m[2][0]};
        al::normalize(&jointFront);
        mSwimJoint->_0 =
            sead::Mathf::max(0.0f, al::calcAngleOnPlaneDegree(-gravity, jointUp, jointFront));
    }

    bool isSpinCapSurface = al::isLessEqualStep(this, mConst->getSwimSurfaceSpinCapFrame());
    updateSwimSurfaceVelocity(actor, &_d8, mConst, mInput, mCollision, mWaterSurfaceFinder,
                              al::getNerveStep(this), _a0, isSpinCapSurface);

    if (mWaterSurfaceFinder->isFoundSurface()) {
        sead::Vector3f displacement = mWaterSurfaceFinder->getSurfaceDisplacement();
        sead::Vector3f trans = displacement + al::getTrans(actor);
        al::setTrans(actor, trans);
    }

    updateSwimTurn(actor, mTurnControl, mInput, mConst, _a0, 1.0f);
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

    s32 throwFrame = mSpinCapAttack->getThrowFrameSwim();
    if (isSeparateSingleSpin) {
        if (!al::isGreaterEqualStep(this, throwFrame))
            return;
    } else {
        if (al::isStep(this, throwFrame) && mSpinCapAttack->isCapSpinAttack()) {
            sead::Vector3f up = -gravity;
            mSpinCapAttack->startCapThrow(_c0, up, 0.5f, false, sead::Vector3f::zero);
        }
        if (mSpinCapAttack->isCapSpinAttack())
            return;
    }

    if (rs::isJudge(mJudgeStartSwimJump)) {
        if (mAnimator->isSubAnimPlaying())
            mAnimator->endSubAnim();
        al::setNerve(this, &NrvPlayerStateSwim.EndSwimJump);
        kill();
        return;
    }

    if (mInput->isTriggerHipDrop()) {
        if (mAnimator->isSubAnimPlaying())
            mAnimator->endSubAnim();
        al::setNerve(this, &NrvPlayerStateSwim.SwimHipDropStart);
        return;
    }

    if (!mAnimator->isAnimEnd())
        return;
    if (!mWaterSurfaceFinder->isFoundSurface() ||
        mWaterSurfaceFinder->getDistance() >= mConst->getSwimSurfaceEndDist())
        al::setNerve(this, &NrvPlayerStateSwim.SwimMove);
    else
        al::setNerve(this, &NrvPlayerStateSwim.SwimSurface);
}

void PlayerStateSwim::exeSwimTrample() {
    sead::Vector3f groundNormal;
    sead::Vector3f velocity;
    if (al::isFirstStep(this)) {
        const PlayerConst* pConst = mConst;
        f32 intervalRate =
            1.0f - al::calcRate01(static_cast<f32>(_a0),
                                  static_cast<f32>(pConst->getSwimPaddleAnimRateIntervalMin()),
                                  static_cast<f32>(pConst->getSwimPaddleAnimRateIntervalMax()));
        intervalRate = sead::Mathf::clamp(intervalRate, 0.0f, 1.0f);
        _a4 = al::lerpValue(1.0f, pConst->getSwimPaddleAnimMaxRate(), intervalRate);
        _a0 = 0;
        _a8 = false;
        mAnimator->startAnim("SwimTrample");
    }

    updateSwimVelocity(mActor, &_d8, mConst, mInput, mCollision, mTrigger, _a0);
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
        bool isMove = mInput->isMove();
        const IUsePlayerCollision* collision = mCollision;
        const PlayerConst* pConst = mConst;
        al::LiveActor* actor = mActor;
        if (isMove) {
            groundNormal = {0.0f, 0.0f, 0.0f};
            rs::calcGroundNormalOrGravityDir(&groundNormal, actor, collision);
            velocity = {0.0f, 0.0f, 0.0f};
            al::verticalizeVec(&velocity, groundNormal, al::getVelocity(actor));
            velocity -= groundNormal * pConst->getSwimGravityWalk();
            al::setVelocity(actor, velocity);
            al::setNerve(this, &NrvPlayerStateSwim.SwimWalk);
        } else {
            rs::waitGround(actor, collision, pConst->getSwimGravity(),
                           pConst->getSwimFallSpeedMax(), 0.0f, 0.0f);
            al::setNerve(this, &NrvPlayerStateSwim.SwimLand);
        }
        return;
    }

    if (mAnimator->isAnimEnd() || al::calcSpeedV(mActor) < 0.0f)
        al::setNerve(this, &NrvPlayerStateSwim.SwimMove);
}



void PlayerStateSwim::exeSwimReflectDownFall() {
    al::LiveActor* player = mActor;
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("SwimTrample");
        const sead::Vector3f& gravity = al::getGravity(player);
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

    const PlayerConst* pConstSurface = mConst;
    if (mWaterSurfaceFinder->isFoundSurface() &&
        mWaterSurfaceFinder->getDistance() < pConstSurface->getSwimSurfaceStartDist()) {
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

namespace {
// NON_MATCHING: target is the unlabeled 872-byte helper at 0x710048AC80; current is 868 bytes.
// Behavior, 0xC0 frame, target stack-vector reuse, and the otherwise surprising acceleration.length()
// operation are recovered. Remaining mismatch is one split X/Y component store before limitLength.
void updateSwimVelocityH(sead::Vector3f* velocityH, al::LiveActor* actor,
                         const PlayerConst* pConst, const PlayerInput* input,
                         const IUsePlayerCollision* collision, bool isHighAccel, bool isSurface,
                         bool isSpinCapSurface) {
    f32 brakeRate = pConst->getSwimBrakeRateH();
    if (!input->isMove()) {
        *velocityH *= brakeRate;
        return;
    }

    const sead::Vector3f& gravity = al::getGravity(actor);
    sead::Vector3f moveInput = {0.0f, 0.0f, 0.0f};
    sead::Vector3f moveDir = -gravity;
    input->calcMoveInput(&moveInput, moveDir);

    moveDir.set(0.0f, 0.0f, 0.0f);
    al::tryNormalizeOrZero(&moveDir, moveInput);
    if (PlayerActionFunction::isOppositeVec(moveDir, *velocityH)) {
        *velocityH *= brakeRate;
    } else {
        sead::Vector3f front = {0.0f, 0.0f, 0.0f};
        sead::Vector3f gravityUp = -gravity;
        if (rs::calcAlongDirFront(&front, actor, gravityUp)) {
            al::scaleVectorExceptDirection(velocityH, front, *velocityH, brakeRate);
            if (!input->isMoveDeepDown()) {
                f32 directionScale = al::lerpValue(1.0f, brakeRate, moveInput.length());
                al::scaleVectorDirection(velocityH, front, *velocityH, directionScale);
            }
        }
    }

    sead::Vector3f acceleration;
    f32 maxSpeed;
    if (isSpinCapSurface) {
        acceleration = moveInput * pConst->getSwimSurfaceAccelH();
        maxSpeed = pConst->getSwimSurfaceSpinCapSpeedMaxH();
    } else if (isSurface) {
        acceleration = moveInput * pConst->getSwimSurfaceAccelH();
        maxSpeed = pConst->getSwimSurfaceSpeedMaxH();
    } else if (rs::isOnGround(actor, collision)) {
        acceleration = moveInput * pConst->getSwimFloorAccelH();
        maxSpeed = pConst->getSwimFloorSpeedMaxH();
    } else if (isHighAccel) {
        acceleration = moveInput * pConst->getSwimHighAccelH();
        maxSpeed = pConst->getSwimHighSpeedMaxH();
    } else {
        acceleration = moveInput * pConst->getSwimLowAccelH();
        maxSpeed = pConst->getSwimLowSpeedMaxH();
    }

    acceleration.length();
    f32 speed = velocityH->length();
    if (speed > maxSpeed) {
        f32 brakedSpeed = brakeRate * speed;
        if (brakedSpeed >= maxSpeed)
            maxSpeed = brakedSpeed;
    }

    velocityH->setAdd(*velocityH, acceleration);
    al::limitLength(velocityH, *velocityH, maxSpeed);
}

// NON_MATCHING: target is the unlabeled 364-byte helper at 0x710048AFE8; behavior and target call
// order are recovered from corpus. Exact selection is unavailable because the target has no label.
void updateSwimSurfaceVerticalSpeed(f32* speed, const al::WaterSurfaceFinder* waterSurfaceFinder,
                                    const PlayerConst* pConst, s32 counter, bool isMove) {
    f32 rate = static_cast<f32>(counter - pConst->getSwimSurfaceDamperStart()) /
               static_cast<f32>(pConst->getSwimSurfaceDamperFrame());
    if (rate >= 0.0f) {
        if (rate > 1.0f)
            rate = 1.0f;
    } else {
        rate = 0.0f;
    }

    if (!waterSurfaceFinder->isFoundSurface())
        return;

    if (isMove) {
        *speed *= pConst->getSwimSurfaceMoveDamper();
        *speed += (waterSurfaceFinder->getDistance() - pConst->getSwimSurfaceMoveBaseHeight()) *
                  pConst->getSwimSurfaceMoveSpring();
    } else {
        *speed *= rate + (1.0f - rate) * pConst->getSwimSurfaceDamper();
        *speed += (1.0f - rate) *
                  (waterSurfaceFinder->getDistance() - pConst->getSwimSurfaceBaseHeight()) *
                  pConst->getSwimSurfaceSpring();
    }

    *speed = sead::Mathf::min(*speed, pConst->getSwimRiseSpeedMax());
}

// NON_MATCHING: target/current are both 348 bytes and 87 instructions for the unlabeled helper at
// 0x710048B154. Corpus proves unsigned step comparison and setScale write shape; exact selection is
// unavailable because the target has no recovered label (MOV/ORR immediate aliases differ by disassembler).
void updateSwimVelocityV(sead::Vector3f* velocityV, const PlayerConst* pConst,
                         const IUsePlayerCollision* collision, const PlayerTrigger* trigger,
                         const sead::Vector3f& normal, u32 step, f32 gravity) {
    f32 speed = velocityV->dot(normal);
    if (rs::isCollidedGround(collision) && speed < 0.0f)
        speed = 0.0f;

    f32 targetSpeed;
    if (trigger->isOn(PlayerTrigger::EAttackSensorTrigger_val2)) {
        targetSpeed = pConst->getDiveTramplePower();
    } else if (trigger->isOn(PlayerTrigger::EAttackSensorTrigger_val1)) {
        targetSpeed = pConst->getSwimTramplePower();
    } else {
        f32 risePower = pConst->getSwimRisePower();
        if (pConst->getSwimHighAccelPermitFrame() <= static_cast<u32>(step)) {
            targetSpeed = speed - gravity;
            f32 fallSpeed = -pConst->getSwimFallSpeedMax();
            if (targetSpeed < fallSpeed)
                targetSpeed = fallSpeed;
        } else {
            targetSpeed = speed + risePower;
            f32 riseSpeedMax = pConst->getSwimRiseSpeedMax();
            if (targetSpeed > riseSpeedMax)
                targetSpeed = riseSpeedMax;
        }
    }

    velocityV->setScale(normal, targetSpeed);
}
}  // namespace
