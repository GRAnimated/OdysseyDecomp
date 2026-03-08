#pragma once

#include <basis/seadTypes.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class GamePadSystem;
}

namespace sead {
template <typename>
class Matrix34;
using Matrix34f = Matrix34<f32>;
}  // namespace sead

struct PlayerInitInfo {
    al::GamePadSystem* gamePadSystem;
    const sead::Matrix34f* viewMtxPtr;
    s32 controllerPort;
    s32 _14;
    const char* costumeName;
    const char* capTypeName;
    sead::Vector3f trans;
    sead::Quatf quat;
    u8 _44;
    bool _45;
};

static_assert(sizeof(PlayerInitInfo) == 0x48);
