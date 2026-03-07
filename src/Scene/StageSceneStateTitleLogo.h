#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
}

class StageScene;

class StageSceneStateTitleLogo : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateTitleLogo* tryCreate(StageScene*);
};
