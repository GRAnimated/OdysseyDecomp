#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
}

class StageSceneStateCollectionList;
class StageSceneStatePauseMenu;
class StageSceneStateSnapShot;

class StageSceneStateRadicon : public al::NerveStateBase {
public:
    StageSceneStateRadicon(const char*, al::Scene*, StageSceneStateCollectionList*,
                           StageSceneStatePauseMenu*, StageSceneStateSnapShot*);

    void appear() override;
    void kill() override;

private:
    al::Scene* mScene = nullptr;
    StageSceneStateCollectionList* mStateCollectionList = nullptr;
    StageSceneStatePauseMenu* mStatePauseMenu = nullptr;
    StageSceneStateSnapShot* mStateSnapShot = nullptr;
    bool _38 = false;

    friend class StageScene;
};
