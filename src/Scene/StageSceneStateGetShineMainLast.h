#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class CameraTicket;
class LiveActor;
class Scene;
}  // namespace al

class StageSceneStateGetShineMainLast : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateGetShineMainLast(const char*, al::Scene*, al::LiveActor*, al::CameraTicket*);

private:
    unsigned char filler[0x10];
};

static_assert(sizeof(StageSceneStateGetShineMainLast) == 0x30);
