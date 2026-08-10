#include "Player/YoshiTongueCollider.h"

#include <math/seadMatrix.h>

#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"

#include "Player/CollisionShapeKeeper.h"
#include "Player/PlayerCollider.h"

YoshiTongueCollider::YoshiTongueCollider(al::LiveActor* actor) : mActor(actor) {
    CollisionShapeKeeper* shapeKeeper = new CollisionShapeKeeper(1, 0x40, 0x20);
    mShapeKeeper = shapeKeeper;
    shapeKeeper->createShapeSphere("Tongue", 20.0f, sead::Vector3f::zero);
    mShapeKeeper->updateShape();

    PlayerCollider* playerCollider =
        new PlayerCollider(mActor->getCollisionDirector(), &sead::Matrix34f::ident, &mPosition,
                           al::getGravityPtr(mActor), false);
    mPlayerCollider = playerCollider;
    playerCollider->setCollisionShapeKeeper(mShapeKeeper);
    al::onCollide(actor);
}

sead::Vector3f YoshiTongueCollider::collide(const sead::Vector3f& position,
                                             const sead::Vector3f& direction) {
    mPosition = position;
    return mPlayerCollider->collide(direction);
}

void YoshiTongueCollider::resetCollision(const sead::Vector3f& position) {
    mPosition = position;
    mPlayerCollider->onInvalidate();
}
