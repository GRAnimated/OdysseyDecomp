#pragma once

#include <common/aglDrawContext.h>
#include <common/aglRenderBuffer.h>
#include <framework/nx/seadGameFrameworkNx.h>
#include "Library/HostIO/HioNode.h"

namespace sead {
class Event;
}

namespace al {
class GpuPerf;

class GameFrameworkNx : public sead::GameFrameworkNx, al::HioNode {
    SEAD_RTTI_OVERRIDE(GameFrameworkNx, sead::GameFramework);

public:
    GameFrameworkNx(sead::GameFrameworkNx::CreateArg const&);
    virtual void createControllerMgr(sead::TaskBase*) override;
    void initAgl(sead::Heap*, s32, s32, s32, s32, s32, s32);
    virtual void createInfLoopChecker(sead::TaskBase*, sead::TickSpan const&, s32) override;
    void clearFrameBuffer();
    void procFrame_();
    void procDraw_();
    void present_();
    virtual void createHostIOMgr(sead::TaskBase*, sead::HostIOMgr::Parameter*,
                                 sead::Heap*) override;

    void enableRendering() { mIsNotRendering = false; }

    void disableRendering() { mIsNotRendering = true; }

private:
    agl::DrawContext* mDrawContext;
    agl::RenderBuffer* mRenderBuffer;
    agl::RenderTargetColor* mRenderTargetColor;
    agl::RenderBuffer* mRenderBuffer2;
    agl::RenderTargetColor* mRenderTargetColor2;
    agl::DisplayList* mDisplayList[2];
    al::GpuPerf* mGpuPerf;
    s32 mDisplaySelection;
    void* mDisplayControlMem[2];
    bool mIsNotRendering;
    bool mIsBufferSelection;
    sead::Event* mEvent;
};
}  // namespace al
