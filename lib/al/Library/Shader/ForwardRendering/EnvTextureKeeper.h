#pragma once

#include <basis/seadTypes.h>

namespace nn::g3d {
class ResMaterial;
}

namespace al {
struct EnvTexInfo;

struct EnvTexId {
    EnvTexId();
    void invalidateAll();
    void initForCache();
    void isEnableTexId(s32);
    void change(const EnvTexId&);
    void checkAndSetReflectCubeMapChange(const EnvTexInfo&);
    void checkAndSetMirrorTexChange(const EnvTexInfo&);
    void checkAndSetLightMapChange(const EnvTexInfo&);
    void checkAndSetSphereLightMapChange(const EnvTexInfo&);
    void checkAndSetProcTex2DChange(const EnvTexInfo&);
    void checkAndSetProcTex3DChange(const EnvTexInfo&);
    void checkAndSetRippleTexChange(const EnvTexInfo&);

    s32 _0;
    s32 _4;
    s32 _8;
    s32 mMirrorId;
    s32 _10;
    s32 mProc3DId;
    s32 _18;
};

static_assert(sizeof(EnvTexId) == 0x1C);

struct EnvTexInfo {
    s32 getCubeMapId() const;
    s32 getMirrorTexId() const;
    s32 getLightMapId() const;
    s32 getSphereLightMapId() const;
    s32 getProcTex2DId() const;
    s32 getProcTex3DId() const;
    s32 getRippleTexId() const;
    EnvTexInfo();
    EnvTexInfo(const nn::g3d::ResMaterial&);

    s32 _0;
    s32 _4;
    s32 _8;
    s32 _c;
    s32 _10;
    s32 _14;
    s32 _18;
    EnvTexId mEnvTexId;
    s32 _38;
    bool _3c;
};

static_assert(sizeof(EnvTexInfo) == 0x40);

}  // namespace al
