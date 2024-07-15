#pragma once

#include <basis/seadTypes.h>
#include <common/aglDrawContext.h>
#include <container/seadPtrArray.h>

#include "Library/HostIO/HioNode.h"

namespace al {
class ScreenCapture;

class ScreenCaptureExecutor : public IUseHioNode {
public:
    ScreenCaptureExecutor(s32);
    ~ScreenCaptureExecutor();

    void createScreenCapture(s32, s32, s32);
    void draw(agl::DrawContext*, const agl::RenderBuffer*, s32) const;
    void tryCapture(agl::DrawContext*, const agl::RenderBuffer*, s32);
    void tryCaptureAndDraw(agl::DrawContext*, const agl::RenderBuffer*, s32);

    void requestCapture(bool, s32);
    void onDraw(s32 screenCaptureIndex);
    void offDraw(s32 screenCaptureIndex);
    void offDraw();

    bool isDraw(s32) const;

private:
    sead::PtrArray<ScreenCapture> mArray;
    bool mIsCaptured;
};
}  // namespace al