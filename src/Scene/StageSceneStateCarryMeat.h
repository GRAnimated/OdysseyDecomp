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
};
