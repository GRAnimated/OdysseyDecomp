#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LayoutInitInfo;
class LiveActor;
class Scene;
struct ActorInitInfo;
struct SceneInitInfo;
}  // namespace al

class GameDataHolder;
class ProjectItemDirector;
class ScenarioStartCameraHolder;
class StageSceneLayout;
class StageSceneStateRecoverLife;
class StageSceneStateScenarioCamera;
class StageSceneStateSkipDemo;

class StageSceneStateGetShine : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateGetShine(const char*, al::Scene*, StageSceneLayout*, const al::SceneInitInfo*,
                            const al::ActorInitInfo&, const al::LayoutInitInfo&, al::LiveActor*,
                            al::LiveActor*, void*, GameDataHolder*, ProjectItemDirector*);
    void setLifeRecoverState(StageSceneStateRecoverLife*);
    void setShopShine10();

private:
    u8 _padding[0x108];
};

static_assert(sizeof(StageSceneStateGetShine) == 0x128);
