#include "Player/PlayerStateEndHack.h"

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerTrigger.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(PlayerStateEndHack, Jump)
NERVES_MAKE_STRUCT(PlayerStateEndHack, Jump)
}  // namespace
PlayerStateEndHack::PlayerStateEndHack(al::LiveActor* player, const PlayerConst* pConst,
                                       const IUsePlayerCollision* collision,
                                       const PlayerInput* input, PlayerTrigger* trigger,
                                       PlayerAnimator* animator)
    : al::ActorStateBase("", player), mConst(pConst), mCollision(collision), mInput(input),
      mTrigger(trigger), mAnimator(animator) {
    mAirMoveControl = new PlayerActionAirMoveControl(player, pConst, input, collision, false);
    initNerve(&NrvPlayerStateEndHack.Jump, 0);
}

void PlayerStateEndHack::appear() {
    al::setVelocity(mActor, mEndVelocity);
    al::NerveStateBase::appear();
    al::setNerve(this, &NrvPlayerStateEndHack.Jump);
}

void PlayerStateEndHack::exeJump() {
    const al::LiveActor* actor = mActor;

    if (al::isFirstStep(this)) {
        if (!mAnimator->isAnim("JumpEndHack"))
            mAnimator->startAnim("JumpEndHack");
        mAirMoveControl->setup(10000.0f, mConst->getNormalMaxSpeed(), 0, al::calcSpeedV(actor),
                               mConst->getJumpGravity(), mDelayFrames,
                               mConst->getJumpInertiaRate());
    }

    if (mTrigger->isOnUpperPunchHit())
        rs::reflectCeilingUpperPunch(mActor, mCollision, mInput, mConst, mTrigger, false);
    if (rs::isCollidedCeiling(mCollision))
        rs::reflectCeiling(mActor, 0.0f);

    mAirMoveControl->update();
    if (rs::isOnGround(actor, mCollision))
        kill();
}

PlayerStateEndHack::~PlayerStateEndHack() = default;

