#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

struct PlayerJointParamGrab {
    f32 dynamicsRate;
    f32 dynamicsFollowRate;
    f32 poseRate;
    sead::Vector3f direction;
    f32 interpolateRate;
    bool isEnable;
    u8 padding_1d[3];
};

