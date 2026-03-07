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
};
