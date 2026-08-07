#pragma once

#include <basis/seadTypes.h>

#include "Library/Scene/ISceneObj.h"

namespace agl {
class DrawContext;
class SamplerLocation;
class TextureData;

namespace g3d {
class ResFile;
}
}  // namespace agl

class PlayerHackStartTexKeeper : public al::ISceneObj {
public:
    PlayerHackStartTexKeeper();
    ~PlayerHackStartTexKeeper() override;

    void clearHackStartTextureOnlyFirstTime(agl::DrawContext*);
    void activateHackStartTexture(agl::DrawContext*, const agl::SamplerLocation&) const;
    const char* getSceneObjName() const override;

    void setCaptureTextureCleared(bool value) { mIsCaptureTextureCleared = value; }

private:
    agl::TextureData* mHackStartTexture;
    agl::g3d::ResFile* mResource;
    bool mIsTextureCleared;
    bool mIsCaptureTextureCleared;
    u8 mPadding1a[6];
    agl::TextureData* mCaptureTexture;
    u8 mCaptureImageMemory[24];
};

static_assert(sizeof(PlayerHackStartTexKeeper) == 0x40);
