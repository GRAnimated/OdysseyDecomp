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
    PlayerCollisionCheckSphereMove(al::CollisionDirector* collisionDirector, s32 maxNum);

    bool checkSphereMove(const sead::Vector3f& start, const sead::Vector3f& move, f32 radius);

    void gatherCollisionParts(al::CollisionParts* collisionParts);
    u32 getNum() const;
    f32 getTValue(u32 index) const;
    const sead::Vector3f& getPos(u32 index) const;
    const sead::Vector3f& getNormal(u32 index) const;
    const al::CollisionParts* getCollisionParts(u32 index) const;
    const char* getMapCodeName(u32 index) const;
    const char* getWallCodeName(u32 index) const;
    const char* getMaterialCodeName(u32 index) const;
    al::CollisionDirector* getCollisionDirector() const override;

    void setCollisionPartsFilter(const al::CollisionPartsFilterBase* filter) {
        mCollisionPartsFilter = filter;
    }

private:
    al::CollisionDirector* mCollisionDirector;
    u32 mMaxNum;
    u32 mNum = 0;
    f32* mTValues;
    sead::Vector3f* mPositions;
    al::Triangle* mTriangles;
    alCollisionUtil::SphereMoveHitInfo* mHitInfos;
    void* _38 = nullptr;
    u32* mIndices;
    sead::PtrArray<al::CollisionParts> mCollisionParts;
    const al::CollisionPartsFilterBase* mCollisionPartsFilter = nullptr;
};

static_assert(sizeof(PlayerCollisionCheckSphereMove) == 0x60);
