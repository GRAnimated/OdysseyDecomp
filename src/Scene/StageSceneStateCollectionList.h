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

private:
    u8 _padding[0x80];
};

static_assert(sizeof(StageSceneStateCollectionList) == 0xa0);
