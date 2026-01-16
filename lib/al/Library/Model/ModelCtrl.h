#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>

namespace nn::g3d {
class ModelObj;
class ResModel;
class Sphere;
}  // namespace nn::g3d

namespace al {
class GpuMemAllocator;
class ModelShaderHolder;
class Resource;
class ActorDitherAnimator;
class GraphicsQualityInfo;
class ModelOcclusionQuery;

class ModelCtrl {
public:
    ModelCtrl();
    ~ModelCtrl();

    void initialize(nn::g3d::ResModel*, s32, s32, sead::Heap*, al::ShaderHolder*);
    void tryBindShader();
    void tryUpdateModelAdditionalInfoUbo(s32);
    void updateWorldMatrix(const sead::Matrix34f&, const sead::Vector3f&);
    void updateGpuBuffer(s32);
    void calcBounding();
    void getLodLevel() const;
    void getLodLevelDepthShadow() const;
    void calcBoundingLod(s32);
    void updateQueryBox();
    void updateModelDrawBuffer(s32);
    void updateGpuBufferAll();
    bool isShapeVisible(s32) const;
    void setCubeMapIndexAllShape(s32);
    void recreateDisplayList();
    void setMaterialProgrammable(s32, bool);
    bool isMaterialProgrammable(s32);
    void setSkeletonUpdateInfo(bool, const sead::Matrix34f&, const sead::Vector3f&);
    void setDirtyTexture();
    void onZPrePass();
    void offZPrePass();
    EnvTexInfo& getEnvTexInfo(s32) const;
    void requestModelAdditionalInfoUbo();
    void setLodCtrl(al::ModelLodCtrl*);
    void getLodLevelMax() const;
    void getLodLevelMaterial() const;
    void getLodLevelNoClamp() const;
    void setLodLevelForce(s32);
    void updateLodCtrl();
    void setDitherAnimator(al::DitherAnimator*);
    void updateDitherAnimator();
    void checkChangeDisplayList();
    void addToDrawerCulling();
    void removeFromDrawerCulling();
    void updateSubMesh();
    void setModelMaterialCategory(const al::ModelMaterialCategory*);
    void setModelAlphaMask(f32);
    void setModelUvOffset(const sead::Vector2f&);
    void setModelProjMtx0(const sead::Matrix44f&);
    void setModelProjMtx1(const sead::Matrix44f&);
    void setModelProjMtx2(const sead::Matrix44f&);
    void setModelProgProjMtx0(const sead::Matrix44f&);
    void setModelProgProjMtx1(const sead::Matrix44f&);
    void setModelProgProjMtx2(const sead::Matrix44f&);
    void setModelProgProjMtx3(const sead::Matrix44f&);
    void setModelProgConstant0(f32);
    void setModelProgConstant1(f32);
    void setModelProgConstant2(f32);
    void setModelProgConstant3(f32);
    void setModelProgConstant4(f32);
    void setModelProgConstant5(f32);
    void setModelProgConstant6(f32);
    void setModelProgConstant7(f32);
    void setNormalAxisXScale(f32);
    void calcCameraToBoundingSphereDistance() const;
    bool isUseLocalShadowMap() const;
    void validateOcclusionQuery();
    void invalidateOcclusionQuery();
    bool isValidOcclusionQuery() const;
    void createUniqShader();
    bool isCreateUniqShader(s32);
    void getUniqModelShader(s32);
    void getUniqModelShaderAssgin(s32);
    void pushDisplayListModel(agl::DisplayList*);
    void pushDisplayListShape(agl::DisplayList*, s32);
    void getModelShapeCtrl(s32) const;
    void initResource(al::Resource*, al::Resource*);
    void initModel(al::GpuMemAllocator*, al::ModelShaderHolder*, al::ModelOcclusionCullingDirector*,
                   al::ShadowDirector*, al::PrepassTriangleCulling*, s32, s32);
    void tryCreateCulledIndexBuffer();
    void show();
    void hide();
    void recreateDisplayList();
    void setCameraInfo(const sead::Matrix34f*, const sead::Matrix34f*, const sead::Matrix44f*,
                       const sead::Matrix44f*);

    nn::g3d::ModelObj* getModelObj() const { return mModelObj; }

    bool isNeedUpdateModel() const { return mIsNeedUpdateModel; }

    bool isEnableZPrePass() const { return mIsEnableZPrePass; }

    bool isValidateZPrePass() const { return mIsValidateZPrePass; }

    void validateZPrePass() { mIsValidateZPrePass = true; }

    void invalidateZPrePass() { mIsValidateZPrePass = false; }

    bool get_166() const { return field_166; }

    bool isExistZPrePass() const { return mIsExistZPrePass; }

    bool isNeedUpdate() const { return mIsNeedUpdate; }

    void update() { mIsNeedUpdate = false; }

    f32 getAlphaMask() const { return mAlphaMask; }

    ActorDitherAnimator* getActorDitherAnimator() const { return mActorDitherAnimator; }

    s32 getCalcViewCore() const { return mCalcViewCore; }

    void setCalcViewCore(s32 core) { mCalcViewCore = core; }

    void setGraphicsQualityInfo(GraphicsQualityInfo* info) { mGraphicsQualityInfo = info; }

    void setModelOcclusionQuery(ModelOcclusionQuery* query) { mModelOcclusionQuery = query; }

private:
    nn::g3d::ModelObj* mModelObj;
    s32 _8;
    GpuMemAllocator* mGpuMemAllocator;
    ModelShaderHolder* mShaderHolder;
    s32 mBlockBufferSize;
    unsigned char padding1[332];
    GraphicsQualityInfo* mGraphicsQualityInfo;
    unsigned char padding2[514];
    ActorDitherAnimator* mActorDitherAnimator;
    unsigned char padding3[36];
    s32 mCalcViewCore;
    s32 pad_3b0;
    unsigned char padding4[124];
    ModelOcclusionQuery* mModelOcclusionQuery;
    unsigned char padding5[16];
};

static_assert(sizeof(ModelCtrl) == 0x448);
}  // namespace al
