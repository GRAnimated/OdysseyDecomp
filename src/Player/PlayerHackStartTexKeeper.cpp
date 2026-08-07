#include "Player/PlayerHackStartTexKeeper.h"

#include <common/aglRenderBuffer.h>
#include <common/aglTextureSampler.h>
#include <gfx/seadColor.h>
#include <gfx/seadViewport.h>

#include "Library/Draw/RenderBufferAttacher.h"

// NON_MATCHING: target orders attacher, buffer, and viewport stack objects differently; next
// source-level hypothesis is the original declaration and lifetime shape.
void PlayerHackStartTexKeeper::clearHackStartTextureOnlyFirstTime(agl::DrawContext* context) {
    if (mIsTextureCleared)
        return;

    agl::RenderBuffer renderBuffer;
    al::RenderBufferAttacher attacher(context, &renderBuffer, mCaptureTexture, nullptr, nullptr,
                                      nullptr, nullptr);
    sead::Viewport viewport(renderBuffer);
    renderBuffer.fastClear(context, 0, 1, sead::Color4f::cBlack, 1.0f, 0, viewport, true);
    mIsTextureCleared = true;
}

void PlayerHackStartTexKeeper::activateHackStartTexture(
    agl::DrawContext* context, const agl::SamplerLocation& location) const {
    agl::TextureSampler sampler;
    sampler.applyTextureData(*mHackStartTexture);
    sampler.activate(context, location, -1, false);
}

const char* PlayerHackStartTexKeeper::getSceneObjName() const {
    return "キャプチャ開始テクスチャ保持";
}
