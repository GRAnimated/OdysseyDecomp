#pragma once

#include <basis/seadTypes.h>

namespace nn::g3d {
class MaterialObj;
}

namespace sead {
class GraphicsContext;
class GraphicsContextMRT;
}

namespace al {
class Scene;
class LiveActor;

void setDepthFuncNearDraw(sead::GraphicsContext* context);
void setDepthFuncFarDraw(sead::GraphicsContext* context);
void setDepthFuncNearDraw(sead::GraphicsContextMRT* context);
void setDepthFuncFarDraw(sead::GraphicsContextMRT* context);
bool isUsingReverseProjection();
bool isDepthFuncReverse();
f32 getDepthClearValue();
bool getAlphaTestEnable(const nn::g3d::MaterialObj* material);
void setBlendCtrlToContext(sead::GraphicsContextMRT* context,
                           const nn::g3d::MaterialObj* material);
}  // namespace al

namespace alGraphicsFunction {
bool isGraphicsQualityModeConsole(const al::LiveActor* actor);
bool isGraphicsQualityModeHandheld(const al::LiveActor* actor);
void forceGraphicsQualityModeConsole(al::Scene* scene);
void forceGraphicsQualityModeHandheld(al::Scene* scene);
void forceGraphicsQualityModeSnapShot(al::Scene* scene);
void requestChangeShaderVariation(const al::LiveActor* actor, const char* materialName,
                                  const char* variationName, bool isRefresh);
void requestUpdateMaterialInfo(al::Scene* scene);
void validateGpuStressAnalyzer(al::Scene* scene);
void invalidateGpuStressAnalyzer(al::Scene* scene);
}  // namespace alGraphicsFunction
