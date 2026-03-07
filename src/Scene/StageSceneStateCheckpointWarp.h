#pragma once

#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class CameraTicket;
class LiveActor;
}  // namespace al

class GameDataHolder;
class StageScene;

class StageSceneStateCheckpointWarp : public al::HostStateBase<StageScene> {
public:
    StageSceneStateCheckpointWarp(const char*, StageScene*, al::LiveActor*, GameDataHolder*,
                                  al::CameraTicket*, sead::Vector3f*, sead::Vector3f*);

private:
    u8 _padding[0x38];
};

static_assert(sizeof(StageSceneStateCheckpointWarp) == 0x58);
