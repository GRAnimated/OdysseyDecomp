#pragma once

#include <basis/seadTypes.h>

namespace agl {
class DrawContext;
class RenderBuffer;
class RenderTargetDepth;
class TextureData;
}  // namespace agl

namespace al {
class alignas(8) RenderBufferAttacher {
public:
    RenderBufferAttacher(agl::DrawContext*, agl::RenderBuffer*, const agl::TextureData*,
                         const agl::TextureData*, const agl::TextureData*,
                         const agl::TextureData*, const agl::RenderTargetDepth*);
    ~RenderBufferAttacher();

private:
    u8 _0[0x610];
};

}  // namespace al
