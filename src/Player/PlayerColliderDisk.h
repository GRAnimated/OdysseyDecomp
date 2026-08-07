#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

class PlayerColliderDisk {
public:
    void onInvalidate();
    sead::Vector3f collide(const sead::Vector3f& velocity);
    void changeCollisionMini();
    void changeCollisionNormal();
    void changeCollisionWallGrab();
    void calcBoundingCenter(sead::Vector3f* center) const;

    f32 getHalfHeight() const { return mHalfHeight; }

private:
    u8 _0[0x38];
    f32 mHalfHeight;
};
