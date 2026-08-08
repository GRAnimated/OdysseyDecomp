#include "Player/PlayerColliderHackCap.h"

#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"

#include "Player/CollisionShapeKeeper.h"
#include "Player/PlayerCollider.h"

PlayerColliderHackCap::PlayerColliderHackCap(al::LiveActor* player)
    : mPlayer(player) {}

void PlayerColliderHackCap::init() {
    mShapeKeeper = new CollisionShapeKeeper(1, 64, 32);
    mShapeKeeper->createShapeSphere("Body", 60.0f, sead::Vector3f::ey * 10.0f);
    mShapeKeeper->updateShape();

    mCollider = new PlayerCollider(mPlayer->getCollisionDirector(), mPlayer->getBaseMtx(),
                                   al::getTransPtr(mPlayer), &al::getGravity(mPlayer), false);
    mCollider->setCollisionShapeKeeper(mShapeKeeper);
}

void PlayerColliderHackCap::updateCollider(const sead::Vector3f& velocity) {
    mCollider->collide(velocity);
}

f32 PlayerColliderHackCap::getColliderRadius() const {
    return 60.0f;
}

PlayerCollider* PlayerColliderHackCap::getPlayerCollider() const {
    return mCollider;
}
