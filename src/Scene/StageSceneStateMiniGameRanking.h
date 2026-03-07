#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
}

class StageScene;

class StageSceneStateMiniGameRanking : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateMiniGameRanking* tryCreate(StageScene*);
};
