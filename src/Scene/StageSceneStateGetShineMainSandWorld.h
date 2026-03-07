#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
class Scene;
}  // namespace al

class Pyramid;
class StageScene;
class StageSceneStateGetShine;
class StageSceneStateGetShineMainLast;
class StageSceneStateRecoverLife;
class StageSceneStateScenarioCamera;

class StageSceneStateGetShineMainSandWorld : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateGetShineMainSandWorld(const char*, StageScene*, const al::ActorInitInfo&);
    void setStateGetShine(StageSceneStateGetShineMain*);
    void setStateScenarioCamera(StageSceneStateScenarioCamera*);
    void setStateRecoverLife(StageSceneStateRecoverLife*);
    void setStateGetShineLast(StageSceneStateGetShineMainLast*);
    void setPyramid(Pyramid*);

private:
    unsigned char _padding[0x50 - 0x20];
};

static_assert(sizeof(StageSceneStateGetShineMainSandWorld) == 0x50);
