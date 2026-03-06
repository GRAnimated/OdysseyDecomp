#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {

class JointLookAtParam {
public:
    JointLookAtParam(s32 jointIndex, f32 lookRate, const sead::Vector2f& limitYawDegree,
                     const sead::Vector2f& limitPitchDegree, const sead::Vector3f& localFrontDir,
                     const sead::Vector3f& localUpDir);

    s32 mJointIndex;
    f32 mLookRate;
    sead::Vector2f mLimitYawDegree;
    sead::Vector2f mLimitPitchDegree;
    sead::Vector3f mLocalFrontDir;
    sead::Vector3f mLocalUpDir;
    u8 _30[0xC];
};

static_assert(sizeof(JointLookAtParam) == 0x3C);

}  // namespace al
