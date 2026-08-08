#pragma once

#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <math/seadMatrix.h>
#include <prim/seadBitFlag.h>

#include "Library/Collision/IUseCollision.h"
#include "Library/HostIO/HioNode.h"

namespace al {
class CollisionPartsFilterBase;
struct HitInfo;
class SpherePoseInterpolator;
}  // namespace al
class CollidedShapeResult;
class CollisionShapeKeeper;
class CollisionMultiShape;

class PlayerCollider : public al::HioNode, public al::IUseCollision {
public:
    PlayerCollider(al::CollisionDirector* collisionDirector, const sead::Matrix34f* mtx,
                   const sead::Vector3f* trans, const sead::Vector3f* gravity,
                   bool isLargeCollisionBuffer);
    void onInvalidate();
    void setCollisionShapeKeeper(CollisionShapeKeeper* keeper);
    void calcBoundingRadius(f32* radius) const;
    void setCollisionShapeScale(f32 scale);
    void onCutCollideAffectDir(const sead::Vector3f& direction);
    void offCutCollideAffectDir();
    void clear();
    void calcCheckPos(sead::Vector3f* checkPos) const;
    void resetPose(const sead::Quatf& quat);
    sead::Vector3f collide(const sead::Vector3f& move);
    bool calcMovePowerByContact(sead::Vector3f* movePower, const sead::Vector3f& contactPos);
    void moveCollide(sead::Vector3f* pos, f32* size, sead::Quatf* quat,
                     const sead::Vector3f& targetPos, f32 targetSize,
                     const sead::Quatf& targetQuat, const sead::Vector3f& moveVec,
                     f32 checkStepRange, bool skipFirstStep);
    bool findCollidePos(al::SpherePoseInterpolator* interpolator);
    void calcResultVec(sead::Vector3f* fixResult, sead::Vector3f* collideResult,
                       const sead::Vector3f& previousFix);
    void calcGroundArrowAverage(bool* hasGroundPos, sead::Vector3f* groundPos,
                                bool* hasGroundNormal, sead::Vector3f* groundNormal,
                                const CollisionShapeKeeper* shapeKeeper);
    void calcResultVecArrow(sead::BitFlag32* flags, sead::Vector3f* staticMin,
                            sead::Vector3f* staticMax, sead::Vector3f* movingMin,
                            sead::Vector3f* movingMax, const CollidedShapeResult* result);
    void calcResultVecSphere(sead::BitFlag32* flags, sead::Vector3f* staticMin,
                             sead::Vector3f* staticMax, sead::Vector3f* movingMin,
                             sead::Vector3f* movingMax, const CollidedShapeResult* result);
    void calcResultVecDisk(sead::BitFlag32* flags, sead::Vector3f* staticMin,
                           sead::Vector3f* staticMax, sead::Vector3f* movingMin,
                           sead::Vector3f* movingMax, const CollidedShapeResult* result);
    void collectHitInfoArray(const al::HitInfo& hitInfo, s32 arrayIndex);
    bool isNeedWallBorderCheck(const al::HitInfo& hitInfo) const;
    void setWallBorderCheckTypeNone();
    void setWallBorderCheckTypeNoFace();
    void setWallBorderCheckTypeAll();
    void setCollisionPartsFilter(const al::CollisionPartsFilterBase* filter);
    void calcBoundingCenter(sead::Vector3f* center) const;
    void validateCorrectMovePartsCheck();

    void setInFastMoveCollisionArea(bool value) { mIsInFastMoveCollisionArea = value; }

    void setValidGroundSupport(bool value) { mIsValidGroundSupport = value; }

    void setDuringRecovery(bool value) { mIsDuringRecovery = value; }

    CollisionShapeKeeper* getCollisionShapeKeeper() const { return mCollisionShapeKeeper; }

    al::CollisionDirector* getCollisionDirector() const override;

    f32 get_70() const { return _70; }

    const sead::Vector3f& getCollidedGroundNormal() const { return mCollidedGroundNormal; }

    void set1b0(f32 value) { _1b0 = value; }

private:
    al::CollisionDirector* mCollisionDirector;
    const sead::Matrix34f* mMtxPtr;
    const sead::Vector3f* mTransPtr;
    const sead::Vector3f* mGravityPtr;
    sead::Vector3f mTrans;
    f32 mSize;
    sead::Matrix34f mMtx;
    al::HitInfo* _68;
    f32 _70;
    al::HitInfo* _78;
    f32 _7c;
    al::HitInfo* _88;
    f32 _8c;
    sead::Vector3f mCollidedFixReaction;
    bool _a0;
    bool _a1;
    sead::Vector3f mCollisionHitNormal;
    sead::Vector3f mCollisionHitPos;
    s32 mTimeInAir;
    sead::Matrix34f mCollidePosMtx;
    CollisionShapeKeeper* mCollisionShapeKeeper;
    f32 mCollisionShapeScale;
    CollisionMultiShape* mCollisionMultiShape;
    s32 _108;
    bool mIsInFastMoveCollisionArea;
    bool mIsValidGroundSupport;
    bool mIsDuringRecovery;
    sead::Vector3f mCutCollideAffectDir;
    s32 mWallBorderCheckType;
    const al::CollisionPartsFilterBase* mCollisionPartsFilter;
    sead::PtrArray<al::HitInfo> _128[3];
    al::HitInfo* _158;
    u32 _160;
    s32 _164;
    sead::PtrArray<al::HitInfo> _168;
    sead::Buffer<f32> mHitDistanceArray;
    sead::Buffer<f32> mHitValueArray;
    sead::Vector3f _198;
    sead::Vector3f mCollidedGroundNormal;
    f32 _1b0;
};

static_assert(sizeof(PlayerCollider) == 0x1b8);
