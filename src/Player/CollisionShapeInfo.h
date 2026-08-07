#pragma once

#include <math/seadBoundBox.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/HostIO/HioNode.h"

enum class CollisionShapeId : u32 {
    Arrow,
    Sphere,
    Disk,
};

class CollisionShapeInfoBase : public al::HioNode {
public:
    CollisionShapeInfoBase(CollisionShapeId id, const char* name);

    virtual const sead::Vector3f& getBoundingCenter() const { return sead::Vector3f::zero; }

    virtual const sead::Vector3f& getBoundingCenterWorld() const { return sead::Vector3f::zero; }

    virtual f32 getBoundingRadius() const { return 0.0f; }

    virtual f32 getBoundingRadiusWorld() const { return 0.0f; }

    virtual f32 getCheckStepRange() const { return 100000.0f; }

    virtual f32 getCheckStepRangeWorld() const { return 100000.0f; }

    virtual void updateShapeOffset(const sead::Vector3f&) {}

    virtual void calcWorldShapeInfo(const sead::Matrix34f&, f32) {}

    virtual void calcRelativeShapeInfo(const sead::Matrix34f&) {}

    CollisionShapeId getId() const { return mId; }

    const char* getName() const { return mName; }

private:
    CollisionShapeId mId;
    const char* mName;
};

static_assert(sizeof(CollisionShapeInfoBase) == 0x18);

class CollisionShapeInfoArrow : public CollisionShapeInfoBase {
public:
    CollisionShapeInfoArrow(const char* name, const sead::Vector3f& start,
                            const sead::Vector3f& arrow, f32 radius, s32 index);

    const sead::Vector3f& getBoundingCenter() const override;
    const sead::Vector3f& getBoundingCenterWorld() const override;
    f32 getBoundingRadius() const override;
    f32 getBoundingRadiusWorld() const override;
    f32 getCheckStepRange() const override;
    f32 getCheckStepRangeWorld() const override;
    void updateShapeOffset(const sead::Vector3f& offset) override;
    void calcWorldShapeInfo(const sead::Matrix34f& matrix, f32 scale) override;
    void calcRelativeShapeInfo(const sead::Matrix34f& matrix) override;

    s32 getIndex() const { return mIndex; }

    const sead::Vector3f& getStartWorld() const { return mStartWorld; }
    const sead::Vector3f& getEndWorld() const { return mEndWorld; }
    const sead::Vector3f& getArrowWorld() const { return mArrowWorld; }
    const sead::Vector3f& getStartRelative() const { return mStartRelative; }
    const sead::Vector3f& getArrowRelative() const { return mArrowRelative; }
    const sead::BoundBox3f& getWorldAabb() const { return mWorldAabb; }
    f32 getRadius() const { return mRadius; }

private:
    s32 mIndex;
    sead::Vector3f mBoundingCenter;
    f32 mBoundingRadius;
    sead::Vector3f mBoundingCenterWorld;
    f32 mBoundingRadiusWorld;
    sead::Vector3f mStart;
    sead::Vector3f mArrow;
    f32 mRadius;
    sead::Vector3f mShapeOffset;
    sead::Vector3f mStartWorld;
    sead::Vector3f mEndWorld;
    sead::Vector3f mArrowWorld;
    sead::Vector3f mStartRelative;
    sead::Vector3f mArrowRelative;
    sead::BoundBox3f mWorldAabb;
};

static_assert(sizeof(CollisionShapeInfoArrow) == 0xb8);

class CollisionShapeInfoSphere : public CollisionShapeInfoBase {
public:
    CollisionShapeInfoSphere(const char* name, f32 radius, const sead::Vector3f& center);

    const sead::Vector3f& getBoundingCenter() const override;
    const sead::Vector3f& getBoundingCenterWorld() const override;
    f32 getBoundingRadius() const override;
    f32 getBoundingRadiusWorld() const override;
    f32 getCheckStepRange() const override;
    f32 getCheckStepRangeWorld() const override;
    void updateShapeOffset(const sead::Vector3f& offset) override;
    void calcWorldShapeInfo(const sead::Matrix34f& matrix, f32 scale) override;
    void calcRelativeShapeInfo(const sead::Matrix34f& matrix) override;

    void setSupportGround(const sead::Vector3f& up, f32 range) {
        mIsSupportGround = true;
        mUp = up;
        mSupportGroundRange = range;
    }

    void setIgnoreGround() { mIsIgnoreGround = true; }

    void set48(f32 value) { mSupportGroundRange = value; }

    bool isSupportGround() const { return mIsSupportGround; }

    bool isIgnoreGround() const { return mIsIgnoreGround; }

    bool is69() const { return _69; }

    f32 getSupportGroundRangeWorld() const { return mSupportGroundRangeWorld; }

    const sead::Vector3f& getUpWorld() const { return mUpWorld; }

    const sead::Vector3f& getCenterRelative() const { return mBoundingCenterRelative; }

private:
    f32 mBoundingRadius;
    sead::Vector3f mBoundingCenter;
    sead::Vector3f mBoundingCenterWorld;
    f32 mBoundingRadiusWorld;
    sead::Vector3f mBoundingCenterRelative;
    bool mIsSupportGround;
    f32 mSupportGroundRange;
    f32 mSupportGroundRangeWorld;
    sead::Vector3f mUp;
    sead::Vector3f mUpWorld;
    bool mIsIgnoreGround;
    bool _69;
};

static_assert(sizeof(CollisionShapeInfoSphere) == 0x70);

class CollisionShapeInfoDisk : public CollisionShapeInfoBase {
public:
    CollisionShapeInfoDisk(const char* name, f32 radius, const sead::Vector3f& center,
                           const sead::Vector3f& axis, f32 halfHeight);

    void updateDiskShape(f32 radius, const sead::Vector3f& axis, f32 halfHeight);

    const sead::Vector3f& getBoundingCenter() const override;
    const sead::Vector3f& getBoundingCenterWorld() const override;
    f32 getBoundingRadius() const override;
    f32 getBoundingRadiusWorld() const override;
    f32 getCheckStepRange() const override;
    f32 getCheckStepRangeWorld() const override;
    void updateShapeOffset(const sead::Vector3f& offset) override;
    void calcWorldShapeInfo(const sead::Matrix34f& matrix, f32 scale) override;
    void calcRelativeShapeInfo(const sead::Matrix34f& matrix) override;

    void setSupportGround(const sead::Vector3f& up, f32 range) {
        mIsSupportGround = true;
        mUp = up;
        mSupportGroundRange = range;
    }

    void setIgnoreGround() { mIsIgnoreGround = true; }

    f32 getRadiusWorld() const { return mRadiusWorld; }
    f32 getHalfHeightWorld() const { return mHalfHeightWorld; }
    const sead::Vector3f& getCenterWorld() const { return mBoundingCenterWorld; }
    const sead::Vector3f& getAxisWorld() const { return mAxisWorld; }
    const sead::Vector3f& getCenterRelative() const { return mBoundingCenterRelative; }
    const sead::Vector3f& getAxisRelative() const { return mAxisRelative; }

    bool isSupportGround() const { return mIsSupportGround; }

    bool isIgnoreGround() const { return mIsIgnoreGround; }

    f32 getSupportGroundRangeWorld() const { return mSupportGroundRangeWorld; }

    const sead::Vector3f& getUpWorld() const { return mUpWorld; }

private:
    f32 mBoundingRadius;
    f32 mBoundingRadiusWorld;
    f32 mCheckStepRange;
    f32 mCheckStepRangeWorld;
    f32 mRadius;
    f32 mRadiusWorld;
    sead::Vector3f mBoundingCenter;
    sead::Vector3f mAxis;
    f32 mHalfHeight;
    f32 mHalfHeightWorld;
    sead::Vector3f mBoundingCenterWorld;
    sead::Vector3f mAxisWorld;
    sead::Vector3f mBoundingCenterRelative;
    sead::Vector3f mAxisRelative;
    bool mIsSupportGround;
    f32 mSupportGroundRange;
    f32 mSupportGroundRangeWorld;
    sead::Vector3f mUp;
    sead::Vector3f mUpWorld;
    bool mIsIgnoreGround;
};

static_assert(sizeof(CollisionShapeInfoDisk) == 0xa8);
