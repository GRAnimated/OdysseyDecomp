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
using Matrix34f = Matrix34<float>;
}  // namespace sead

struct PlayerInitInfo {
    al::GamePadSystem* gamePadSystem;       // 0x00
    const sead::Matrix34f* viewMtxPtr;      // 0x08
    s32 controllerPort;                     // 0x10
    s32 _14;                                // 0x14
    const char* costumeName;                // 0x18
    const char* capTypeName;                // 0x20
    sead::Vector3f trans;                   // 0x28
    sead::Quatf quat;                       // 0x34
    u8 _44;                                 // 0x44
    bool _45;                               // 0x45
};

static_assert(sizeof(PlayerInitInfo) == 0x48);
