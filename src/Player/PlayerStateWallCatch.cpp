#include "Player/PlayerStateWallCatch.h"

#include "Library/LiveActor/ActorPoseUtil.h"

#include "Player/IJudge.h"
#include "Player/PlayerActionCollisionSnap.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerJointParamGrab.h"
#include "Util/JudgeUtil.h"

bool PlayerStateWallCatch::update() {
    rs::updateJudge(reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing));
    return al::NerveStateBase::update();
}


void PlayerStateWallCatch::control() {
    _a0 = al::getTrans(mActor);
    mCollisionSnap->calcFollowDir(&mGrabJoint->direction, _80);
}

void PlayerStateWallCatch::setup(const al::CollisionParts* collisionParts,
                                 const sead::Vector3f& position,
                                 const sead::Vector3f& front,
                                 const sead::Vector3f& up) {
    mCollisionSnap->setup(collisionParts, position, front, up);
    _80 = up;
}


sead::Vector3f PlayerStateWallCatch::getWallCatchFront() const {
    sead::Vector3f front(0.0f, 0.0f, 0.0f);
    mCollisionSnap->calcFollowDir(&front, mCollisionSnap->getSnapFront());
    return front;
}


// NON_MATCHING: exact math is recovered; target keeps this in X19 and reloads mAnimator after getAnimFrameMax.
void PlayerStateWallCatch::initMoveFrameLeftRight() {
    const s32 moveFrame = 2 * mCollisionSnap->getMoveFrame() - 1;
    mAnimator->setAnimRate(mAnimator->getAnimFrameMax() / static_cast<f32>(moveFrame) * 0.999f);
}

void PlayerStateWallCatch::exeEndFall() {}

const sead::Vector3f& PlayerStateWallCatch::getCeilingCheckPos() const {
    return _a0;
}
