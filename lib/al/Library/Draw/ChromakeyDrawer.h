#pragma once

#include <common/aglDrawContext.h>
#include <common/aglRenderBuffer.h>
#include <gfx/seadCamera.h>
#include <gfx/seadViewport.h>

#include "Library/Draw/IUsePartsGraphics.h"

namespace agl::pfx {
class FilterAA;
}
namespace al {
class GraphicsSystemInfo;
class ExecuteDirector;
class EffectSystem;
class Projection;
class SimpleModelEnv;

class ChromakeyDrawer : public al::IUsePartsGraphics {
public:
    ChromakeyDrawer(al::GraphicsSystemInfo*, const al::ExecuteDirector*, const al::EffectSystem*);

    virtual void finalize();
    void drawChromakey(agl::DrawContext*, const al::Projection&, const sead::Camera&,
                       const agl::RenderBuffer*, const sead::Viewport&, const char*, const char*,
                       const char*);
    virtual void update(const al::GraphicsUpdateInfo&);
    virtual void calcGpu(const al::GraphicsCalcGpuInfo&);
    void getName() const;

    void setPhysicalArea(sead::Vector2f area) { mPhysicalArea = area; }

private:
    al::GraphicsSystemInfo* mGraphicsSystemInfo;
    agl::pfx::FilterAA* mFilterAA;
    const al::ExecuteDirector* mExecuteDirector;
    const al::EffectSystem* mEffectSystem;
    sead::Vector2f mPhysicalArea;
    bool field_30;
    al::SimpleModelEnv* mSimpleModelEnv;
};
}  // namespace al