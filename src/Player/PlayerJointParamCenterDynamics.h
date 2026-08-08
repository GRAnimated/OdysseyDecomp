#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

struct PlayerJointParamCenterDynamics {
    bool isEnable;
    bool isReset;
    bool isUseTilt;
    bool isUseBalance;
    f32 tiltRate;
    sead::Vector3f tiltDirection;
    sead::Vector3f balanceDirection;
    sead::Vector3f balanceUp;
};
