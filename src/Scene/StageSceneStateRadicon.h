#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
}

class StageSceneStateCollectionList;
class StageSceneStatePauseMenu;
class StageSceneStateSnapShot;

class StageSceneStateRadicon : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateRadicon(const char*, al::Scene*, StageSceneStateCollectionList*,
                           StageSceneStatePauseMenu*, StageSceneStateSnapShot*);
};
