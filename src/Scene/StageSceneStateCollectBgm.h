#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
class LayoutInitInfo;
class Scene;
}  // namespace al

class CollectBgmPlayer;

class StageSceneStateCollectBgm : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateCollectBgm* tryCreate(al::Scene*, const al::ActorInitInfo&,
                                                const al::LayoutInitInfo&, CollectBgmPlayer*);
};
