#include "Player/YoshiTongueCollider.h"

#include <math/seadMatrix.h>

#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"

#include "Player/CollisionShapeKeeper.h"
#include "Player/PlayerCollider.h"

// NON_MATCHING: target/current are both 256 bytes, but target saves the PlayerCollider allocation in X22 before loading the actor vtable; next source-level hypothesis is recovering the original allocation/argument helper expression.
YoshiTongueCollider::YoshiTongueCollider(al::LiveActor* actor) : mActor(actor) {
    CollisionShapeKeeper* shapeKeeper = new CollisionShapeKeeper(1, 0x40, 0x20);
    mShapeKeeper = shapeKeeper;
    shapeKeeper->createShapeSphere("Tongue", 20.0f, sead::Vector3f::zero);
    mShapeKeeper->updateShape();

    PlayerCollider* playerCollider =
        new PlayerCollider(actor->getCollisionDirector(), &sead::Matrix34f::ident, &mPosition,
                           al::getGravityPtr(actor), false);
    mPlayerCollider = playerCollider;
    playerCollider->setCollisionShapeKeeper(mShapeKeeper);
    al::onCollide(actor);
}

void YoshiTongueCollider::collide(const sead::Vector3f& position,
                                  const sead::Vector3f& direction) {
    mPosition = position;
    mPlayerCollider->collide(direction);
}

void YoshiTongueCollider::resetCollision(const sead::Vector3f& position) {
    mPosition = position;
    mPlayerCollider->onInvalidate();
}
