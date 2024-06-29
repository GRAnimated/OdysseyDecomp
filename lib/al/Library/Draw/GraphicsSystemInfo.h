#pragma once

#include <gfx/seadFrameBuffer.h>
namespace agl {
class DrawContext;
}

namespace al {
class AreaObjDirector;
class ExecuteDirector;
class EffectSystem;
class PlayerHolder;
class SceneCameraInfo;
class ShaderHolder;

class GraphicsInitArg {
public:
    GraphicsInitArg(agl::DrawContext*, sead::FrameBuffer*);
    bool isUsingCubeMapAtmosScatter() const;
    u8 getAtmosScatterViewNum() const;

    s32 dword_0;
    bool field_4;
    u8 mAtmosScatterViewNum;
    s32 mDisplayWidth;
    s32 mDisplayHeight;
    s32 dword_10;
    s32 dword_14;
    s32 dword_18;
    s32 dword_1c;
    bool byte_20;
    s32 dword_24;
    bool byte_28;
    s32 dword_2c;
    s32 dword_30;
    s32 dword_34;
    s32 dword_38;
    s32 dword_3c;
    agl::DrawContext* field_40;
};

class GraphicsSystemInfo {
public:
    GraphicsSystemInfo();
    ~GraphicsSystemInfo();

    void init(const GraphicsInitArg&, AreaObjDirector*, ExecuteDirector*, EffectSystem*,
              PlayerHolder*, SceneCameraInfo*, ShaderHolder*);

    agl::DrawContext* getDrawContext() const;
    void endInit();
    void initAfterPlacement();
    void clearGraphicsRequest();
    void updateGraphics();
    void preDrawGraphics(SceneCameraInfo*);

    // incomplete
private:
    void* filler[312];
};

static_assert(sizeof(GraphicsSystemInfo) == 0x9c0);

}  // namespace al
