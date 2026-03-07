#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
}

class ControllerGuideSnapShotCtrl;
class InputSeparator;
class NpcEventDirector;
class SceneAudioSystemPauseController;

class StageSceneStateSnapShot : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateSnapShot(const char*, al::Scene*, ControllerGuideSnapShotCtrl*,
                            SceneAudioSystemPauseController*, InputSeparator*,
                            NpcEventDirector*);
    void init();
};
