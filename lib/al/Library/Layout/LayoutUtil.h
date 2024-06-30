#pragma once

#include <common/aglRenderBuffer.h>

namespace al {
class LayoutKit;

void setRenderBuffer(al::LayoutKit*, const agl::RenderBuffer*);
void executeUpdate(al::LayoutKit*);
void executeUpdateList(al::LayoutKit*, const char*, const char*);
void executeUpdateEffect(al::LayoutKit*);
void executeDraw(const al::LayoutKit*, const char*);
void executeDrawEffect(const al::LayoutKit*);
}  // namespace al
