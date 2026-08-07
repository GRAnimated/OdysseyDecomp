#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
}

class HackCapAboveGroundChecker {
public:
    explicit HackCapAboveGroundChecker(const al::LiveActor*);

    void update(const sead::Vector3f& gravityDirection);

private:
    const al::LiveActor* mActor;
    bool mIsAboveGround;
    u8 mPadding09[3];
    sead::Vector3f mGroundPosition;
    sead::Vector3f mGroundNormal;
    f32 mGroundDistance;
    f32 mGravityDistance;
    u32 mPadding2C;
};

static_assert(sizeof(HackCapAboveGroundChecker) == 0x30);
