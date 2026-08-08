#pragma once

#include <basis/seadTypes.h>
#include <common/aglGPUMemAddr.h>

#include "Library/Scene/ISceneObj.h"

namespace nn::g3d {
class ResFile;
}

namespace agl {
class DrawContext;
class SamplerLocation;
class TextureData;

}  // namespace agl

class PlayerHackStartTexKeeper : public al::ISceneObj {
public:
    PlayerHackStartTexKeeper();
    ~PlayerHackStartTexKeeper() override;

    void clearHackStartTextureOnlyFirstTime(agl::DrawContext* context);
    void activateHackStartTexture(agl::DrawContext* context, const agl::SamplerLocation& location) const;
    const char* getSceneObjName() const override;

    void setCaptureTextureCleared(bool value) { mIsCaptureTextureCleared = value; }

private:
    agl::TextureData* mHackStartTexture;
    nn::g3d::ResFile* mResource;
    bool mIsTextureCleared;
    bool mIsCaptureTextureCleared;
    u8 mPadding1a[6];
    agl::TextureData* mCaptureTexture;
    agl::GPUMemAddrBase mCaptureImageMemory;
};

static_assert(sizeof(PlayerHackStartTexKeeper) == 0x40);
