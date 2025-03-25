#pragma once

#include <math/seadBoundBox.h>
#include <math/seadMatrix.h>
#include <nn/g3d/ModelObj.h>

namespace al {
class ModelCtrl;
}

namespace alModelFunction {
f32 calcBoundingSphere(const al::ModelCtrl*);
f32 calcBoundingRadius(const al::ModelCtrl*);
s32 getLodModelCount(const nn::g3d::ModelObj*);
bool isExistLodModel(const nn::g3d::ModelObj*);
void calcBoundingBox(sead::BoundBox3f*, const al::ModelCtrl*);
void calcBoundingBoxMtx(sead::Matrix34f*, const al::ModelCtrl*);
bool isDisablePrepassCulling(const nn::g3d::ModelObj&);
bool isEnablePrepassCulling(const nn::g3d::ModelObj&, s32);
}  // namespace alModelFunction
