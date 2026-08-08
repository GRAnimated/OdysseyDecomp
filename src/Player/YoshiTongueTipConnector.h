#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class CollisionParts;
class LiveActor;
class MtxConnector;
}  // namespace al

class YoshiTongueTipConnector {
public:
    YoshiTongueTipConnector(const al::LiveActor* actor);

    void reset();
    void attachCollision(const al::CollisionParts* collisionParts,
                         const sead::Vector3f& front, const sead::Vector3f& up,
                         const sead::Vector3f& position, const sead::Vector3f& polygonNormal,
                         const sead::Vector3f& gravity);
    bool tryCalcConnect(sead::Vector3f* front, sead::Vector3f* up,
                        sead::Vector3f* position);

    bool isGroundAttached() const { return mIsGroundAttached; }
    const al::CollisionParts* getCollisionParts() const { return mCollisionParts; }

private:
    const al::CollisionParts* mCollisionParts;
    al::MtxConnector* mMtxConnector;
    sead::Matrix34f mBaseMtx;
    bool mIsGroundAttached;
};

static_assert(sizeof(YoshiTongueTipConnector) == 0x48);
