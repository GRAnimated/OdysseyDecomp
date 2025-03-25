#pragma once

#include <basis/seadTypes.h>
#include <common/aglDrawContext.h>
#include <heap/seadHeap.h>
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
class ShaderHolder;
class ModelLodCtrl;
struct EnvTexInfo;
class ModelOcclusionCullingDirector;
class PrepassTriangleCulling;
class ShadowDirector;
class DitherAnimator;
class ModelMaterialCategory;

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
    void calc(const sead::Matrix34f&, const sead::Vector3f&);
    bool calcView();
    void calcModelObjBoundingWithOffset(nn::g3d::Sphere*) const;
    void setCameraInfo(const sead::Matrix34f*, const sead::Matrix34f*, const sead::Matrix44f*,
                       const sead::Matrix44f*);
    void getShapeObj(s32) const;

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

    ModelMaterialCategory* getModelMaterialCategory() const { return mModelMaterialCategory; }

private:
    nn::g3d::ModelObj* mModelObj;
    s32 _8;
    GpuMemAllocator* mGpuMemAllocator;
    ModelShaderHolder* mShaderHolder;
    s32 mBlockBufferSize;
    unsigned char padding1[316];
    bool _160;
    bool mIsNeedUpdateModel;
    bool _162;
    bool _163;
    bool mIsEnableZPrePass;
    bool mIsValidateZPrePass;
    bool field_166;
    bool mIsExistZPrePass;
    bool field_168;
    bool mIsNeedUpdate;
    unsigned char padding2[0xB];
    f32 mAlphaMask;
    unsigned char padding3[0x1F8];
    ActorDitherAnimator* mActorDitherAnimator;
    ModelMaterialCategory* mModelMaterialCategory;
    unsigned char padding4[0xC0];
};

static_assert(sizeof(ModelCtrl) == 0x448);
}  // namespace al
