#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
class Scene;
}  // namespace al

class GameDataHolder;
class SceneAudioSystemPauseController;
class StageScene;
class StageSceneLayout;
class StageSceneStateCollectionList;
class StageSceneStateGetLifeMaxUpItem;
class StageSceneStateMiss;
class StageSceneStateSnapShot;
class StageSceneStateWarp;

class StageSceneStateRaceManRace : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateRaceManRace*
    tryCreate(al::Scene*, GameDataHolder*, StageSceneLayout*, StageSceneStateMiss*,
              StageSceneStateCollectionList*, StageSceneStateGetLifeMaxUpItem*,
              StageSceneStateWarp*, const al::ActorInitInfo&,
              SceneAudioSystemPauseController*, StageSceneStateSnapShot*);
};
