#include "Player/PlayerColliderDisk.h"

#include "Library/Collision/Collider.h"
#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/CollisionPartsTriangle.h"
#include "Library/Collision/PartsInterpolator.h"
#include "Library/Math/MathUtil.h"

#include "Player/CollidedShapeResult.h"
#include "Player/CollisionMultiShape.h"
#include "Player/CollisionShapeKeeper.h"

// NON_MATCHING: current is 876 bytes versus the 948-byte target with all 23 semantic calls in
// target order after compiler-generated memset normalization. The target emits three aggregate
// zeroing calls while current initialization lowers directly; next source-level hypothesis is a
// natural HitInfo/array initialization form that restores those aggregate lifetimes.
PlayerColliderDisk::PlayerColliderDisk(al::CollisionDirector* collisionDirector,
                                       const sead::Matrix34f* mtx,
                                       const sead::Vector3f* position,
                                       const sead::Vector3f* gravity, f32 radius, f32 height,
                                       const sead::Vector3f& axis, f32 halfHeight)
    : mCollisionDirector(collisionDirector), mMtxPtr(mtx), mPositionPtr(position),
      mGravityPtr(gravity), mRadius(radius), mOffset(axis), mHalfHeight(halfHeight),
      mPosition(*position), mRadiusCurrent(radius), mHalfHeightCurrent(halfHeight),
      mOffsetWorld(sead::Vector3f::ey), mMtx(sead::Matrix34f::ident),
      mHitInfo(new al::HitInfo), mHitDistance(0.0f), mHitInfo2(new al::HitInfo),
      mHitDistance2(0.0f), mHitInfo3(new al::HitInfo), mHitFrames(0),
      mCollisionMtx(sead::Matrix34f::ident), mCollisionNormal(nullptr),
      mCollisionMini(nullptr), mCollisionCurrent(nullptr), mCollisionMultiShape(nullptr),
      mHitInfoBuffer(nullptr), mHitInfoCount(0) {
    clear();
    mOffsetWorld.setRotated(*mMtxPtr, mOffset);
    mMtx = *mMtxPtr;

    mCollisionNormal = new CollisionShapeKeeper(2, 128, 0);
    mCollisionNormal->createShapeDisk("Body", mRadius, sead::Vector3f::ey * height, mOffset,
                                      mHalfHeight);
    mCollisionNormal->createShapeDisk("Head", mRadius, sead::Vector3f::ey * (height + 75.0f),
                                      mOffset, mHalfHeight);
    mCollisionNormal->updateShape();

    mCollisionMini = new CollisionShapeKeeper(2, 128, 0);
    mCollisionMini->createShapeDisk("Body", mRadius, sead::Vector3f::ey * height, mOffset,
                                    mHalfHeight);
    mCollisionMini->createShapeDisk("Head", mRadius, sead::Vector3f::ey * height, mOffset,
                                    mHalfHeight);
    mCollisionMini->updateShape();

    mCollisionCurrent = mCollisionNormal;
    mCollisionMultiShape = new CollisionMultiShape(this, 128);
    mHitInfoBuffer = new al::HitInfo[128];
    mHitInfoArray.allocBuffer(128, nullptr, 8);
}

void PlayerColliderDisk::clear() {
    mHitDistance = -99999.0f;
    mHitDistance2 = -99999.0f;
    mHitDistance3 = -99999.0f;
}

// NON_MATCHING: target performs the position copy as three scalar load/store pairs while the
// validator-safe sead copy coalesces it, leaving current 188 bytes versus 196. Next source-level
// hypothesis: find a sead vector copy idiom that preserves scalar component stores.
void PlayerColliderDisk::onInvalidate() {
    clear();
    mPosition.set(*mPositionPtr);
    mRadiusCurrent = mRadius;
    mHalfHeightCurrent = mHalfHeight;
    mOffsetWorld.setRotated(*mMtxPtr, mOffset);
    mMtx = *mMtxPtr;
}

// NON_MATCHING: target performs the output position copy as three scalar load/store pairs while
// validator-safe `set` coalesces it, leaving current 136 bytes versus 144. Next source-level
// hypothesis: find a sead copy helper/source shape that retains three scalar stores.
void PlayerColliderDisk::calcCheckPosAndDir(sead::Vector3f* checkPos,
                                             sead::Vector3f* checkDir) const {
    checkPos->set(*mPositionPtr);
    if (checkDir)
        checkDir->setRotated(*mMtxPtr, mOffset);
}

// NON_MATCHING: current is 1124 bytes versus the 1228-byte target with an exact 12/12 semantic
// direct-call sequence. Remaining differences are local vector/matrix lifetime and stack layout
// around the two moveCollide phases; next source-level hypothesis is target-shaped lifetime sharing
// between externalPosition, contactTarget, and the final correction vector.
sead::Vector3f PlayerColliderDisk::collide(const sead::Vector3f& velocity) {
    mHitInfoArray.clear();
    mHitInfoCount = 0;

    const sead::Vector3f externalPosition = *mPositionPtr;
    sead::Vector3f position = mPosition;
    f32 radius = mRadiusCurrent;
    f32 halfHeight = mHalfHeightCurrent;
    sead::Matrix34f mtx = mMtx;

    sead::Vector3f currentAxis;
    currentAxis.setRotated(mMtx, mOffset);

    sead::Vector3f contactMove(0.0f, 0.0f, 0.0f);
    calcMovePowerByContact(&contactMove, externalPosition);
    clear();

    const f32 firstCheckRange =
        sead::Mathf::min(sead::Mathf::min(mRadius * 0.9f, mRadiusCurrent * 0.9f), 35.0f);
    const sead::Vector3f contactTarget = externalPosition + contactMove;
    const sead::Vector3f contactDelta = contactTarget - position;
    sead::Vector3f targetAxis;
    targetAxis.setRotated(*mMtxPtr, mOffset);

    if (!al::isNearZero(contactDelta, 0.001f) ||
        !al::isNearZero(mRadiusCurrent - mRadius, 0.001f) ||
        !al::isNearZero(mHalfHeightCurrent - mHalfHeight, 0.001f) ||
        !al::isNearDirection(targetAxis, currentAxis, 0.01f)) {
        moveCollide(&position, &radius, &halfHeight, &mtx, contactTarget, mRadius, mHalfHeight,
                    mMtxPtr, contactDelta, firstCheckRange);
    }

    const sead::Vector3f moveTarget = position + velocity;
    const f32 moveCheckRange = sead::Mathf::min(mRadiusCurrent * 0.9f, 35.0f);
    moveCollide(&position, &radius, &halfHeight, &mtx, moveTarget, mRadius, mHalfHeight, mMtxPtr,
                velocity, moveCheckRange);

    const sead::Vector3f result = position - externalPosition;
    mPosition = position;
    mRadiusCurrent = mRadius;
    mHalfHeightCurrent = mHalfHeight;
    mOffsetWorld.setRotated(*mMtxPtr, mOffset);
    mMtx = *mMtxPtr;

    if (mHitDistance >= 0.0f) {
        if (mHitFrames < 100000)
            ++mHitFrames;
    } else {
        mHitFrames = 0;
    }

    return result;
}

bool PlayerColliderDisk::calcMovePowerByContact(sead::Vector3f* movePower,
                                                 const sead::Vector3f& contact) const {
    if (mHitDistance < 0.0f || !alCollisionUtil::isCollisionMoving(mHitInfo))
        return false;

    al::Triangle triangle = mHitInfo->triangle;
    triangle.calcForceMovePower(movePower, contact);
    const sead::Vector3f& faceNormal = triangle.getFaceNormal();
    if (faceNormal.dot(*movePower) > 0.0f)
        al::verticalizeVec(movePower, faceNormal, *movePower);
    return true;
}

// NON_MATCHING: target/current are both 724 bytes with matching 0x130-byte frames, 181/181
// instructions, and an exact 13/13 semantic direct-call sequence. Loading remainMove.x before the
// normalization-result branch matches the target lifetime and reduces the aligned diff from six to five
// FP-scheduling hunks; scalarized component rebuilds are validator-invalid, so the next hypothesis is a
// sanctioned vector/helper spelling that preserves this frame and operand schedule.
void PlayerColliderDisk::moveCollide(sead::Vector3f* position, f32* radius, f32* halfHeight,
                                     sead::Matrix34f* mtx,
                                     const sead::Vector3f& targetPosition, f32 targetRadius,
                                     f32 targetHalfHeight, const sead::Matrix34f* targetMtx,
                                     const sead::Vector3f& moveVector, f32 checkStepRange) {
    al::DiskInterpolator interpolator;
    interpolator.startInterp(*position, targetPosition, *radius, targetRadius, *halfHeight,
                             targetHalfHeight, mtx, targetMtx, mOffset, checkStepRange);

    if (!findCollidePos(&interpolator)) {
        sead::Vector3f offset(0.0f, 0.0f, 0.0f);
        interpolator.calcInterp(position, radius, halfHeight, &offset, mtx, nullptr);
        return;
    }

    while (true) {
        sead::Vector3f remainMove(0.0f, 0.0f, 0.0f);
        sead::Vector3f offset(0.0f, 0.0f, 0.0f);
        interpolator.calcInterp(position, radius, halfHeight, &offset, mtx, &remainMove);

        sead::Vector3f fix(0.0f, 0.0f, 0.0f);
        calcResultVec(&fix, remainMove);
        *position += fix;

        if (interpolator._48 >= 1.0f)
            break;

        sead::Vector3f fixDirection = fix;
        const bool hasFixDirection = al::tryNormalizeOrZero(&fixDirection);
        const f32 remainX = remainMove.x;
        if (hasFixDirection) {
            const f32 fixDot = fixDirection.x * remainX + fixDirection.y * remainMove.y +
                               fixDirection.z * remainMove.z;
            if (fixDot < 0.0f)
                remainMove -= fixDirection * fixDot;
        }

        const f32 moveDot = moveVector.dot(remainMove);
        if (moveDot < 0.0f && !al::isNearZero(moveDot, 0.001f))
            break;

        sead::Vector3f retryPosition = *position + remainMove;
        interpolator.startInterp(*position, retryPosition, *radius, targetRadius, *halfHeight,
                                 targetHalfHeight, mtx, targetMtx, mOffset, checkStepRange);
        interpolator.nextStep();

        retryPosition.set(0.0f, 0.0f, 0.0f);
        interpolator.calcInterpPos(&retryPosition);
        if (!findCollidePos(&interpolator)) {
            interpolator.calcInterp(position, radius, halfHeight, &offset, mtx, nullptr);
            break;
        }
    }
}

void PlayerColliderDisk::changeCollisionMini() {
    mCollisionCurrent = mCollisionMini;
}

void PlayerColliderDisk::changeCollisionNormal() {
    mCollisionCurrent = mCollisionNormal;
}

void PlayerColliderDisk::changeCollisionWallGrab() {}

bool PlayerColliderDisk::findCollidePos(al::DiskInterpolator* interpolator) {
    while (interpolator->_4c != 1.0f || interpolator->_48 != 1.0f) {
        sead::Vector3f position;
        f32 radius;
        f32 halfHeight;
        sead::Vector3f offset;
        sead::Vector3f velocity;
        sead::Matrix34f mtx;
        interpolator->calcInterp(&position, &radius, &halfHeight, &offset, &mtx, &velocity);

        mCollisionMtx = mtx;
        mCollisionMtx.setTranslation(position);
        if (mCollisionMultiShape->check(mCollisionCurrent, &mCollisionMtx, 1.0f, velocity,
                                        nullptr))
            return true;

        interpolator->nextStep();
    }
    return false;
}

// NON_MATCHING: current is 1728 bytes versus the 1736-byte target with an exact 6/6 semantic
// direct-call sequence. The ABI velocity parameter is present but unread by the target body; next
// source-level hypothesis is a pointer-valued duplicate search that reproduces the target PtrArray
// control shape while preserving the current per-axis fix behavior.
void PlayerColliderDisk::calcResultVec(sead::Vector3f* result, const sead::Vector3f& velocity) {
    (void)velocity;

    const sead::Vector3f gravity = *mGravityPtr;
    sead::Vector3f minFix(0.0f, 0.0f, 0.0f);
    sead::Vector3f maxFix(0.0f, 0.0f, 0.0f);

    const s32 resultCount = mCollisionCurrent->getNumCollidedShapeResults();
    for (s32 i = 0; i < resultCount; ++i) {
        const CollidedShapeResult* collideResult = mCollisionCurrent->getCollidedShapeResult(i);
        const al::DiskHitInfo& diskHit = collideResult->getDiskHitInfo();
        const al::HitInfo& hitInfo = **diskHit;
        const sead::Vector3f& normal = hitInfo.triangle.getNormal(0);

        sead::Vector3f fix(0.0f, 0.0f, 0.0f);
        sead::Vector3f fixNormal(0.0f, 0.0f, 0.0f);
        diskHit.calcFixVector(&fix, &fixNormal);

        minFix.set(sead::Mathf::min(minFix.x, fix.x), sead::Mathf::min(minFix.y, fix.y),
                   sead::Mathf::min(minFix.z, fix.z));
        maxFix.set(sead::Mathf::max(maxFix.x, fix.x), sead::Mathf::max(maxFix.y, fix.y),
                   sead::Mathf::max(maxFix.z, fix.z));

        if (al::isFloorPolygon(normal, gravity)) {
            if (mHitDistance < hitInfo._70) {
                *mHitInfo = hitInfo;
                mHitDistance = hitInfo._70;
            }
        } else if (al::isWallPolygon(normal, gravity)) {
            if (mHitDistance2 < hitInfo._70) {
                *mHitInfo2 = hitInfo;
                mHitDistance2 = hitInfo._70;
            }
        } else if (mHitDistance3 < hitInfo._70) {
            *mHitInfo3 = hitInfo;
            mHitDistance3 = hitInfo._70;
        }

        bool isRegistered = false;
        for (s32 j = 0; j < mHitInfoArray.size(); ++j) {
            if (mHitInfoArray[j]->triangle.getCollisionParts() ==
                hitInfo.triangle.getCollisionParts()) {
                isRegistered = true;
                break;
            }
        }

        if (!isRegistered && static_cast<u32>(mHitInfoCount) <
                                 static_cast<u32>(mHitInfoArray.capacity())) {
            mHitInfoBuffer[mHitInfoCount] = hitInfo;
            if (!mHitInfoArray.isFull())
                mHitInfoArray.pushBack(&mHitInfoBuffer[mHitInfoCount]);
            ++mHitInfoCount;
        }
    }

    result->set(minFix.x + maxFix.x, minFix.y + maxFix.y, minFix.z + maxFix.z);
}

void PlayerColliderDisk::calcBoundingCenter(sead::Vector3f* center) const {
    center->setRotated(*mMtxPtr, mCollisionCurrent->getBoundingCenter());
    center->add(*mPositionPtr);
}

al::CollisionDirector* PlayerColliderDisk::getCollisionDirector() const {
    return mCollisionDirector;
}
