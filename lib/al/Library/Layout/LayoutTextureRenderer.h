#pragma once

#include "Library/Scene/ISceneObj.h"

namespace al {
class LayoutTextureRenderer : public ISceneObj {
public:
    LayoutTextureRenderer();
    const char* getSceneObjName() const override;
    void initAfterPlacementSceneObj(const ActorInitInfo&) override {}
    void initSceneObj() override {}
};
}  // namespace al
