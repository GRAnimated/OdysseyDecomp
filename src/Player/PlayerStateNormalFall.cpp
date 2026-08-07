#include "Player/PlayerStateNormalFall.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(PlayerStateNormalFall, Fall)
NERVES_MAKE_STRUCT(PlayerStateNormalFall, Fall)
}  // namespace

PlayerStateNormalFall::PlayerStateNormalFall(al::LiveActor* player, const PlayerConst* pConst,
                                             const IUsePlayerCollision* collision,
                                             const PlayerInput* input,
                                             PlayerAnimator* animator)
    : al::ActorStateBase("落下", player), mConst(pConst), mCollision(collision),
      mAnimator(animator), mAirMoveControl(nullptr) {
    if (input)
        mAirMoveControl = new PlayerActionAirMoveControl(player, pConst, input, collision, false);
    initNerve(&NrvPlayerStateNormalFall.Fall, 0);
}

PlayerStateNormalFall::~PlayerStateNormalFall() = default;

void PlayerStateNormalFall::appear() {
    al::NerveStateBase::appear();
    al::setNerve(this, &NrvPlayerStateNormalFall.Fall);
}

void PlayerStateNormalFall::exeFall() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        if (mAnimator)
            mAnimator->startAnim("Fall");
        else
            al::startAction(actor, "Fall");

        if (mAirMoveControl)
            mAirMoveControl->setup(100.0f, mConst->getJumpMoveSpeedMin(), 0, 0.0f,
                                   mConst->getJumpGravity(), 0,
                                   mConst->getJumpInertiaRate());
    }

    if (mAirMoveControl)
        mAirMoveControl->update();

    bool isOnGround = mCollision ? rs::isOnGround(actor, mCollision) : al::isOnGround(actor, 0);
    if (!isOnGround)
        return;

    if (mCollision)
        rs::reboundVelocityFromCollision(actor, mCollision, 0.0f, 0.0f, 1.0f);
    else
        al::reboundVelocityFromCollision(actor, 0.0f, 0.0f, 1.0f);
    kill();
}
