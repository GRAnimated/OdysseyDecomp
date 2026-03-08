#pragma once

#include "Library/Scene/ISceneObj.h"

class CollectBgmPlayer : public al::ISceneObj {
public:
    const char* getSceneObjName() const override;
    void initAfterPlacementSceneObj(const al::ActorInitInfo&) override {}
    void initSceneObj() override {}

    void* _8 = nullptr;
    void* _10 = nullptr;
    void* _18 = nullptr;
};
