#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LayoutInitInfo;
class Scene;
}  // namespace al

class MapLayout;
class SceneAudioSystemPauseController;
class StageSceneStateCollectBgm;

class StageSceneStateCollectionList : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateCollectionList(const char*, al::Scene*, const al::LayoutInitInfo&, MapLayout*,
                                  StageSceneStateCollectBgm*, SceneAudioSystemPauseController*);
};
