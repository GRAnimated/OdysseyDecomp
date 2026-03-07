#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
class Scene;
}  // namespace al

class ScenarioStartCameraHolder;
class StageSceneStateSkipDemo;

namespace al {
class SimpleLayoutAppearWaitEnd;
}

class StageSceneStateScenarioCamera : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateScenarioCamera(const char*, al::Scene*, const char*, s32, al::LiveActor*);
    void init();
    void setStateSkipDemo(StageSceneStateSkipDemo*);
    void setScenarioStartCameraHolder(ScenarioStartCameraHolder*);
    void setScenarioStartLayout(al::SimpleLayoutAppearWaitEnd*);
    bool tryStart();
};
