#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class DemoSyncedEventKeeper;
class Scene;
class WindowConfirm;
class WipeHolder;
}  // namespace al

class PlayGuideSkip;
class SceneAudioSystemPauseController;
class StageSceneStateWorldMap;

class StageSceneStateSkipDemo : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateSkipDemo(const char*, al::Scene*, al::WindowConfirm*, al::WipeHolder*,
                            PlayGuideSkip*, SceneAudioSystemPauseController*,
                            al::DemoSyncedEventKeeper*);
    void setWorldMapState(StageSceneStateWorldMap*);
};
