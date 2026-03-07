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
class ScenarioStartCameraHolder;
class StageSceneLayout;
class StageSceneStateScenarioCamera;

class StageSceneStateGetShineMain : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateGetShineMain(const char*, al::Scene*, StageSceneLayout*,
                                const al::SceneInitInfo*, const al::ActorInitInfo&,
                                const al::LayoutInitInfo&, al::LiveActor*,
                                ScenarioStartCameraHolder*, GameDataHolder*);
    void setScenarioCameraState(StageSceneStateScenarioCamera*);

    al::LiveActor* getDemoShineActor() const;
    al::CameraTicket* getCameraTicket() const;
};
