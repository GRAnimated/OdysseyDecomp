#pragma once

#include <gfx/seadCamera.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class SceneCameraInfo;
class IUseCamera;
class Projection;
class CameraSubTargetBase;
class CameraSubTargetTurnParam;
class LiveActor;

Projection getProjection(const IUseCamera* cameraHolder, s32 cameraNum);

const sead::Vector3f& getCameraPos(const IUseCamera* cameraHolder, s32 cameraNum);
void calcCameraUpDir(sead::Vector3f* out, const IUseCamera* cameraHolder, s32 cameraNum);
s32 getViewNumMax(const IUseCamera* cameraHolder);
CameraSubTargetBase* createActorCameraSubTarget(const LiveActor* actor, const sead::Vector3f*);
void initCameraSubTargetTurnParam(CameraSubTargetBase* cameraSubTarget,
                                  const CameraSubTargetTurnParam* params);
void setCameraPlacementSubTarget(IUseCamera* cameraHolder, CameraSubTargetBase* cameraSubTarget);
void resetCameraPlacementSubTarget(IUseCamera* cameraHolder, CameraSubTargetBase* cameraSubTarget);
const sead::Matrix34f& getViewMtx(const IUseCamera* cameraHolder, s32 cameraNum);
void calcCameraFront(sead::Vector3f* out, const IUseCamera* cameraHolder, s32 cameraNum);
}  // namespace al

void validateFixPointCameraUsePreCameraPos(al::CameraTicket*);

namespace alCameraFunction {
al::CameraTicket* initCameraNoPlacementInfoNoSave(al::CameraPoser*, const al::IUseCamera*,
                                                  al::PlacementId const*, const char*, s32,
                                                  const sead::Matrix34f&);
al::CameraTicket* initCameraNoSave(al::CameraPoser*, const al::IUseCamera*,
                                   const al::ActorInitInfo&, const char*, s32);
al::CameraTicket* initCamera(al::CameraPoser*, const al::IUseCamera*, const al::ActorInitInfo&,
                             const char*, s32);
al::CameraTicket* initCameraNoSave(al::CameraPoser*, const al::IUseCamera*,
                                   const al::PlacementInfo&, const char*, s32);
al::CameraTicket* initCameraNoPlacementInfo(al::CameraPoser*, const al::IUseCamera*,
                                            al::PlacementId const*, const char*, s32,
                                            const sead::Matrix34f&);
void requestCameraShakeLoop(const al::IUseCamera*, const char*);
al::CameraTicket* initCamera(al::CameraPoser*, const al::IUseCamera*, const al::PlacementInfo&,
                             const char*, s32);
al::CameraTicket* initAreaCamera(const al::IUseCamera*, const al::PlacementInfo&, const char*);
al::CameraTicket* initForceAreaCamera(const al::IUseCamera*, const al::PlacementInfo&, const char*);
void initPriorityBossField(al::CameraTicket*);
void initPriorityCapture(al::CameraTicket*);
void initPriorityObject(al::CameraTicket*);
void initPrioritySafetyPointRecovery(al::CameraTicket*);
void initPriorityDemoTalk(al::CameraTicket*);
void initPriorityDemo(al::CameraTicket*);
void initPriorityDemo2(al::CameraTicket*);
bool isCurrentCameraPriorityPlayer(const al::IUseCamera*, s32);
void setPoserNearClipDistance(al::CameraTicket*, f32);
f32 getNearClipDistance(const al::IUseCamera*, s32);
void validateCameraArea2D(al::IUseCamera*);
void invalidateCameraArea2D(al::IUseCamera*);
void validateCameraAreaKids(al::IUseCamera*);
bool isValidCameraAreaKids(const al::CameraFlagCtrl*);
void onSeparatePlayMode(al::IUseCamera*);
void offSeparatePlayMode(al::IUseCamera*);
void validateResetPoseNextCamera(al::CameraTicket*);
void validateKeepPreSelfPoseNextCamera(al::CameraTicket*);
void validateCameraInterpoleEaseOut(al::CameraTicket*);
void onForceCollideAtStartInterpole(al::CameraTicket*);
void initCameraSettingCloudSea(al::IUseCamera*, f32);
al::CameraTicket* initMirrorAreaCamera(const al::IUseCamera*, const al::PlacementInfo&,
                                       const char*);
}  // namespace alCameraFunction