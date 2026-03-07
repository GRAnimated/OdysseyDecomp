#pragma once

#include <basis/seadTypes.h>

namespace al {
class Scene;
struct ActorInitInfo;
}  // namespace al

class ScenarioStartCameraHolder {
public:
    ScenarioStartCameraHolder();
    void init(const al::Scene*, const al::ActorInitInfo&);

private:
    u8 _padding[0x58];
};

static_assert(sizeof(ScenarioStartCameraHolder) == 0x58);
