#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Collision/IUseCollision.h"

namespace al {
class CollisionDirector;
class DiskInterpolator;
struct HitInfo;
}

class CollisionMultiShape;
class CollisionShapeKeeper;

class PlayerColliderDisk : public al::IUseCollision {
public:
    PlayerColliderDisk(al::CollisionDirector* collisionDirector, const sead::Matrix34f* mtx,
                       const sead::Vector3f* position, const sead::Vector3f* gravity, f32 radius,
                       f32 height, const sead::Vector3f& axis, f32 halfHeight);

    void clear();
    void onInvalidate();
    void calcCheckPosAndDir(sead::Vector3f* checkPos, sead::Vector3f* checkDir) const;
    sead::Vector3f collide(const sead::Vector3f& velocity);
    bool calcMovePowerByContact(sead::Vector3f* movePower, const sead::Vector3f& contact) const;
    void moveCollide(sead::Vector3f* position, f32* radius, f32* halfHeight, sead::Matrix34f* mtx,
                     const sead::Vector3f& targetPosition, f32 targetRadius, f32 targetHalfHeight,
                     const sead::Matrix34f* targetMtx, const sead::Vector3f& moveVector,
                     f32 checkStepRange);
    void changeCollisionMini();
    void changeCollisionNormal();
    void changeCollisionWallGrab();
    bool findCollidePos(al::DiskInterpolator* interpolator);
    void calcResultVec(sead::Vector3f* result, const sead::Vector3f& velocity);
    void calcBoundingCenter(sead::Vector3f* center) const;
    al::CollisionDirector* getCollisionDirector() const override;

    f32 getHalfHeight() const { return mHalfHeight; }

private:
    al::CollisionDirector* mCollisionDirector;
    const sead::Matrix34f* mMtxPtr;
    const sead::Vector3f* mPositionPtr;
    const sead::Vector3f* mGravityPtr;
    f32 mRadius;
    sead::Vector3f mOffset;
    f32 mHalfHeight;
    sead::Vector3f mPosition;
    f32 mRadiusCurrent;
    f32 mHalfHeightCurrent;
    sead::Vector3f mOffsetWorld;
    sead::Matrix34f mMtx;
    al::HitInfo* mHitInfo;
    f32 mHitDistance;
    u32 _9c;
    al::HitInfo* mHitInfo2;
    f32 mHitDistance2;
    u32 _ac;
    al::HitInfo* mHitInfo3;
    f32 mHitDistance3;
    u32 mHitFrames;
    sead::Matrix34f mCollisionMtx;
    CollisionShapeKeeper* mCollisionNormal;
    CollisionShapeKeeper* mCollisionMini;
    CollisionShapeKeeper* mCollisionCurrent;
    CollisionMultiShape* mCollisionMultiShape;
    sead::PtrArray<al::HitInfo> mHitInfoArray;
    al::HitInfo* mHitInfoBuffer;
    s32 mHitInfoCount;
};
