#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
}

class StageScene;

class StageSceneStateCloset : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateCloset* tryCreate(StageScene*);
};
