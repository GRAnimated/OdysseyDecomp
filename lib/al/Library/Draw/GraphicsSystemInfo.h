#pragma once

#include <container/seadOffsetList.h>
#include <container/seadPtrArray.h>
#include <container/seadStrTreeMap.h>
#include <gfx/seadCamera.h>
#include <gfx/seadFrameBuffer.h>
#include <utility/aglParameterIO.h>

namespace agl {
class DrawContext;
namespace pfx {
class FilterAA;
}
namespace sdw {
class PrimitiveOcclusion;
}  // namespace sdw
}  // namespace agl
namespace nn::g3d {
class ViewVolume {
    char size[0xE8];
};
}  // namespace nn::g3d

namespace al {
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
class SkyDirector;
class OcclusionCullingJudge;
class VignettingDrawer;
class CameraBlurController;
class ThunderRenderKeeper;
class StarrySky;
class NoiseTextureKeeper;
class CloudRenderKeeper;
class GpuMemAllocator;
class FootPrintTextureKeeper;
class ProgramTextureKeeper;
class RippleTextureKeeper;
class ViewRenderer;
class SubCameraRenderer;
class TemporalInterlace;
class PeripheryRendering;
class PostProcessingFilter;
class GBufferArray;
class Projection;
class UniformBlock;
class AtmosScatter;
class AtmosScatterDrawer;
class GraphicsParamFilePath;
class Resource;
class VastGridMeshDirector;
class FullScreenTriangle;
class ReducedBufferRenderer;
class ModelOcclusionCullingDirector;
class ModelLodAllCtrl;
class ModelShaderHolder;
class PrepassTriangleCulling;
class ApplicationMessageReceiver;

class GraphicsInitArg {
public:
    GraphicsInitArg(agl::DrawContext*, sead::FrameBuffer*);
    bool isUsingCubeMapAtmosScatter() const;
    u8 getAtmosScatterViewNum() const;

    s32 dword_0;
    bool field_4;
    u8 mAtmosScatterViewNum;
    bool field_6;
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

    void setField2F4(int set) { field_2F4 = set; }
    const al::ViewRenderer* getViewRenderer() const { return mViewRenderer; }

private:
    sead::StrTreeMap<128, sead::PtrArray<al::UniformBlock> const*> mViewIndexedUboArrayTree;
    al::GraphicsInitArg mInitArg;
    al::BloomDirector* mBloomDirector;
    al::CubeMapDirector* mCubeMapDirector;
    al::DirectionalLightKeeper* mDirectionalLightKeeper;
    al::GraphicsAreaDirector* mGraphicsAreaDirector;
    al::GraphicsPresetDirector* mGraphicsPresetDirector;
    al::DemoGraphicsController* mDemoGraphicsController;
    al::RadialBlurDirector* mRadialBlurDirector;
    al::PrePassLightKeeper* mPrePassLightKeeper;
    al::ShaderEnvTextureKeeper* mShaderEnvTextureKeeper;
    al::ShadowDirector* mShadowDirector;
    al::DepthOfFieldDrawer* mDepthOfFieldDrawer;
    al::GraphicsQualityController* mGraphicsQualityController;
    al::ShaderMirrorDirector* mShaderMirrorDirector;
    al::GraphicsParamRequesterImpl* field_D0;
    al::GraphicsParamRequesterImpl* mColorCorrection;
    al::FlareFilterDirector* mFlareFilterDirector;
    al::GodRayDirector* mGodRayDirector;
    al::FogDirector* mFogDirector;
    al::OccludedEffectDirector* mOccludedEffectDirector;
    al::LightStreakDirector* mLightStreakDirector;
    al::HdrCompose* mHdrCompose;
    al::SSIIKeeper* mSSIIKeeper;
    agl::sdw::PrimitiveOcclusion* mPrimitiveOcclusion;
    nn::g3d::ViewVolume mViewVolume;
    void* field_208;
    al::OceanWave* mOceanWave;
    al::RandomTextureKeeper* mRandomTextureKeeper;
    al::WorldAODirector* mWorldAODirector;
    al::PointSpriteCursorHolder* mPointSpriteCursorHolder;
    al::MaterialLightDirector* mMaterialLightDirector;
    al::MaterialCategoryKeeper* mMaterialCategoryKeeper;
    al::SkyDirector* mSkyDirector;
    al::ShaderHolder* mShaderHolder;
    al::OcclusionCullingJudge* mOcclusionCullingJudge;
    al::VignettingDrawer* mVignettingDrawer;
    al::CameraBlurController* mCameraBlurController;
    al::ThunderRenderKeeper* mThunderRenderKeeper;
    al::StarrySky* mStarrySky;
    al::NoiseTextureKeeper* mNoiseTextureKeeper;
    al::CloudRenderKeeper* mCloudRenderKeeper;
    al::GpuMemAllocator* mGpuMemAllocator;
    al::FootPrintTextureKeeper* mFootPrintTextureKeeper;
    al::ProgramTextureKeeper* mProgramTextureKeeper;
    al::RippleTextureKeeper* mRippleTextureKeeper;
    sead::PtrArrayImpl* field_2A8;
    al::ViewRenderer* mViewRenderer;
    al::SubCameraRenderer* mSubCameraRenderer;
    al::TemporalInterlace* mTemporalInterface;
    al::PeripheryRendering* mPeripheryRendering;
    al::PostProcessingFilter* mPostProcessingFilter;
    al::GBufferArray* mDrawEnvGBufferArray;
    const sead::Camera* mDrawEnvCamera;
    const al::Projection* mDrawEnvProjection;
    int mDrawEnvViewIndex;
    int field_2F4;
    agl::pfx::FilterAA* mFilterAA;
    al::AtmosScatter* mAtmosScatter;
    al::AtmosScatterDrawer* mAtmosScatterDrawer;
    al::GraphicsParamFilePath* mParamFilePath;
    /*
    agl::utl::IParameterIO field_318;
    agl::utl::IParameterObj field_4E8;
    agl::utl::ParameterOfInt mAreaFindMode;
    agl::utl::ParameterOfInt mAtmosScatterType;
    agl::utl::ParameterOfBool mIsUsingUpdateAtmosCubeMap;
    agl::utl::ParameterOfBool mIsUsingOceanWave;
    agl::utl::ParameterOfInt mOccGroupNum;
    agl::utl::IParameterIO field_5B8;
    agl::utl::IParameterObj field_788;
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
    char params[0x918 - 0x318];
    al::UniformBlock* field_918;
    al::Resource* field_920;
    al::AreaObjDirector* mAreaObjDirector;
    al::ExecuteDirector* mExecuteDirector;
    al::EffectSystem* mEffectSystem;
    al::SceneCameraInfo* mSceneCameraInfo;
    const char* field_948;
    void* field_950[3];  // sead::OffsetList
    al::VastGridMeshDirector* mVastGridMeshDirector;
    al::FullScreenTriangle* mFullScreenTriangle;
    s32 field_978;
    al::ReducedBufferRenderer* mReducedBufferRenderer;
    al::ModelOcclusionCullingDirector* mModelOcclusionCullingDirector;
    al::ModelLodAllCtrl* mModelLodAllCtrl;
    al::ModelShaderHolder* mModelShaderHolder;
    al::PrepassTriangleCulling* mPrepassTriangleCulling;
    bool field_9A8;
    al::ApplicationMessageReceiver* mApplicationMessageReceiver;
    void* field_9B8;
};

static_assert(sizeof(GraphicsSystemInfo) == 0x9c0);

}  // namespace al
