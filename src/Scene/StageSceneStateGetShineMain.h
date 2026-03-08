#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class CameraTicket;
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

    al::LiveActor* getDemoShineActor() const { return mDemoShineActor; }
    al::CameraTicket* getCameraTicket() const { return mCameraTicket; }

private:
    unsigned char _20[0x28];
    al::CameraTicket* mCameraTicket;
    unsigned char _50[0x30];
    al::LiveActor* mDemoShineActor;
    unsigned char _88[0x50];
};

static_assert(sizeof(StageSceneStateGetShineMain) == 0xd8);
