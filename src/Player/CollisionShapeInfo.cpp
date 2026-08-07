#include "Player/CollisionShapeInfo.h"

#include "Library/Math/MathUtil.h"

CollisionShapeInfoBase::CollisionShapeInfoBase(CollisionShapeId id, const char* name)
    : mId(id), mName(name) {}

CollisionShapeInfoArrow::CollisionShapeInfoArrow(const char* name, const sead::Vector3f& start,
                                                 const sead::Vector3f& arrow, f32 radius,
                                                 s32 index)
    : CollisionShapeInfoBase(CollisionShapeId::Arrow, name), mIndex(index),
      mBoundingCenter(sead::Vector3f(0.0f, 0.0f, 0.0f)), mBoundingRadius(0.0f),
      mBoundingCenterWorld(sead::Vector3f(0.0f, 0.0f, 0.0f)), mBoundingRadiusWorld(0.0f), mStart(start),
      mArrow(arrow), mRadius(radius), mShapeOffset(sead::Vector3f(0.0f, 0.0f, 0.0f)),
      mStartWorld(sead::Vector3f(0.0f, 0.0f, 0.0f)), mEndWorld(sead::Vector3f(0.0f, 0.0f, 0.0f)),
      mArrowWorld(sead::Vector3f(0.0f, 0.0f, 0.0f)), mStartRelative(sead::Vector3f(0.0f, 0.0f, 0.0f)),
      mArrowRelative(sead::Vector3f(0.0f, 0.0f, 0.0f)) {
    sead::Vector3f center = mStart;
    center += mShapeOffset;
    center += mArrow * 0.5f;
    mBoundingCenter = center;
    mBoundingRadius = mArrow.length() * 0.5f;
    calcWorldShapeInfo(sead::Matrix34f::ident, 1.0f);
}

void CollisionShapeInfoArrow::updateShapeOffset(const sead::Vector3f& offset) {
    mShapeOffset = offset;
    sead::Vector3f center = mStart;
    const sead::Vector3f shapeOffset = mShapeOffset;
    center += shapeOffset;
    center += mArrow * 0.5f;
    mBoundingCenter = center;
    mBoundingRadius = mArrow.length() * 0.5f;
}

void CollisionShapeInfoArrow::calcWorldShapeInfo(const sead::Matrix34f& matrix, f32 scale) {
    mStartWorld.setMul(matrix, (mStart + mShapeOffset) * scale);
    mArrowWorld.setRotated(matrix, mArrow * scale);
    mBoundingCenterWorld.setMul(matrix, mBoundingCenter * scale);
    mBoundingRadiusWorld = mBoundingRadius * scale;
    mEndWorld = mStartWorld + mArrowWorld;
    al::calcArrowAabb(&mWorldAabb, mStartWorld, mEndWorld);
}

void CollisionShapeInfoArrow::calcRelativeShapeInfo(const sead::Matrix34f& matrix) {
    mStartRelative.setMul(matrix, mStartWorld);
    mArrowRelative.setRotated(matrix, mArrowWorld);
}

CollisionShapeInfoSphere::CollisionShapeInfoSphere(const char* name, f32 radius,
                                                   const sead::Vector3f& center)
    : CollisionShapeInfoBase(CollisionShapeId::Sphere, name), mBoundingRadius(radius),
      mBoundingCenter(center), mBoundingCenterRelative(sead::Vector3f(0.0f, 0.0f, 0.0f)),
      mIsSupportGround(false), mSupportGroundRange(0.0f),
      mUp(sead::Vector3f::ey), mIsIgnoreGround(false), _69(false) {
    calcWorldShapeInfo(sead::Matrix34f::ident, 1.0f);
}

void CollisionShapeInfoSphere::updateShapeOffset(const sead::Vector3f& offset) {
    mBoundingCenter = offset;
}

void CollisionShapeInfoSphere::calcWorldShapeInfo(const sead::Matrix34f& matrix, f32 scale) {
    mBoundingCenterWorld.setMul(matrix, mBoundingCenter * scale);
    mBoundingRadiusWorld = mBoundingRadius * scale;
    mSupportGroundRangeWorld = mSupportGroundRange * scale;
    mUpWorld.setRotated(matrix, mUp);
}

void CollisionShapeInfoSphere::calcRelativeShapeInfo(const sead::Matrix34f& matrix) {
    mBoundingCenterRelative.setMul(matrix, mBoundingCenterWorld);
}

CollisionShapeInfoDisk::CollisionShapeInfoDisk(const char* name, f32 radius,
                                               const sead::Vector3f& center,
                                               const sead::Vector3f& axis, f32 halfHeight)
    : CollisionShapeInfoBase(CollisionShapeId::Disk, name), mBoundingRadius(0.0f),
      mBoundingRadiusWorld(0.0f), mCheckStepRange(0.0f), mCheckStepRangeWorld(0.0f),
      mRadius(0.0f), mRadiusWorld(0.0f), mBoundingCenter(center),
      mAxis(sead::Vector3f::ey), mHalfHeight(0.0f), mHalfHeightWorld(0.0f),
      mBoundingCenterWorld(sead::Vector3f(0.0f, 0.0f, 0.0f)), mAxisWorld(sead::Vector3f(0.0f, 0.0f, 0.0f)),
      mBoundingCenterRelative(sead::Vector3f(0.0f, 0.0f, 0.0f)), mAxisRelative(sead::Vector3f(0.0f, 0.0f, 0.0f)),
      mIsSupportGround(false), mSupportGroundRange(0.0f), mSupportGroundRangeWorld(0.0f),
      mUp(sead::Vector3f::ey), mUpWorld(sead::Vector3f::ey), mIsIgnoreGround(false) {
    updateDiskShape(radius, axis, halfHeight);
    calcWorldShapeInfo(sead::Matrix34f::ident, 1.0f);
}

void CollisionShapeInfoDisk::updateDiskShape(f32 radius, const sead::Vector3f& axis,
                                             f32 halfHeight) {
    mRadius = radius;
    mAxis = axis;
    mHalfHeight = halfHeight;
    mBoundingRadius = sead::Vector2f(mRadius, mHalfHeight).length();
    mCheckStepRange = sead::Mathf::min(mRadius, mHalfHeight + mHalfHeight);
}

void CollisionShapeInfoDisk::updateShapeOffset(const sead::Vector3f& offset) {
    mBoundingCenter = offset;
}

void CollisionShapeInfoDisk::calcWorldShapeInfo(const sead::Matrix34f& matrix, f32 scale) {
    mBoundingCenterWorld.setMul(matrix, mBoundingCenter * scale);
    mAxisWorld.setRotated(matrix, mAxis);
    al::tryNormalizeOrZero(&mAxisWorld);
    mRadiusWorld = mRadius * scale;
    mHalfHeightWorld = mHalfHeight * scale;
    mBoundingRadiusWorld = mBoundingRadius * scale;
    mCheckStepRangeWorld = mCheckStepRange * scale;
    mSupportGroundRangeWorld = mSupportGroundRange * scale;
    mUpWorld.setRotated(matrix, mUp);
}

void CollisionShapeInfoDisk::calcRelativeShapeInfo(const sead::Matrix34f& matrix) {
    mBoundingCenterRelative.setMul(matrix, mBoundingCenterWorld);
    mAxisRelative.setRotated(matrix, mAxisWorld);
    al::tryNormalizeOrZero(&mAxisRelative);
}

const sead::Vector3f& CollisionShapeInfoArrow::getBoundingCenter() const {
    return mBoundingCenter;
}

const sead::Vector3f& CollisionShapeInfoArrow::getBoundingCenterWorld() const {
    return mBoundingCenterWorld;
}

f32 CollisionShapeInfoArrow::getBoundingRadius() const {
    return mBoundingRadius;
}

f32 CollisionShapeInfoArrow::getBoundingRadiusWorld() const {
    return mBoundingRadiusWorld;
}

f32 CollisionShapeInfoArrow::getCheckStepRange() const {
    return mBoundingRadius;
}

f32 CollisionShapeInfoArrow::getCheckStepRangeWorld() const {
    return mBoundingRadiusWorld;
}

const sead::Vector3f& CollisionShapeInfoSphere::getBoundingCenter() const {
    return mBoundingCenter;
}

const sead::Vector3f& CollisionShapeInfoSphere::getBoundingCenterWorld() const {
    return mBoundingCenterWorld;
}

f32 CollisionShapeInfoSphere::getBoundingRadius() const {
    return mBoundingRadius;
}

f32 CollisionShapeInfoSphere::getBoundingRadiusWorld() const {
    return mBoundingRadiusWorld;
}

f32 CollisionShapeInfoSphere::getCheckStepRange() const {
    return mBoundingRadius;
}

f32 CollisionShapeInfoSphere::getCheckStepRangeWorld() const {
    return mBoundingRadiusWorld;
}

const sead::Vector3f& CollisionShapeInfoDisk::getBoundingCenter() const {
    return mBoundingCenter;
}

const sead::Vector3f& CollisionShapeInfoDisk::getBoundingCenterWorld() const {
    return mBoundingCenterWorld;
}

f32 CollisionShapeInfoDisk::getBoundingRadius() const {
    return mBoundingRadius;
}

f32 CollisionShapeInfoDisk::getBoundingRadiusWorld() const {
    return mBoundingRadiusWorld;
}

f32 CollisionShapeInfoDisk::getCheckStepRange() const {
    return mCheckStepRange;
}

f32 CollisionShapeInfoDisk::getCheckStepRangeWorld() const {
    return mCheckStepRangeWorld;
}
