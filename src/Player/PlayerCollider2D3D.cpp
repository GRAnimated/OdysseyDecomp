#include "Player/PlayerCollider2D3D.h"

#include "Library/Collision/Collider.h"
#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/CollisionPartsTriangle.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"

#include "Player/PlayerCollider.h"
#include "Player/PlayerColliderDisk.h"
#include "Player/PlayerCollisionCheckSphereMove.h"
#include "Player/PlayerConst.h"
#include "Util/PlayerCollisionUtil.h"

PlayerCollider2D3D::PlayerCollider2D3D(al::LiveActor* player, const PlayerConst* playerConst,
                                       PlayerCollider* collider, PlayerColliderDisk* colliderDisk)
    : mPlayer(player), mConst(playerConst), mIs2D(true), mCollider(collider),
      mColliderDisk(colliderDisk), mSphereMove(nullptr), mIsEnableStandUp(true),
      mIsAboveGround(false), mGroundHeight(0.0f), mFallStartPos(0.0f, 0.0f, 0.0f),
      mFallDistance(0.0f) {
    mColliderDisk->onInvalidate();
    mSphereMove = new PlayerCollisionCheckSphereMove(player->getCollisionDirector(), 16);
}

void PlayerCollider2D3D::setFlag2D(bool is2D) {}

sead::Vector3f PlayerCollider2D3D::updateCollider(const sead::Vector3f& velocity) {
    if (mIs2D)
        return mColliderDisk->collide(velocity);
    return mCollider->collide(velocity);
}

void PlayerCollider2D3D::updateHeightCheck(const sead::Vector3f& trans,
                                            const sead::Vector3f& up) {
    mIsAboveGround = false;
    mGroundHeight = 1000.0f;

    sead::Vector3f origin = trans + 30.0f * up;
    sead::Vector3f arrow = -2000.0f * up;
    s32 numHits = alCollisionUtil::checkStrikeArrow(mCollider, origin, arrow, nullptr, nullptr);
    for (s32 i = 0; i < numHits; i++) {
        const al::ArrowHitInfo& hitInfo = alCollisionUtil::getStrikeArrowInfo(mCollider, i);
        if ((*hitInfo)->triangle.getFaceNormal().dot(up) < 0.5f)
            continue;

        f32 distance = ((*hitInfo)->collisionHitPos - trans).length();
        if (distance < mGroundHeight) {
            mIsAboveGround = true;
            mGroundHeight = distance;
        }
    }

    if (mGroundHeight < 0.0f)
        mGroundHeight = 0.0f;
}

void PlayerCollider2D3D::updateCeilingCheck(const sead::Vector3f& trans,
                                             const sead::Vector3f& up) {
    f32 radius = mConst->getCollisionRadiusSquat();
    f32 checkHeight = mConst->getTall() - 2.0f * radius;
    mSphereMove->checkSphereMove(radius * up + trans, checkHeight * up, radius);

    f32 minT = 1.0f;
    for (u32 i = 0; i < mSphereMove->getNum(); i++) {
        if (!al::isCeilingPolygon(mSphereMove->getNormal(i), al::getGravity(mPlayer)))
            continue;

        f32 t = mSphereMove->getTValue(i);
        if (minT > t)
            minT = t;
    }
    mIsEnableStandUp = minT >= 0.99f;
}

void PlayerCollider2D3D::updateFallDistanceCheck(const sead::Vector3f& trans,
                                                  const sead::Vector3f& velocity,
                                                  const sead::Vector3f& gravity, f32 threshold) {
    if (rs::isOnGround(mPlayer, this) && !rs::isJustLand(this)) {
        mFallStartPos.set(0.0f, 0.0f, 0.0f);
        mFallDistance = 0.0f;
    } else {
        f32 velocityGravity = velocity.dot(gravity);
        if (velocityGravity < threshold) {
            mFallStartPos.set(0.0f, 0.0f, 0.0f);
            mFallDistance = 0.0f;
        } else {
            if (al::isNearZero(mFallStartPos, 0.001f))
                mFallStartPos.set(trans);

            sead::Vector3f fallVec = trans - mFallStartPos;
            f32 fallDistance = fallVec.dot(gravity);
            mFallDistance = fallDistance < 0.0f ? 0.0f : fallDistance;
        }
    }
}

void PlayerCollider2D3D::changeCollisionMini() {
    mColliderDisk->changeCollisionMini();
}

void PlayerCollider2D3D::changeCollisionNormal() {
    mColliderDisk->changeCollisionNormal();
}

void PlayerCollider2D3D::changeCollisionWallGrab() {
    mColliderDisk->changeCollisionWallGrab();
}

f32 PlayerCollider2D3D::getColliderRadius() const {
    f32 radius = 0.0f;
    mCollider->calcBoundingRadius(&radius);
    return radius;
}

f32 PlayerCollider2D3D::getColliderDiskHalfHeight() const {
    return mColliderDisk->getHalfHeight();
}

void PlayerCollider2D3D::calcDiskColliderBoundingCenter(sead::Vector3f* center) const {
    mColliderDisk->calcBoundingCenter(center);
}

PlayerCollider* PlayerCollider2D3D::getPlayerCollider() const {
    return mCollider;
}

bool PlayerCollider2D3D::isEnableStandUp() const {
    return mIsEnableStandUp;
}

bool PlayerCollider2D3D::isEnableHoldUp() const {
    return false;
}

bool PlayerCollider2D3D::isPressedCeil() const {
    return false;
}

bool PlayerCollider2D3D::isAboveGround() const {
    return mIsAboveGround;
}

f32 PlayerCollider2D3D::getGroundHeight() const {
    return mGroundHeight;
}

f32 PlayerCollider2D3D::getFallDistance() const {
    return mFallDistance;
}
