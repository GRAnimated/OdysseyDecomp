#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {

class JointLookAtParam;

struct JointLookAtInfo {
    JointLookAtInfo()
        : param(nullptr), targetQuat(sead::Quatf::unit), currentQuat(sead::Quatf::unit),
          _28(0.0f), _30(0), frontDir(sead::Vector3f::ez), upDir(sead::Vector3f::ey),
          rightDir(sead::Vector3f::ex), _5c(sead::Vector3f::ez) {}

    JointLookAtParam* param;
    sead::Quatf targetQuat;
    sead::Quatf currentQuat;
    f32 _28;
    u64 _30;
    sead::Vector3f frontDir;
    sead::Vector3f upDir;
    sead::Vector3f rightDir;
    sead::Vector3f _5c;
};

static_assert(sizeof(JointLookAtInfo) == 0x68);

}  // namespace al
