#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LayoutInitInfo;
class Scene;
class SubCameraRenderer;
}  // namespace al

class GameDataHolder;
class StageSceneLayout;

class StageSceneStateMiss : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateMiss(const char*, al::Scene*, const al::LayoutInitInfo&, GameDataHolder*,
                        StageSceneLayout*, al::SubCameraRenderer*);

private:
    unsigned char _padding[0x48 - 0x20];
};

static_assert(sizeof(StageSceneStateMiss) == 0x48);
