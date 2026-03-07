#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
}

class StageSceneStateSkipDemo;

class StageSceneStateCarryMeat : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateCarryMeat(const char*, al::Scene*);
    void setState(StageSceneStateSkipDemo*);

private:
    unsigned char _padding[0x28 - 0x20];
};

static_assert(sizeof(StageSceneStateCarryMeat) == 0x28);
