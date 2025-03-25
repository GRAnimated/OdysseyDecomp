#pragma once

#include <math/seadMatrix.h>

namespace al {
class GraphicsSystemInfo;
class MaterialCategoryParam;
class ModelMaterialCategory;

class MaterialCategoryKeeper {
public:
    s32 getCategoryNum() const;
    const char* getCategoryName(s32) const;
    MaterialCategoryKeeper(al::GraphicsSystemInfo*);
    void initProjectResource();
    void endInit();
    const MaterialCategoryParam* getCurrentParam() const;
    void setMaterialInfo();
    void clearRequest();
    void update();
    void requestChangeShaderCategoryId(s32, const char*, const char*);
    void requestParam(s32, s32, const al::MaterialCategoryParam&);
    const char* getRefLightMapName(s32) const;
    const char* findRefLightMapName(const char*) const;
    const char* getRefSphereLightMapName(s32) const;
    const char* findRefSphereLightMapName(const char*) const;
    s32 findCategoryId(const char*) const;
    void registerModelMaterialCategory(al::ModelMaterialCategory*);
    void setModelProgProjMtx0(const sead::Matrix44f&);
    void setFootPrintParameter(s32, const sead::Matrix44f&, f32, f32);
    void requestChangeShaderTextureType(s32, const char*, const char*);

private:
    void* filler[7];
};
}  // namespace al
