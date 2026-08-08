#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

struct PlayerJointParamHandLegAngle {
    sead::Vector3f handAngle;
    sead::Vector3f legAngle;
    f32 blendRate;
};

