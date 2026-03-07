#pragma once

#include <basis/seadTypes.h>

#include "Library/Scene/ISceneObj.h"

namespace al {
class LayoutTextureRenderer : public ISceneObj {
public:
    LayoutTextureRenderer();
    const char* getSceneObjName() const override;
    void initAfterPlacementSceneObj(const ActorInitInfo&) override {}
    void initSceneObj() override {}

private:
    u8 _padding[0x10];
};

static_assert(sizeof(LayoutTextureRenderer) == 0x18);
}  // namespace al
