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

    void setScenarioStartCameraHolder(ScenarioStartCameraHolder* holder) {
        mScenarioStartCameraHolder = holder;
    }

    void setScenarioStartLayout(al::SimpleLayoutAppearWaitEnd* layout) {
        mScenarioStartLayout = layout;
    }

    bool tryStart();

private:
    al::SimpleLayoutAppearWaitEnd* mScenarioStartLayout = nullptr;
    ScenarioStartCameraHolder* mScenarioStartCameraHolder = nullptr;
    unsigned char _padding2[0x50 - 0x30];
};

static_assert(sizeof(StageSceneStateScenarioCamera) == 0x50);
