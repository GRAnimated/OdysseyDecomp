#pragma once

#include <container/seadPtrArray.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class CollisionParts;
class CollisionPartsFilterBase;
class IUseCollision;
class KCollisionServer;
struct KCPrismData;
struct KCPrismHeader;
}  // namespace al

class CollisionShapeKeeper;

class CollisionMultiShape {
public:
    CollisionMultiShape(const al::IUseCollision* useCollision, s32 maxParts);

    bool check(CollisionShapeKeeper* shapeKeeper, const sead::Matrix34f* matrix, f32 scale,
               const sead::Vector3f& velocity,
               const al::CollisionPartsFilterBase* collisionPartsFilter);
    void callbackFromParts(al::CollisionParts* collisionParts);
    void callbackFromServer(const al::KCPrismData* prismData, const al::KCPrismHeader* prismHeader);

    void validateCorrectMovePartsCheck() { _78 = false; }

private:
    const al::IUseCollision* mUseCollision;
    sead::Vector3f mCheckPos;
    u8 _14[4];
    CollisionShapeKeeper* mShapeKeeper;
    sead::Vector3f mVelocity;
    sead::Vector3f _2c;  // relative sweep direction used for backface rejection
    s32 _38;             // active collision-shape index
    f32 _3c;             // current collision-parts scale
    sead::Vector3f _40;  // per-step local shape offset
    al::CollisionParts* mCollisionParts;
    al::KCollisionServer* mKCollisionServer;
    const al::KCPrismHeader* mKCPrismHeader;
    sead::PtrArray<const al::KCPrismData> mKCPrismDataArray;
    bool _78;
};

static_assert(sizeof(CollisionMultiShape) == 0x80);
