#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
}

class StageScene;

class StageSceneStateCollectBgm : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateCollectBgm* tryCreate(StageScene*);
};
