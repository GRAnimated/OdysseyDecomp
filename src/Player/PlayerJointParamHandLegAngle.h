#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

struct PlayerJointParamHandLegAngle {
    sead::Vector3f _0;
    f32 _c;
    f32 _10;
    f32 _14;
    f32 _18;
};

static_assert(sizeof(PlayerJointParamHandLegAngle) == 0x1C);
