#include "Player/YoshiStateHackFall.h"

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"

namespace {
NERVE_IMPL(YoshiStateHackFall, Fall)
NERVES_MAKE_NOSTRUCT(YoshiStateHackFall, Fall)
}  // namespace


YoshiStateHackFall::YoshiStateHackFall(al::LiveActor* actor, IUsePlayerHack** playerHack,
                                       const PlayerConst* playerConst,
                                       const IUsePlayerCollision* collision,
                                       PlayerAnimator* animator)
    : HackerStateBase("待機", actor, playerHack), mPlayerConst(playerConst), mAnimator(animator) {
    mAirMoveControl = new PlayerActionAirMoveControl(actor, playerConst, nullptr, collision, false);
    mAirMoveControl->setPlayerHack(playerHack);
    mAirMoveControl->setupCollideWallScaleVelocity(mPlayerConst->getFallWallScaleVelocity(), 0.0f,
                                                   mPlayerConst->getNormalMaxSpeed());
    initNerve(&Fall, 0);
}

void YoshiStateHackFall::appear() {
    HackerStateBase::appear();
    al::setNerve(this, &Fall);
}

void YoshiStateHackFall::exeFall() {
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("Fall");
        mAirMoveControl->setup(mPlayerConst->getJumpMoveSpeedMax(),
                               mPlayerConst->getJumpMoveSpeedMin(), 0, al::calcSpeedV(mActor),
                               mPlayerConst->getJumpGravity(), 0, 0.0f);
    }

    mAirMoveControl->update();
}
