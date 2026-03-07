#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
}

class StageScene;

class StageSceneStateRaceManRace : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateRaceManRace* tryCreate(StageScene*);
};
