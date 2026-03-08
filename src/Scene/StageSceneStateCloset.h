#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
class LayoutInitInfo;
class Scene;
}  // namespace al

class StageSceneStateCloset : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateCloset* tryCreate(al::Scene*, const al::ActorInitInfo&,
                                            const al::LayoutInitInfo&);
};
