#include "Player/YoshiStateHackRun.h"

#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionGroundMoveControl.h"
#include "Player/PlayerActionPivotTurnControl.h"
#include "Player/PlayerAnimControlRun.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerCounterForceRun.h"
#include "Player/PlayerEffect.h"
#include "Player/PlayerAnimator.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackInputFunction.h"

namespace {
NERVE_END_IMPL(YoshiStateHackRun, Run)
NERVE_IMPL(YoshiStateHackRun, Brake)
NERVE_IMPL(YoshiStateHackRun, Turn)
NERVE_END_IMPL_(YoshiStateHackRun, RunAfterTurn, Run)
NERVE_IMPL(YoshiStateHackRun, Pivot)
NERVES_MAKE_STRUCT(YoshiStateHackRun, Run, Brake, Turn, RunAfterTurn, Pivot)
}  // namespace

// NON_MATCHING: target reloads mGroundMoveControl from +0x68 before storing its +0xC4 flag, while current reuses the fresh constructor result; all prior instructions match and current is 348 vs 352 bytes. Next source-level hypothesis: recover the original accessor/lifetime that ends the fresh-pointer value before set_c4 without forcing attributes.
YoshiStateHackRun::YoshiStateHackRun(al::LiveActor* player, IUsePlayerHack** playerHack,
                                     al::LiveActor* hackActor, const PlayerConst* playerConst,
                                     const IUsePlayerCollision* collision,
                                     const PlayerCounterForceRun* counterForceRun,
                                     PlayerAnimator* animator, PlayerEffect* effect)
    : HackerStateBase("Run", player, playerHack), mHackActor(hackActor),
      mPlayerConst(playerConst), mCollision(collision), mCounterForceRun(counterForceRun),
      mAnimator(animator), mEffect(effect) {
    mAnimControlRun = new PlayerAnimControlRun(animator, playerConst, nullptr, effect, false);
    mPivotTurnControl =
        new PlayerActionPivotTurnControl(player, mPlayerConst, nullptr, mCollision,
                                         mPlayerConst->getGravity());
    mPivotTurnControl->setPlayerHack(playerHack);
    mGroundMoveControl =
        new PlayerActionGroundMoveControl(player, mPlayerConst, nullptr, mCollision);
    mGroundMoveControl->setupHackRunFlags();
    mGroundMoveControl->set_c4(true);
    mGroundMoveControl->setPlayerHack(playerHack);
    initNerve(&NrvYoshiStateHackRun.Run);
}

void YoshiStateHackRun::appear() {
    HackerStateBase::appear();
    rs::startHitReactionLandIfLanding(mActor, mCollision, false);
    mIKBlendCounter = 0;
    mPoseRate = 1.0f;
    const PlayerConst* playerConst = mPlayerConst;
    mGroundMoveControl->setup(
        playerConst->getNormalMaxSpeed(), playerConst->getNormalMinSpeed(),
        playerConst->getNormalAccelFrame(), playerConst->getStickOnBrakeFrame(),
        playerConst->getNormalBrakeFrame(), playerConst->getGravityMove(),
        playerConst->getNormalMaxSpeed() * playerConst->getBrakeOnSpeedRate(),
        playerConst->getBrakeOnCounterBorder());
    mGroundMoveControl->appear();
    _74.set(0.0f, 0.0f, 0.0f);
    if (mAnimator->isSubAnimPlaying())
        mAnimator->endSubAnim();
    mIsTurnJump = false;
    al::setNerve(this, &NrvYoshiStateHackRun.Run);
}

void YoshiStateHackRun::kill() {
    HackerStateBase::kill();
    if (al::isVisAnimPlayingForAction(mHackActor, "DashFastMove"))
        al::startVisAnimForAction(mHackActor, "Wait");
    mEffect->clearRunEffect();
}

void YoshiStateHackRun::endRun() {
    if (al::isVisAnimPlayingForAction(mHackActor, "DashFastMove"))
        al::startVisAnimForAction(mHackActor, "Wait");
}

bool YoshiStateHackRun::tryTurnJump(sead::Vector3f* front) {
    if (isDead())
        return false;

    if (al::isNerve(this, &NrvYoshiStateHackRun.Brake) ||
        (al::isNerve(this, &NrvYoshiStateHackRun.Turn) && al::getNerveStep(this) <= 0)) {
        if (!mIsTurnJump)
            return false;
        al::calcFrontDir(front, mActor);
        front->negate();
        return true;
    }

    if (!al::isNerve(this, &NrvYoshiStateHackRun.Turn))
        return false;
    al::calcFrontDir(front, mActor);
    return true;
}

void YoshiStateHackRun::invalidateTurn() {
    mGroundMoveControl->setTurnInvalid(true);
}

void YoshiStateHackRun::validateTurn() {
    mGroundMoveControl->setTurnInvalid(false);
}

void YoshiStateHackRun::exePivot() {
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("TurnPoint");
        mPivotTurnControl->reset();
    }

    mIKBlendCounter = 0;
    mPoseRate = 0.0f;
    mPivotTurnControl->update();
    if (!mPivotTurnControl->isTurnFinished())
        return;

    if (mPivotTurnControl->hasMoveDirection())
        al::setNerve(this, &NrvYoshiStateHackRun.Run);
    else
        kill();
}

// NON_MATCHING: target 1144/current 1092 with the full 19/19 semantic call sequence. The
// reconstructed behavior is complete; remaining differences begin with ground-move/control value
// lifetimes and branch scheduling. Next source-level hypothesis: recover the target's narrower
// collaborator lifetimes without caching mGroundMoveControl before the first-step branch.
void YoshiStateHackRun::exeRun() {
    PlayerActionGroundMoveControl* groundMove = mGroundMoveControl;
    if (al::isFirstStep(this))
        groundMove->reset(groundMove->getGroundNormal());

    const PlayerCounterForceRun* forceRun = mCounterForceRun;
    groundMove->setForceRunControl(forceRun->isForceRun(), forceRun->getSpeed());

    f32 speedMax = _80;
    const PlayerConst* playerConst = mPlayerConst;
    if (al::isNerve(this, &NrvYoshiStateHackRun.RunAfterTurn)) {
        const s32 runAfterTurnFrame = playerConst->getRunAfterTurnFrame();
        const bool isInitialTurn = al::isLessEqualStep(this, runAfterTurnFrame);
        const f32 speedMin = playerConst->getNormalMinSpeed();
        s32 accelFrame;
        if (isInitialTurn) {
            accelFrame = playerConst->getRunAfterTurnFrame();
        } else {
            speedMax = playerConst->getNormalMaxSpeed();
            accelFrame = playerConst->getNormalAccelFrame();
        }
        groundMove->setup(speedMax, speedMin, accelFrame, playerConst->getStickOnBrakeFrame(),
                          playerConst->getNormalBrakeFrame(), playerConst->getGravityMove(),
                          playerConst->getNormalMaxSpeed() * playerConst->getBrakeOnSpeedRate(),
                          playerConst->getBrakeOnCounterBorder());
    }

    f32 speed = groundMove->update();
    if (rs::isOnGroundSkateCode(mActor, mCollision))
        speed = playerConst->getDashJudgeSpeed() + playerConst->getRunSkateAnimSpeedOffset();

    if (al::isFirstStep(this))
        mAnimControlRun->reset(speed, al::isNerve(this, &NrvYoshiStateHackRun.RunAfterTurn));
    mAnimControlRun->update(speed, groundMove->getMoveInput());

    al::tryStartVisAnimIfNotPlayingForAction(
        mHackActor, mEffect->isRunEffectDashFast() ? "DashFastMove" : "Wait");

    const s32 ikBlendFrame = playerConst->getIKBlendFrameRun();
    const f32 normalMaxSpeed = playerConst->getNormalMaxSpeed();
    const s32 ikBlendTarget =
        al::isNearZeroOrGreater(speed - normalMaxSpeed, 0.001f) ? ikBlendFrame : 0;
    mIKBlendCounter = al::converge(mIKBlendCounter, ikBlendTarget, 1);

    f32 ikBlendRate = 1.0f;
    if (ikBlendFrame >= 1)
        ikBlendRate = sead::Mathf::clamp(
            static_cast<f32>(mIKBlendCounter) / ikBlendFrame, 0.0f, 1.0f);

    const f32 speedRate = al::calcRate01(speed, playerConst->getNormalMinSpeed(), normalMaxSpeed);
    const f32 speedBlend = sead::Mathf::clamp(1.0f - speedRate, 0.0f, 1.0f);
    const f32 poseRate = al::lerpValue(playerConst->getIKBlendRateRunMin(),
                                       playerConst->getIKBlendRateRunMax(), speedBlend);
    mPoseRate = poseRate * sead::Mathf::clamp(1.0f - ikBlendRate, 0.0f, 1.0f);

    if (groundMove->isBrake2D()) {
        const f32 brakeBorder = playerConst->getRunBorderSpeed() - playerConst->getRunBlendRange();
        if (speed > brakeBorder) {
            al::setNerve(this, &NrvYoshiStateHackRun.Brake);
            return;
        }
        sead::Vector3f* velocity = al::getVelocityPtr(mActor);
        al::parallelizeVec(velocity, groundMove->getGroundNormal(), *velocity);
        kill();
        return;
    }

    if (groundMove->isPivotTurn()) {
        al::setNerve(this, &NrvYoshiStateHackRun.Pivot);
        return;
    }
    if (groundMove->isStopped())
        kill();
}

// NON_MATCHING: target/current 652/652 and instructions match through index 124; the remaining
// difference is CFG placement of the shared setNerve tail (target emits isLessStep before the
// merged Run/Turn setNerve block). Complete 11/11 semantic calls are present. Next source-level
// hypothesis: recover the original structured branch shape that naturally tail-merges both nerves.
void YoshiStateHackRun::exeBrake() {
    if (al::isFirstStep(this)) {
        if (mAnimator->isAnim("Dash"))
            mAnimator->startAnim("DashBrake");
        else
            mAnimator->startAnim("Brake");
        _80 = 0.0f;
        al::calcFrontDir(&_74, mActor);
        mIKBlendCounter = 0;
        mPoseRate = 0.0f;
    }

    f32 speed = 0.0f;
    rs::moveBrakeRun(&speed, &_74, mActor, mGroundMoveControl, mPlayerConst->getNormalMaxSpeed(),
                     mPlayerConst->getNormalBrakeFrame(), mPlayerConst->getGravityMove(),
                     mPlayerConst->getSlerpQuatRate(), mPlayerConst->getHillPoseDegreeMax());

    if (al::isFirstStep(this)) {
        _80 = sead::Mathf::clamp(speed, mPlayerConst->getNormalMaxSpeed(),
                                 mPlayerConst->getRunAfterTurnSpeedMax());
    }

    sead::Vector3f moveDir(0.0f, 0.0f, 0.0f);
    rs::calcHackerMoveDir(&moveDir, *mPlayerHack, mGroundMoveControl->getGroundNormal());
    const bool stickDeep = rs::isOnHackMoveStickDeepDown(*mPlayerHack);
    const f32 directionDot = _74.dot(moveDir);
    mIsTurnJump = stickDeep && directionDot <= 0.0f;

    if (stickDeep) {
        if (directionDot > 0.0f) {
            al::setNerve(this, &NrvYoshiStateHackRun.Run);
            return;
        }
        if (al::isGreaterEqualStep(this, mPlayerConst->getBrakeTurnStartFrame())) {
            al::setNerve(this, &NrvYoshiStateHackRun.Turn);
            return;
        }
    }

    if (!al::isLessStep(this, mPlayerConst->getNormalBrakeFrame()))
        kill();
}

// NON_MATCHING: target/current 548/548 with 9/9 semantic calls; only three commutative FMUL
// operand encodings differ in gravityNormal * gravityMove. Source lifetimes/frame/register sets
// otherwise match. Next hypothesis: a natural SDK vector-scaling form that preserves component *
// scalar operand order without manual component reconstruction.
void YoshiStateHackRun::exeTurn() {
    if (al::isFirstStep(this)) {
        if (mAnimator->isAnim("DashBrake"))
            mAnimator->startAnim("DashTurn");
        else
            mAnimator->startAnim("Turn");
        al::faceToDirectionSupportUp(mActor, -_74);
        mIKBlendCounter = 0;
        mPoseRate = 0.0f;
    }

    rs::moveBrakeRun(nullptr, &_74, mActor, mGroundMoveControl,
                     mPlayerConst->getNormalMaxSpeed(), mPlayerConst->getNormalBrakeFrame(),
                     mPlayerConst->getGravityMove(), mPlayerConst->getSlerpQuatRate(),
                     mPlayerConst->getHillPoseDegreeMax());

    if (!mAnimator->isAnimEnd())
        return;

    sead::Vector3f front(0.0f, 0.0f, 0.0f);
    al::calcFrontDir(&front, mActor);
    al::LiveActor* actor = mActor;
    PlayerActionGroundMoveControl* groundMove = mGroundMoveControl;
    const f32 gravityMove = mPlayerConst->getGravityMove();
    const sead::Vector3f gravityVelocity = groundMove->getGroundNormal() * gravityMove;
    const sead::Vector3f moveVelocity = _80 * front;
    const f32 runAfterTurnScale = mPlayerConst->getRunAfterTurnScale();
    const sead::Vector3f velocity = moveVelocity * runAfterTurnScale - gravityVelocity;
    al::setVelocity(actor, velocity);
    al::setNerve(this, &NrvYoshiStateHackRun.RunAfterTurn);
}
