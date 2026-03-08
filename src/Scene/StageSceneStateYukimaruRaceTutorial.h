#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
class LayoutInitInfo;
class Scene;
}  // namespace al

class GameDataHolder;
class StageSceneStateSnapShot;

class StageSceneStateYukimaruRaceTutorial : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateYukimaruRaceTutorial* tryCreate(al::Scene*, const al::ActorInitInfo&,
                                                           const al::LayoutInitInfo&,
                                                           const GameDataHolder*,
                                                           StageSceneStateSnapShot*);
};
