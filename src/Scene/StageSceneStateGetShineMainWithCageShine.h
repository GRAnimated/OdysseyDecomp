#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
class Scene;
}  // namespace al

class StageScene;
class StageSceneStateGetShineMain;
class StageSceneStateGetShineMainLast;
class StageSceneStateRecoverLife;
class StageSceneStateScenarioCamera;

class StageSceneStateGetShineMainWithCageShine : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateGetShineMainWithCageShine* tryCreate(StageScene*,
                                                               const al::ActorInitInfo&);
    void setState(StageSceneStateGetShineMain*, StageSceneStateScenarioCamera*,
                  StageSceneStateRecoverLife*, StageSceneStateGetShineMainLast*);
};
