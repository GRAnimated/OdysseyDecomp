#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LayoutInitInfo;
class Scene;
struct ActorInitInfo;
struct SceneInitInfo;
}  // namespace al

class StageSceneStateGetShineGrand : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateGetShineGrand(const char*, al::Scene*, const al::SceneInitInfo*,
                                 const al::ActorInitInfo&, const al::LayoutInitInfo&);
};
