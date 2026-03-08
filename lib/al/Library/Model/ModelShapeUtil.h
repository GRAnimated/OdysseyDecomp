#pragma once

#include <math/seadMatrix.h>

namespace al {
class GraphicsSystemInfo;
class ModelKeeper;

bool isGreaterEqualMaxLodLevelNoClamp(const ModelKeeper* modelKeeper);
bool isLessMaxLodLevelNoClamp(const ModelKeeper* modelKeeper);
void setModelProjMtx0(const ModelKeeper* modelKeeper, const sead::Matrix44f&);
void forceApplyCubeMap(ModelKeeper* modelKeeper, const GraphicsSystemInfo*, const char*);
s32 getJointNum(const ModelKeeper* modelKeeper);
s32 getJointIndex(const ModelKeeper* modelKeeper, const char*);
const char* getJointName(const ModelKeeper* modelKeeper, s32 jointIndex);
s32 getParentJointIndex(const ModelKeeper* modelKeeper, s32 jointIndex);

}  // namespace al
