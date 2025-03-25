#pragma once

#include <container/seadPtrArray.h>
#include "gfx/seadCamera.h"
#include "utility/aglParameterIO.h"

namespace agl {
namespace pfx {
class FilterAA;
}
class DrawContext;
}  // namespace agl

namespace al {
struct GraphicsInitArg {
    s32 dword_0;
    bool _4;
    u8 mAtmosScatterViewNum;
    bool _6;
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
    agl::DrawContext* _40;
};

class AreaObjDirector;
class ExecuteDirector;
class EffectSystem;
class PlayerHolder;
class SceneCameraInfo;
class ShaderHolder;
class BloomDirector;
class CubeMapDirector;
class DirectionalLightKeeper;
class GraphicsAreaDirector;
class GraphicsPresetDirector;
class DemoGraphicsController;
class RadialBlurDirector;
class PrePassLightKeeper;
class ShaderEnvTextureKeeper;
class ShadowDirector;
class DepthOfFieldDrawer;
class GraphicsQualityController;
class ShaderMirrorDirector;
class GraphicsParamRequesterImpl;
class GraphicsParamRequesterImpl;
class FlareFilterDirector;
class GodRayDirector;
class FogDirector;
class OccludedEffectDirector;
class LightStreakDirector;
class HdrCompose;
class SSIIKeeper;
class OceanWave;
class RandomTextureKeeper;
class WorldAODirector;
class PointSpriteCursorHolder;
class MaterialLightDirector;
class MaterialCategoryKeeper;
class OcclusionCullingJudge;
class VignettingDrawer;
class CameraBlurController;
class ThunderRenderKeeper;
class StarrySky;
class SkyDirector;
class NoiseTextureKeeper;
class CloudRenderKeeper;
class GpuMemAllocator;
class FootPrintTextureKeeper;
class ProgramTextureKeeper;
class ViewRenderer;
class SubCameraRenderer;
class TemporalInterlace;
class PeripheryRendering;
class PostProcessingFilter;
class GBufferArray;
class AtmosScatter;
class AtmosScatterDrawer;
class GraphicsParamFilePath;
class Projection;
class UniformBlock;
class Resource;
class VastGridMeshDirector;
class FullModelMaterialCategory;
class FullScreenTriangle;
class ReducedBufferRenderer;
class ModelOcclusionCullingDirector;
class ModelLodAllCtrl;
class ModelShaderHolder;
class PrepassTriangleCulling;
class ApplicationMessageReceiver;

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

    MaterialCategoryKeeper* getMaterialCategoryKeeper() const { return mMaterialCategoryKeeper; }

private:
    char mViewIndexedUboArrayTree[28];
    GraphicsInitArg mInitArg;
    BloomDirector* mBloomDirector;
    CubeMapDirector* mCubeMapDirector;
    DirectionalLightKeeper* mDirectionalLightKeeper;
    GraphicsAreaDirector* mGraphicsAreaDirector;
    GraphicsPresetDirector* mGraphicsPresetDirector;
    DemoGraphicsController* mDemoGraphicsController;
    RadialBlurDirector* mRadialBlurDirector;
    PrePassLightKeeper* mPrePassLightKeeper;
    ShaderEnvTextureKeeper* mShaderEnvTextureKeeper;
    ShadowDirector* mShadowDirector;
    DepthOfFieldDrawer* mDepthOfFieldDrawer;
    GraphicsQualityController* mGraphicsQualityController;
    ShaderMirrorDirector* mShaderMirrorDirector;
    GraphicsParamRequesterImpl* _d0;
    GraphicsParamRequesterImpl* mColorCorrection;
    FlareFilterDirector* mFlareFilterDirector;
    GodRayDirector* mGodRayDirector;
    FogDirector* mFogDirector;
    OccludedEffectDirector* mOccludedEffectDirector;
    LightStreakDirector* mLightStreakDirector;
    HdrCompose* mHdrCompose;
    SSIIKeeper* mSSIIKeeper;
    void* mPrimitiveOcclusion;
    char mViewVolume[232];
    void* _208;
    OceanWave* mOceanWave;
    RandomTextureKeeper* mRandomTextureKeeper;
    WorldAODirector* mWorldAODirector;
    PointSpriteCursorHolder* mPointSpriteCursorHolder;
    MaterialLightDirector* mMaterialLightDirector;
    MaterialCategoryKeeper* mMaterialCategoryKeeper;
    SkyDirector* mSkyDirector;
    ShaderHolder* mShaderHolder;
    OcclusionCullingJudge* mOcclusionCullingJudge;
    VignettingDrawer* mVignettingDrawer;
    CameraBlurController* mCameraBlurController;
    ThunderRenderKeeper* mThunderRenderKeeper;
    StarrySky* mStarrySky;
    NoiseTextureKeeper* mNoiseTextureKeeper;
    CloudRenderKeeper* mCloudRenderKeeper;
    GpuMemAllocator* mGpuMemAllocator;
    FootPrintTextureKeeper* mFootPrintTextureKeeper;
    ProgramTextureKeeper* mProgramTextureKeeper;
    void* mRippleTextureKeeper;
    sead::PtrArrayImpl* _2a8;
    ViewRenderer* mViewRenderer;
    SubCameraRenderer* mSubCameraRenderer;
    TemporalInterlace* mTemporalInterface;
    PeripheryRendering* mPeripheryRendering;
    PostProcessingFilter* mPostProcessingFilter;
    GBufferArray* mDrawEnvGBufferArray;
    const sead::Camera* mDrawEnvCamera;
    const Projection* mDrawEnvProjection;
    s32 mDrawEnvViewIndex;
    s32 _2f4;
    agl::pfx::FilterAA* mFilterAA;
    AtmosScatter* mAtmosScatter;
    AtmosScatterDrawer* mAtmosScatterDrawer;
    GraphicsParamFilePath* mParamFilePath;
    void* filler[0xC0];
    // TODO: Fill these parameter variables once Int, Bool and Float are added to agl
    /*
    agl::utl::IParameterIO _318;
    agl::utl::IParameterObj _4E8;
    agl::utl::ParameterOfInt mAreaFindMode;
    agl::utl::ParameterOfInt mAtmosScatterType;
    agl::utl::ParameterOfBool mIsUsingUpdateAtmosCubeMap;
    agl::utl::ParameterOfBool mIsUsingOceanWave;
    agl::utl::ParameterOfInt mOccGroupNum;
    agl::utl::IParameterIO _5B8;
    agl::utl::IParameterObj _788;
    agl::utl::ParameterOfBool mIsUsingTemporal;
    agl::utl::ParameterOfBool mIsUsingPeriphery;
    agl::utl::ParameterOfBool mIsUsingStarrySky;
    agl::utl::ParameterOfFloat mCupeMapIntensityPower;
    agl::utl::ParameterOfFloat mCubeMapIntensityRange;
    agl::utl::ParameterOfFloat mLineLightAntiArtifact;
    agl::utl::ParameterOfFloat mMinRoughnessGGX;
    agl::utl::ParameterOfFloat mSphereLightDiffuseAdd;
    agl::utl::ParameterOfFloat mSpecularScale;
    agl::utl::ParameterOfFloat mLightUnitScale;
    agl::utl::ParameterOfFloat mLightColorScale;
    */
    UniformBlock* _918;
    Resource* _920;
    AreaObjDirector* mAreaObjDirector;
    ExecuteDirector* mExecuteDirector;
    EffectSystem* mEffectSystem;
    SceneCameraInfo* mSceneCameraInfo;
    const char* _948;
    void* _950[3];  // sead::OffsetList;
    VastGridMeshDirector* mVastGridMeshDirector;
    FullScreenTriangle* mFullScreenTriangle;
    s32 _978;
    ReducedBufferRenderer* mReducedBufferRenderer;
    ModelOcclusionCullingDirector* mModelOcclusionCullingDirector;
    ModelLodAllCtrl* mModelLodAllCtrl;
    ModelShaderHolder* mModelShaderHolder;
    PrepassTriangleCulling* mPrepassTriangleCulling;
    bool _9a8;
    ApplicationMessageReceiver* mApplicationMessageReceiver;
    void* _9b8;
};

static_assert(sizeof(GraphicsSystemInfo) == 0x9c0);

}  // namespace al
