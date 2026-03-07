#pragma once

#include "Library/Scene/ISceneObj.h"

class BgmAnimeSyncDirector : public al::ISceneObj {
public:
    BgmAnimeSyncDirector();
    const char* getSceneObjName() const override;
    void initAfterPlacementSceneObj(const al::ActorInitInfo&) override {}
    void initSceneObj() override {}
};
