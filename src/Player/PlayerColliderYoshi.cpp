#include "Player/PlayerColliderYoshi.h"

#include <math/seadQuat.h>

#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"

#include "Player/CollisionShapeKeeper.h"
#include "Player/PlayerCeilingCheck.h"
#include "Player/PlayerCollider.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
void createCollisionShapeArrows(CollisionShapeKeeper* collisionShapeKeeper) {
    const char* shapeNames[] = {"LegFront", "LegLeft", "LegRight"};
    sead::Vector3f up = sead::Vector3f::ey;
    sead::Vector3f upper = -40.0f * up;
    sead::Vector3f lower = 20.0f * up;
    sead::Vector3f radial = 30.0f * sead::Vector3f::ez;

    for (s32 i = 0; i < 3; i++) {
        sead::Quatf quat;
        sead::QuatCalcCommon<f32>::setAxisAngle(quat, sead::Vector3f::ey, i * 120.0f);
        sead::Vector3f start;
        start.setRotated(quat, radial);
        start += sead::Vector3f::zero;
        start -= upper;
        collisionShapeKeeper->createShapeArrow(shapeNames[i], start, upper - lower, 20.0f, i);
    }
}
}  // namespace

PlayerColliderYoshi::PlayerColliderYoshi(al::LiveActor* actor) : mActor(actor) {}

void PlayerColliderYoshi::init() {
    mCollisionNormal = new CollisionShapeKeeper(5, 64, 32);
    mCollisionNormal->set54(25.0f);
    mCollisionNormal->set58(25.0f);
    createCollisionShapeArrows(mCollisionNormal);
    mCollisionNormal->createShapeSphereSupportGround("Body", 55.0f, sead::Vector3f::ey * 65.0f,
                                                     sead::Vector3f::ey, 14.5f);
    mCollisionNormal->createShapeSphere("Head", 55.0f, sead::Vector3f::ey * 105.0f);
    mCollisionNormal->updateShape();

    mCollisionTongueJump = new CollisionShapeKeeper(7, 64, 32);
    mCollisionTongueJump->set54(25.0f);
    mCollisionTongueJump->set58(25.0f);
    createCollisionShapeArrows(mCollisionTongueJump);
    mCollisionTongueJump->createShapeSphereSupportGround(
        "Body", 55.0f, sead::Vector3f::ey * 65.0f, sead::Vector3f::ey, 14.5f);
    mCollisionTongueJump->createShapeSphere("HeadStand", 55.0f, sead::Vector3f::ey * 105.0f);
    mCollisionTongueJump->createShapeSphereIgnoreGround(
        "DiveBottom", 55.0f, sead::Vector3f::ey * 65.0f + sead::Vector3f::ez * 70.0f);
    mCollisionTongueJump->createShapeSphereIgnoreGround(
        "DiveTop", 55.0f, sead::Vector3f::ey * 105.0f + sead::Vector3f::ez * 70.0f);
    mCollisionTongueJump->updateShape();

    mPlayerCollider =
        new PlayerCollider(mActor->getCollisionDirector(), mActor->getBaseMtx(), al::getTransPtr(mActor),
                           al::getGravityPtr(mActor), false);
    mCeilingCheck = new PlayerCeilingCheck(mActor->getCollisionDirector());
    tryChangeCollisionNormal();
}

void PlayerColliderYoshi::tryChangeCollisionNormal() {
    if (mCurrentCollisionShape == mCollisionNormal)
        return;

    mPlayerCollider->setCollisionShapeKeeper(mCollisionNormal);
    mCeilingCheck->setupCeilingCheckNormal();
    mCurrentCollisionShape = mCollisionNormal;
}

void PlayerColliderYoshi::updateCollider(const sead::Vector3f& velocity) {
    mPlayerCollider->collide(velocity);
}

void PlayerColliderYoshi::updateHeightCheck(const sead::Vector3f& position,
                                             const sead::Vector3f& gravity) {
    mGroundHeight = 1000.0f;
    sead::Vector3f groundNormal(0.0f, 0.0f, 0.0f);
    mIsAboveGround = rs::calcGroundHeight(&mGroundHeight, &groundNormal, mPlayerCollider, position,
                                          gravity, 30.0f, 1000.0f);
    mShadowDropHeight = mGroundHeight;
}

void PlayerColliderYoshi::updateCeilingCheck(const sead::Vector3f& position,
                                              const sead::Vector3f& gravity, f32 height) {
    mCeilingCheck->update(position, gravity, 160.0f, 45.0f, height, 0.0f);
}

void PlayerColliderYoshi::tryChangeCollisionTongueJump() {
    if (mCurrentCollisionShape == mCollisionTongueJump)
        return;

    mPlayerCollider->setCollisionShapeKeeper(mCollisionTongueJump);
    mCeilingCheck->setupCeilingCheckNormal();
    rs::resetCollisionExpandCheck(this);
    mCurrentCollisionShape = mCollisionTongueJump;
}

bool PlayerColliderYoshi::isEnableStandUp() const {
    return mCeilingCheck->isEnableStandUp();
}

bool PlayerColliderYoshi::isEnableHoldUp() const {
    return mCeilingCheck->isEnableHoldUp();
}

bool PlayerColliderYoshi::isPressedCeil() const {
    return mCeilingCheck->isPressedCeil();
}

f32 PlayerColliderYoshi::getSafetyCeilSpace() const {
    return mCeilingCheck->getSafetyCeilSpace();
}

f32 PlayerColliderYoshi::getCeilCheckHeight() const {
    return mCeilingCheck->getCeilCheckHeight();
}

PlayerCollider* PlayerColliderYoshi::getPlayerCollider() const {
    return mPlayerCollider;
}

bool PlayerColliderYoshi::isAboveGround() const {
    return mIsAboveGround;
}

f32 PlayerColliderYoshi::getGroundHeight() const {
    return mGroundHeight;
}

f32 PlayerColliderYoshi::getShadowDropHeight() const {
    return mShadowDropHeight;
}
