#pragma once

#include <common/aglRenderBuffer.h>
#include <common/aglTextureData.h>
#include <gfx/seadCamera.h>

namespace al {
class GraphicsSystemInfo;
class ExecuteDirector;
class EffectSystem;
class SimpleModelEnv;
class DeferredRendering;
class SceneCameraInfo;
class GraphicsQualityInfo;
class ScreenFader;
class Projection;
class GraphicsRenderInfo;
class ViewInfo;
class RenderVariables;
class DrawSystemInfo;
class ShaderHolder;

class ViewRenderer {
public:
    ViewRenderer(al::GraphicsSystemInfo*, al::ExecuteDirector*, al::EffectSystem*,
                 al::SceneCameraInfo*);

    void clearRequest();
    void calcView(s32, const sead::Camera*, const al::Projection*);
    void preDrawGraphics();
    void drawView(s32, al::DrawSystemInfo*, const al::Projection&, const sead::Camera&,
                  const agl::RenderBuffer*, const sead::Viewport&, bool, bool, bool) const;
    void drawView(const al::ViewInfo&, al::DrawSystemInfo*, const al::Projection&,
                  const sead::Camera&, const agl::RenderBuffer*, const sead::Viewport&, bool, bool,
                  bool) const;
    void drawSystem(al::GraphicsRenderInfo const&) const;
    void drawMirror(agl::DrawContext*, s32, al::RenderVariables*) const;
    void drawHdr(al::GraphicsRenderInfo const&, al::RenderVariables const&, bool, bool) const;
    void captureIndirectTexture(agl::DrawContext*, const agl::TextureData*,
                                const agl::TextureData*) const;
    void startForwardPlayerScreenFader(s32, s32, f32);
    void endForwardPlayerScreenFader(s32);

private:
    al::GraphicsSystemInfo* mGraphicsSystemInfo;
    al::ExecuteDirector* mExecuteDirector;
    al::EffectSystem* mEffectSystem;
    al::SimpleModelEnv* mSimpleModelEnv;
    al::DeferredRendering* mDeferredRendering;
    al::SceneCameraInfo* mSceneCameraInfo;
    al::GraphicsQualityInfo* mGraphicsQualityInfo;
    al::ScreenFader* mScreenFader;
    void* field_40;
    void* field_48;
    bool field_50;
    bool mIsWorldMap;
    const agl::TextureData* mEffectTextureColor;
    void* field_60;
};
}  // namespace al

namespace alViewRendererFunction {
void createLinearDepthFromDepthBuffer(agl::DrawContext*, const al::ShaderHolder*,
                                      const agl::TextureData*, const agl::TextureData*);
}