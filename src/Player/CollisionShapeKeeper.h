#pragma once

#include <container/seadPtrArray.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

class CollisionShapeInfoBase;
class CollisionShapeInfoArrow;
class CollisionShapeInfoSphere;
class CollisionShapeInfoDisk;
class CollidedShapeResult;

class CollisionShapeKeeper {
public:
    friend class CollisionMultiShape;
    CollisionShapeKeeper(s32 maxShapes, s32 maxCollideResults, s32 maxCollideSupportResults);
    void createShapeArrow(const char* name, const sead::Vector3f& start,
                          const sead::Vector3f& arrow, f32 radius, s32 index);
    void createShapeSphere(const char* name, f32 radius, const sead::Vector3f& center);
    void createShapeSphereSupportGround(const char* name, f32 radius,
                                        const sead::Vector3f& center,
                                        const sead::Vector3f& up, f32 range);
    void createShapeSphereIgnoreGround(const char* name, f32 radius,
                                       const sead::Vector3f& center);
    void createShapeDisk(const char* name, f32 radius, const sead::Vector3f& center,
                         const sead::Vector3f& axis, f32 halfHeight);
    void createShapeDiskSupportGround(const char* name, f32 radius,
                                      const sead::Vector3f& center,
                                      const sead::Vector3f& axis, f32 halfHeight,
                                      const sead::Vector3f& up, f32 range);
    void createShapeDiskIgnoreGround(const char* name, f32 radius,
                                     const sead::Vector3f& center,
                                     const sead::Vector3f& axis, f32 halfHeight);
    void updateShape();
    void clearResult();
    void calcWorldShapeInfo(const sead::Matrix34f& matrix, f32 scale);
    void calcRelativeShapeInfo(const sead::Matrix34f& matrix);
    void registerCollideResult(const CollidedShapeResult& result);
    void registerCollideSupportResult(const CollidedShapeResult& result);
    bool isCollidedResultFull() const;
    bool isCollidedSupportResultFull() const;
    bool isShapeArrow(s32 index) const;
    bool isShapeSphere(s32 index) const;
    bool isShapeDisk(s32 index) const;
    const CollisionShapeInfoBase* getShapeInfoBase(s32 index) const;
    CollisionShapeInfoArrow* getShapeInfoArrow(s32 index) const;
    CollisionShapeInfoSphere* getShapeInfoSphere(s32 index) const;
    CollisionShapeInfoDisk* getShapeInfoDisk(s32 index) const;
    u32 findShapeInfoIndex(const char* name) const;
    const CollidedShapeResult* getCollidedShapeResult(s32 index) const;
    const CollidedShapeResult* getCollidedShapeSupportResult(s32 index) const;

    s32 getNumCollidedShapeResults() const { return mNumCollideResult; }

    s32 getNumCollidedShapeSupportResults() const { return mNumCollideSupportResult; }

    const sead::Vector3f& getBoundingCenter() const { return mBoundingCenter; }

    f32 getBoundingRadius() const { return mBoundingRadius; }

    f32 getCheckStepRange() const { return mCheckStepRange; }

    f32 get54() const { return _54; }

    f32 get58() const { return _58; }

    bool hasShapeArrow() const { return mHasShapeArrow; }

    void set54(f32 value) { _54 = value; }

    void set58(f32 value) { _58 = value; }

private:
    sead::Vector3f mBoundingCenter = {0.0f, 0.0f, 0.0f};
    f32 mBoundingRadius = 0.0f;
    sead::PtrArray<CollisionShapeInfoBase> mCollisionShape;
    s32 mNumCollideResult = 0;
    sead::PtrArray<CollidedShapeResult> mCollideShapeResult;
    s32 mNumCollideSupportResult = 0;
    sead::PtrArray<CollidedShapeResult> mCollideSupportResult;
    f32 mCheckStepRange = 35.0f;
    f32 _54 = 0.0f;
    f32 _58 = 0.0f;
    bool mHasShapeArrow = false;
};

static_assert(sizeof(CollisionShapeKeeper) == 0x60);
