#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class CameraTicket;
class PlacementInfo;
}  // namespace al

class PlayerStartObj;

struct PlayerStartInfo {
    PlayerStartInfo(const al::PlacementInfo*, al::CameraTicket*, const char*, PlayerStartObj*,
                    const sead::Vector3f*, const sead::Quatf*);

    bool isEqualStartId(const char*) const;
    bool isEqualPlacementIdString(const char*) const;

    unsigned char _0[0xA8];
    void* _a8;
    sead::Vector3f trans;
    sead::Quatf quat;
};

static_assert(offsetof(PlayerStartInfo, trans) == 0xB0);
static_assert(offsetof(PlayerStartInfo, quat) == 0xBC);
