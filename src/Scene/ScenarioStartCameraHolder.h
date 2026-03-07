#pragma once

namespace al {
class Scene;
struct ActorInitInfo;
}  // namespace al

class ScenarioStartCameraHolder {
public:
    ScenarioStartCameraHolder();
    void init(const al::Scene*, const al::ActorInitInfo&);
};
