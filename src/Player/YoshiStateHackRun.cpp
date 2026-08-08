#include "Player/YoshiStateHackRun.h"

#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionGroundMoveControl.h"
#include "Player/PlayerActionPivotTurnControl.h"
#include "Player/PlayerAnimControlRun.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerEffect.h"
#include "Player/PlayerAnimator.h"
#include "Util/PlayerCollisionUtil.h"

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
    initNerve(&NrvYoshiStateHackRun.Run, 0);
}

void YoshiStateHackRun::appear() {
    HackerStateBase::appear();
    rs::startHitReactionLandIfLanding(mActor, mCollision, false);
    _84 = 0.0f;
    _88 = 1.0f;
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

    _84 = 0.0f;
    _88 = 0.0f;
    mPivotTurnControl->update();
    if (!mPivotTurnControl->isTurnFinished())
        return;

    if (mPivotTurnControl->hasMoveDirection())
        al::setNerve(this, &NrvYoshiStateHackRun.Run);
    else
        kill();
}
