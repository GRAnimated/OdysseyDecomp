#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
class Scene;
}  // namespace al

class StageScene;

class StageSceneStateYukimaruRace : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateYukimaruRace* tryCreate(StageScene*, const al::ActorInitInfo&);
};
