#include "Player/PlayerColliderYoshi.h"

#include "Player/PlayerCeilingCheck.h"
#include "Player/PlayerCollider.h"
#include "Util/PlayerCollisionUtil.h"

PlayerColliderYoshi::PlayerColliderYoshi(al::LiveActor* actor) : mActor(actor) {}

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
