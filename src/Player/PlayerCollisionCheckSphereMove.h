#pragma once

#include <container/seadPtrArray.h>
#include <math/seadVector.h>

#include "Library/Collision/IUseCollision.h"

namespace al {
class CollisionDirector;
class CollisionParts;
class CollisionPartsFilterBase;
class Triangle;
}  // namespace al

namespace alCollisionUtil {
struct SphereMoveHitInfo;
}

class PlayerCollisionCheckSphereMove : public al::IUseCollision {
public:
    PlayerCollisionCheckSphereMove(al::CollisionDirector*, s32);

    al::CollisionDirector* getCollisionDirector() const override;

    bool checkSphereMove(const sead::Vector3f&, const sead::Vector3f&, f32);
    void gatherCollisionParts(al::CollisionParts*);
    u32 getNum() const;
    f32 getTValue(u32) const;
    const sead::Vector3f& getPos(u32) const;
    const sead::Vector3f& getNormal(u32) const;
    const al::CollisionParts* getCollisionParts(u32) const;
    const char* getMapCodeName(u32) const;
    const char* getWallCodeName(u32) const;
    const char* getMaterialCodeName(u32) const;

    void setCollisionPartsFilter(const al::CollisionPartsFilterBase* filter) {
        mCollisionPartsFilter = filter;
    }

private:
    al::CollisionDirector* mCollisionDirector;
    u32 mMaxNum;
    u32 mNum;
    f32* mTValues;
    sead::Vector3f* mPositions;
    al::Triangle* mTriangles;
    alCollisionUtil::SphereMoveHitInfo* mHitInfos;
    void* _38;
    u32* mIndices;
    sead::PtrArray<al::CollisionParts> mCollisionParts;
    const al::CollisionPartsFilterBase* mCollisionPartsFilter;
};

static_assert(sizeof(PlayerCollisionCheckSphereMove) == 0x60);
