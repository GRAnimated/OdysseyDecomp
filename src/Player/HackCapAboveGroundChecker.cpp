#include "Player/HackCapAboveGroundChecker.h"

#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"

HackCapAboveGroundChecker::HackCapAboveGroundChecker(const al::LiveActor* actor)
    : mActor(actor), mIsAboveGround(false), mGroundPosition(0.0f, 0.0f, 0.0f),
      mGroundNormal(0.0f, 0.0f, 0.0f), mGroundDistance(0.0f), mGravityDistance(0.0f) {}

void HackCapAboveGroundChecker::update(const sead::Vector3f& gravityDirection) {
    mIsAboveGround = false;
    sead::Vector3f arrow;
    arrow.setScale(gravityDirection, -200.0f);
    const sead::Vector3f& trans = al::getTrans(mActor);
    if (alCollisionUtil::getHitPosAndNormalOnArrow(mActor, &mGroundPosition, &mGroundNormal, trans,
                                                   arrow, nullptr, nullptr)) {
        mIsAboveGround = true;
        mGroundDistance = (trans - mGroundPosition).dot(gravityDirection);
        const sead::Vector3f& gravity = al::getGravity(mActor);
        mGravityDistance = (mGroundPosition - trans).dot(gravity);
    }
}
