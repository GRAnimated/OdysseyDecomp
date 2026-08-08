#include "Player/YoshiStateHackTongueShrink.h"

#include "Library/Collision/PartsConnectorUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/Math/MatrixUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Player/YoshiActionTongueAttack.h"
#include "Player/YoshiTongue.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackInputFunction.h"

void YoshiStateHackTongueShrink::kill() {
    mCollisionParts = nullptr;
    al::disconnectMtxConnector(mMtxConnector);
    mActionTongueAttack->endShrink();
    _118 = 0;
    mLoopRunCount = 0;
    _104 = 0;
    _108 = 0;
    _110 = 0;
    HackerStateBase::kill();
}

void YoshiStateHackTongueShrink::setupGroundSnap(const al::CollisionParts* collisionParts,
                                                  const sead::Vector3f& position,
                                                  const sead::Vector3f& front,
                                                  const sead::Vector3f& up) {
    mCollisionParts = collisionParts;
    al::makeMtxFrontUpPos(&mSnapMtx, front, up, position);
    al::attachMtxConnectorToCollisionParts(mMtxConnector, collisionParts);
    _fc = false;
    _fd = false;
}

void YoshiStateHackTongueShrink::setupWallSnap(const al::CollisionParts* collisionParts,
                                                const sead::Vector3f& position,
                                                const sead::Vector3f& normal,
                                                const sead::Vector3f& up,
                                                bool isCollisionShapeJump) {
    mCollisionParts = collisionParts;
    al::makeMtxUpFrontPos(&mSnapMtx, up, -normal, position);
    al::attachMtxConnectorToCollisionParts(mMtxConnector, collisionParts);
    _fc = true;
    _fd = isCollisionShapeJump;
}

bool YoshiStateHackTongueShrink::isEnableAccelForceRun() const {
    return true;
}

bool YoshiStateHackTongueShrink::isJumpRolling() const {
    return mAnimator->isAnim("JumpRolling");
}

bool YoshiStateHackTongueShrink::isEndCancelForceRun() const {
    return isDead() && _fd && rs::isCollidedGround(mCollision) &&
           !rs::isOnHackMoveStick(*mPlayerHack);
}

s32 YoshiStateHackTongueShrink::getLoopRunCount() const {
    return mLoopRunCount;
}

void YoshiStateHackTongueShrink::exeFall() {
    if (al::isFirstStep(this)) {
        mAirMoveControl->setup(mPlayerConst->getJumpMoveSpeedMax(),
                               mPlayerConst->getJumpMoveSpeedMin(), 0, al::calcSpeedV(mActor),
                               mPlayerConst->getJumpGravity(), 0, 0.0f);
    }

    if (!mAnimator->isAnim("Fall"))
        mAnimator->startAnim("Fall");

    mAirMoveControl->update();
    if (rs::isOnGround(mActor, mCollision))
        kill();
}

YoshiStateHackTongueShrink::~YoshiStateHackTongueShrink() = default;

