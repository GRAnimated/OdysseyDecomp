#include "Player/PlayerHackStartTexKeeper.h"

#include <common/aglRenderBuffer.h>
#include <common/aglTextureData.h>
#include <common/aglTextureSampler.h>
#include <gfx/seadColor.h>
#include <gfx/seadViewport.h>
#include <driver/aglGraphicsDriverMgr.h>
#include <g3d/aglNW4FToNN.h>
#include <nn/g3d/ResFile.h>
#include <nn/gfx/gfx_Device.h>

#include "Library/Draw/RenderBufferAttacher.h"

PlayerHackStartTexKeeper::~PlayerHackStartTexKeeper() {
    if (mResource)
        mResource->Cleanup(
            static_cast<nn::gfx::Device*>(agl::driver::GraphicsDriverMgr::instance()->getDevice()));

    if (mHackStartTexture) {
        delete mHackStartTexture;
        mHackStartTexture = nullptr;
    }

    if (mCaptureTexture) {
        delete mCaptureTexture;
        mCaptureTexture = nullptr;
    }

    if (mCaptureImageMemory.isValid()) {
        mCaptureImageMemory.deleteGPUMemBlock();
        mCaptureImageMemory.invalidate();
    }
}

// NON_MATCHING: target/current are 0xb4 bytes but order the attacher, buffer, and viewport
// stack objects differently; next source-level hypothesis is the original declaration/lifetime
// shape with the render-buffer attachment object spanning the viewport setup.
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
