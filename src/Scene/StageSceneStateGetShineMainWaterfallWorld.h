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

class StageSceneStateGetShineMainWaterfallWorld : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateGetShineMainWaterfallWorld* tryCreate(StageScene*,
                                                                const al::ActorInitInfo&);
    void setStateGetShine(StageSceneStateGetShineMain*);
    void setStateScenarioCamera(StageSceneStateScenarioCamera*);
    void setStateRecoverLife(StageSceneStateRecoverLife*);
    void setStateGetShineLast(StageSceneStateGetShineMainLast*);
};
