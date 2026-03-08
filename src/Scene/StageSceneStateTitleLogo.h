#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
class LayoutInitInfo;
class Scene;
class WipeHolder;
}  // namespace al

class GameDataHolder;

class StageSceneStateTitleLogo : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateTitleLogo* tryCreate(al::Scene*, const al::ActorInitInfo&,
                                               const al::LayoutInitInfo&, al::WipeHolder*,
                                               GameDataHolder*);
};
