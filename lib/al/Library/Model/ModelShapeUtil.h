#pragma once

#include <math/seadMatrix.h>

namespace al {
class GraphicsSystemInfo;
class ModelKeeper;
struct SklAnimRetargettingInfo;

const char* getMaterialName(const ModelKeeper*, s32);
s32 getMaterialIndex(const ModelKeeper*, const char*);
s32 getMaterialCount(const ModelKeeper*);
void hideMaterial(ModelKeeper*, const char*);
void hideMaterial(ModelKeeper*, s32);
void showMaterial(ModelKeeper*, const char*);
void showMaterial(ModelKeeper*, s32);
s32 calcPolygonNum(const ModelKeeper*, s32);
s32 calcPolygonNumCurrentLod(const ModelKeeper*);
s32 getLodLevel(const ModelKeeper*);
s32 getMaterialLodLevel(const ModelKeeper*);
s32 getLodLevelNoClamp(const ModelKeeper*);
bool isGreaterEqualMaxLodLevelNoClamp(const ModelKeeper*);
bool isGreaterMaxLodLevelNoClamp(const ModelKeeper*);
bool isLessMaxLodLevelNoClamp(const ModelKeeper*);
bool isMaxLodLevelNoClamp(const ModelKeeper*);
void validateLodModel(ModelKeeper*);
void invalidateLodModel(ModelKeeper*);
bool isEnableMaterialLod(const ModelKeeper*);
bool isValidateLodModel(const ModelKeeper*);
void setUvOffset(const ModelKeeper*, const sead::Vector2f&);
void setModelProjMtx0(const ModelKeeper*, const sead::Matrix44f&);
void setModelProjMtx1(const ModelKeeper*, const sead::Matrix44f&);
void setModelProjMtx2(const ModelKeeper*, const sead::Matrix44f&);
void setModelProgProjMtx0(const ModelKeeper*, const sead::Matrix44f&);
SklAnimRetargettingInfo* createRetargetInfo(const sead::Vector3f&, const char*, const char*,
                                            const char*);
s32 getJointNum(const ModelKeeper*);
s32 getJointIndex(const ModelKeeper*, const char*);
bool isExistJoint(const ModelKeeper*, const char*);
const char* getJointName(const ModelKeeper*, s32);
sead::Matrix34f* getJointMtxPtr(const ModelKeeper*, const char*);
sead::Matrix34f* getJointMtxPtrByIndex(const ModelKeeper*, s32);
sead::Matrix34f* getJointMtxPtrRaw(const ModelKeeper*, const char*);
sead::Matrix34f* getJointMtxPtrByIndexRaw(const ModelKeeper*, s32);
void getJointLocalTrans(sead::Vector3f*, const ModelKeeper*, const char*);
void getJointLocalTrans(sead::Vector3f*, const ModelKeeper*, s32);
s32 getParentJointIndex(const ModelKeeper*, s32);
void setJointVisibility(const ModelKeeper*, const char*, bool);
bool isJointVisibility(const ModelKeeper*, const char*);
void forceApplyCubeMap(ModelKeeper*, const GraphicsSystemInfo*, const char*);

}  // namespace al
