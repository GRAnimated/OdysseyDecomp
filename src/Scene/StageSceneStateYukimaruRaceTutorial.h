#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
}

class StageScene;

class StageSceneStateYukimaruRaceTutorial : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateYukimaruRaceTutorial* tryCreate(StageScene*);
};
