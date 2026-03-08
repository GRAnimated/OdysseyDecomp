#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

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

    const al::PlacementInfo* placementInfo;
    al::CameraTicket* cameraTicket;
    sead::FixedSafeString<128> startId;
    PlayerStartObj* playerStartObj;
    sead::Vector3f trans;
    sead::Quatf quat;
};

static_assert(offsetof(PlayerStartInfo, trans) == 0xB0);
static_assert(offsetof(PlayerStartInfo, quat) == 0xBC);
static_assert(sizeof(PlayerStartInfo) == 0xD0);
