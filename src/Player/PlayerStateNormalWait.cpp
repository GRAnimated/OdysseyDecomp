#include "Player/PlayerStateNormalWait.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseKeeper.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(PlayerStateNormalWait, Wait)
NERVES_MAKE_STRUCT(PlayerStateNormalWait, Wait)
}  // namespace

PlayerStateNormalWait::PlayerStateNormalWait(al::LiveActor* player, const PlayerConst* pConst,
                                             const char* actionName,
                                             const IUsePlayerCollision* collision,
                                             PlayerAnimator* animator)
    : al::ActorStateBase("", player), mConst(pConst), mActionName(actionName),
      mCollision(collision), mAnimator(animator) {
    initNerve(&NrvPlayerStateNormalWait.Wait, 0);
}

PlayerStateNormalWait::~PlayerStateNormalWait() = default;

void PlayerStateNormalWait::appear() {
    al::NerveStateBase::appear();
    al::setNerve(this, &NrvPlayerStateNormalWait.Wait);
}

// NON_MATCHING: generated function is 0x18c versus target 0x194 after using aggregate zero
// initialization; next preserve the target's scalar ground-normal copy without manual components.
void PlayerStateNormalWait::exeWait() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        if (mAnimator)
            mAnimator->startAnim(mActionName);
        else
            al::startAction(actor, mActionName);
    }

    sead::Vector3f groundNormal;
    bool isGround;
    if (mCollision)
        isGround = rs::isCollidedGround(mCollision);
    else
        isGround = al::isCollidedGround(actor);
    if (isGround) {
        if (mCollision)
            groundNormal = rs::getCollidedGroundNormal(mCollision);
        else
            groundNormal = al::getCollidedGroundNormal(actor);
    } else {
        groundNormal = -al::getGravity(actor);
    }

    if (mCollision)
        rs::reboundVelocityFromCollision(actor, mCollision, 0.0f, 0.0f, 0.0f);
    else
        al::reboundVelocityFromCollision(actor, 0.0f, 0.0f, 0.0f);

    al::addVelocityToDirection(actor, -groundNormal, 3.0f);
    sead::Vector3f front = {};
    al::calcFrontDir(&front, actor);
    rs::slerpUpFront(actor, groundNormal, front, mConst->getSlerpQuatRate(),
                     mConst->getWaitPoseDegreeMax());
}
