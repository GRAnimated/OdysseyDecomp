#pragma once

namespace al {
class GraphicsCopyInfo;
class GraphicsComputeInfo;
class GraphicsUpdateInfo;
class GraphicsRenderInfo;
class GraphicsCalcGpuInfo;
class RenderVariables;

class IUsePartsGraphics {
public:
    virtual void finalize();
    virtual void endInit();
    virtual void doCommandBufferCopy(const al::GraphicsCopyInfo*) const;
    virtual void doComputeShader(const al::GraphicsComputeInfo*) const;
    virtual void drawSystem(const al::GraphicsRenderInfo*) const;
    virtual void update(const al::GraphicsUpdateInfo&);
    virtual void calcGpu(const al::GraphicsCalcGpuInfo&);
    virtual void drawGBufferAfterSky(const al::GraphicsRenderInfo&) const;
    virtual void drawForward(const al::GraphicsRenderInfo&, const al::RenderVariables&) const;
    virtual void drawDeferred(const al::GraphicsRenderInfo&) const;
    virtual void drawLdr(const al::GraphicsRenderInfo&) const;
    virtual void drawIndirect(const al::GraphicsRenderInfo&, const al::RenderVariables&) const;
    virtual void drawCubemap(const al::GraphicsRenderInfo&) const;
    virtual void getName() const;
};
}  // namespace al
