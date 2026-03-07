#pragma once

#include "Library/Scene/ISceneObj.h"

class EventDemoCtrl : public al::ISceneObj {
public:
    const char* getSceneObjName() const override;
    void initAfterPlacementSceneObj(const al::ActorInitInfo&) override {}
    void initSceneObj() override {}
};
