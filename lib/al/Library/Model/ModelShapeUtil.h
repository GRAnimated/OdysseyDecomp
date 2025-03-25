#pragma once

#include <math/seadMatrix.h>

namespace al {
class ModelKeeper;
class GraphicsSystemInfo;
class SklAnimRetargettingInfo;

const char* getMaterialName(const al::ModelKeeper*, s32);
s32 getMaterialIndex(const al::ModelKeeper*, const char*);
s32 getMaterialCount(const al::ModelKeeper*);
void hideMaterial(al::ModelKeeper*, const char*);
void hideMaterial(al::ModelKeeper*, s32);
void showMaterial(al::ModelKeeper*, const char*);
void showMaterial(al::ModelKeeper*, s32);
s32 calcPolygonNum(const al::ModelKeeper*, s32);
s32 calcPolygonNumCurrentLod(const al::ModelKeeper*);
s32 getLodLevel(const al::ModelKeeper*);
s32 getMaterialLodLevel(const al::ModelKeeper*);
s32 getLodLevelNoClamp(const al::ModelKeeper*);
bool isGreaterEqualMaxLodLevelNoClamp(const al::ModelKeeper*);
bool isGreaterMaxLodLevelNoClamp(const al::ModelKeeper*);
bool isLessMaxLodLevelNoClamp(const al::ModelKeeper*);
bool isMaxLodLevelNoClamp(const al::ModelKeeper*);
void validateLodModel(al::ModelKeeper*);
void invalidateLodModel(al::ModelKeeper*);
bool isEnableMaterialLod(const al::ModelKeeper*);
bool isValidateLodModel(const al::ModelKeeper*);
void setUvOffset(const al::ModelKeeper*, const sead::Vector2f&);
void setModelProjMtx0(const al::ModelKeeper*, const sead::Matrix44f&);
void setModelProjMtx1(const al::ModelKeeper*, const sead::Matrix44f&);
void setModelProjMtx2(const al::ModelKeeper*, const sead::Matrix44f&);
void setModelProgProjMtx0(const al::ModelKeeper*, const sead::Matrix44f&);
SklAnimRetargettingInfo* createRetargetInfo(const sead::Vector3f&, const char*, const char*,
                                            const char*);
s32 getJointNum(const al::ModelKeeper*);
s32 getJointIndex(const al::ModelKeeper*, const char*);
bool isExistJoint(const al::ModelKeeper*, const char*);
const char* getJointName(const al::ModelKeeper*, s32);
sead::Matrix34f* getJointMtxPtr(const al::ModelKeeper*, const char*);
sead::Matrix34f* getJointMtxPtrByIndex(const al::ModelKeeper*, s32);
sead::Matrix34f* getJointMtxPtrRaw(const al::ModelKeeper*, const char*);
sead::Matrix34f* getJointMtxPtrByIndexRaw(const al::ModelKeeper*, s32);
void getJointLocalTrans(sead::Vector3f*, const al::ModelKeeper*, const char*);
void getJointLocalTrans(sead::Vector3f*, const al::ModelKeeper*, s32);
s32 getParentJointIndex(const al::ModelKeeper*, s32);
void setJointVisibility(const al::ModelKeeper*, const char*, bool);
bool isJointVisibility(const al::ModelKeeper*, const char*);
void forceApplyCubeMap(ModelKeeper*, const GraphicsSystemInfo*, const char*);

}  // namespace al
