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

namespace al {
const al::SceneCameraInfo* getSceneCameraInfo(const IUseCamera*);
s32 getViewNumMax(const IUseCamera*);
s32 getViewNumMax(const SceneCameraInfo*);
bool isValidView(const IUseCamera*, s32);
bool isValidView(const SceneCameraInfo*, s32);
const char16* getViewName(const IUseCamera*, s32);
const char16* getViewName(const SceneCameraInfo*, s32);
const sead::Matrix34f& getViewMtx(const IUseCamera*, s32);
const sead::Matrix34f& getViewMtx(const SceneCameraInfo*, s32);
sead::Matrix34f* getViewMtxPtr(const IUseCamera*, s32);
sead::Matrix34f* getViewMtxPtr(const SceneCameraInfo*, s32);
const sead::Matrix34f& getProjectionMtx(const IUseCamera*, s32);
const sead::Matrix34f& getProjectionMtx(const SceneCameraInfo*, s32);
sead::Matrix34f* getProjectionMtxPtr(const IUseCamera*, s32);
sead::Matrix34f* getProjectionMtxPtr(const SceneCameraInfo*, s32);
const sead::LookAtCamera& getLookAtCamera(const IUseCamera*, s32);
const sead::LookAtCamera& getLookAtCamera(const SceneCameraInfo*, s32);
sead::Projection getProjectionSead(const IUseCamera*, s32);
sead::Projection getProjectionSead(const SceneCameraInfo*, s32);
const Projection& getProjection(const IUseCamera*, s32);
const Projection& getProjection(const SceneCameraInfo*, s32);
const sead::Vector3f& getCameraPos(const IUseCamera*, s32);
const sead::Vector3f& getCameraPos(const SceneCameraInfo*, s32);
const sead::Vector3f& getCameraAt(const IUseCamera*, s32);
const sead::Vector3f& getCameraAt(const SceneCameraInfo*, s32);
const sead::Vector3f& getCameraUp(const IUseCamera*, s32);
const sead::Vector3f& getCameraUp(const SceneCameraInfo*, s32);
f32 getFovyDegree(const IUseCamera*, s32);
f32 getFovyDegree(const SceneCameraInfo*, s32);
f32 getFovy(const IUseCamera*, s32);
f32 getFovy(const SceneCameraInfo*, s32);
f32 getNear(const IUseCamera*, s32);
f32 getNear(const SceneCameraInfo*, s32);
f32 getFar(const IUseCamera*, s32);
f32 getFar(const SceneCameraInfo*, s32);
f32 calcCameraDistance(const IUseCamera*, s32);
f32 calcFovxDegree(const IUseCamera*, s32);
f32 calcCurrentFovyRate(const IUseCamera*, s32);
void calcCameraFront(sead::Vector3f*, const IUseCamera*, s32);
void setNearClipDistance(const IUseCamera*, f32, s32);
void setFarClipDistance(const IUseCamera*, f32, s32);
void setCurrentCameraPose(CameraPoseInfo*, const IUseCamera*);
void calcCameraDir(sead::Vector3f*, const IUseCamera*, s32);
void calcCameraLookDir(sead::Vector3f*, const IUseCamera*, s32);
void calcCameraSideDir(sead::Vector3f*, const IUseCamera*, s32);
void calcCameraUpDir(sead::Vector3f*, const IUseCamera*, s32);
void tryCalcCameraDir(sead::Vector3f*, const SceneCameraInfo*, s32);
void tryCalcCameraDirH(sead::Vector3f*, const SceneCameraInfo*, const sead::Vector3f&, s32);
void tryCalcCameraLookDirH(sead::Vector3f*, const SceneCameraInfo*, const sead::Vector3f&, s32);
void startCamera(const IUseCamera*, CameraTicket*, s32);
void startCameraSub(const IUseCamera*, CameraTicket*, s32);
void startAnimCamera(const IUseCamera*, CameraTicket*, const char*, s32);
void startAnimCameraAnim(CameraTicket*, const char*, s32, s32, s32);
void startAnimCameraWithStartStepAndEndStepAndPlayStep(const IUseCamera*, CameraTicket*,
                                                       const char*, s32, s32, s32, s32);
void endCamera(const IUseCamera*, CameraTicket*, s32, bool);
void endCameraWithNextCameraPose(const IUseCamera*, CameraTicket*, const CameraPoseInfo*, s32);
void endCameraSub(const IUseCamera*, CameraTicket*, s32);
bool isActiveCamera(const CameraTicket*);
CameraTicket* initObjectCamera(const IUseCamera*, const PlacementInfo&, const char*, const char*);
CameraTicket* initObjectCamera(const IUseCamera*, const ActorInitInfo&, const char*, const char*);
CameraTicket* initObjectCameraNoPlacementInfo(const IUseCamera*, const char*, const char*);
CameraTicket* initFixCamera(const IUseCamera*, const char*, const sead::Vector3f&,
                            const sead::Vector3f&);
CameraTicket* initFixDoorwayCamera(const IUseCamera*, const char*, const sead::Vector3f&,
                                   const sead::Vector3f&);
CameraTicket* initFixActorCamera(const LiveActor*, const ActorInitInfo&, const char*,
                                 const sead::Vector3f&, f32, f32, f32, bool);
CameraTicket* initFixLookCamera(LiveActor*, const ActorInitInfo&, const char*);
CameraTicket* initFixTalkCamera(const LiveActor*, const ActorInitInfo&, const char*,
                                const sead::Vector3f&, f32, f32, f32, bool);
CameraTicket* initFixFishingCamera(const LiveActor*, const ActorInitInfo&, const char*,
                                   const sead::Vector3f&, const sead::Vector3f&, f32, f32, f32,
                                   bool);
CameraTicket* initFixPointCamera(const IUseCamera*, const ActorInitInfo&, const char*, bool);
CameraTicket* initLookDownCamera(const LiveActor*, const ActorInitInfo&, const char*);
CameraTicket* initProgramableCamera(const IUseCamera*, const ActorInitInfo&, const char*,
                                    const sead::Vector3f*, const sead::Vector3f*,
                                    const sead::Vector3f*);
CameraTicket* initProgramableCamera(const IUseCamera*, const char*, const sead::Vector3f*,
                                    const sead::Vector3f*, const sead::Vector3f*);
CameraTicket* initProgramableCameraWithCollider(const IUseCamera*, const ActorInitInfo&,
                                                const char*, const sead::Vector3f*,
                                                const sead::Vector3f*, const sead::Vector3f*);
CameraTicket* initProgramableAngleCamera(const IUseCamera*, const PlacementInfo&, const char*,
                                         const sead::Vector3f*, const float*, const float*,
                                         const float*);
CameraTicket* initProgramableCameraKeepColliderPreCamera(const IUseCamera*, const ActorInitInfo&,
                                                         const char*, const sead::Vector3f*,
                                                         const sead::Vector3f*,
                                                         const sead::Vector3f*);
CameraTicket* initShooterCameraSingle(const IUseCamera*, const char*);
CameraTicket* initTowerCameraWithSave(const IUseCamera*, const sead::Vector3f*,
                                      const ActorInitInfo&, const char*);
CameraTicket* initTowerCamera(const IUseCamera*, const sead::Vector3f*, const ActorInitInfo&,
                              const char*);
CameraTicket* initBossBattleCamera(const IUseCamera*, const sead::Vector3f*, const ActorInitInfo&,
                                   const char*);
CameraTicket* initProgramableCameraAngleSwing(CameraTicket*);
CameraTicket* initFollowCameraSimple(const IUseCamera*, const char*);
CameraTicket* initFollowCameraSimple(const IUseCamera*, const ActorInitInfo&, const char*);
CameraTicket* initDemoObjectCamera(const IUseCamera*, const ActorInitInfo&, const char*,
                                   const char*);
CameraTicket* initDemoProgramableCamera(const IUseCamera*, const ActorInitInfo&, const char*,
                                        const sead::Vector3f*, const sead::Vector3f*,
                                        const sead::Vector3f*);
CameraTicket* initDemoProgramableCameraKeepColliderPreCamera(const IUseCamera*,
                                                             const ActorInitInfo&, const char*,
                                                             const sead::Vector3f*,
                                                             const sead::Vector3f*,
                                                             const sead::Vector3f*);
CameraTicket* initDemoAnimCamera(const IUseCamera*, const ActorInitInfo&, Resource const*,
                                 const sead::Matrix34f*, const char*);
CameraTicket* initAnimCamera(const IUseCamera*, const ActorInitInfo&, Resource const*,
                             const sead::Matrix34f*, const char*);
CameraTicket* initDemoAnimCamera(const LiveActor*, const ActorInitInfo&, const char*);
void loadActorCameraParam(CameraTicket*, const LiveActor*, const char*, const char*);
void loadActorCameraParamInitFile(CameraTicket*, const LiveActor*, const char*);
void setFixActorCameraTarget(CameraTicket*, const LiveActor*);
void setFixActorCameraAngleH(CameraTicket*, f32);
void setTowerCameraDistance(CameraTicket*, f32);
void setTowerCameraStartAngleV(CameraTicket*, f32);
void setTowerCameraUserMarginAngleH(CameraTicket*, f32);
void resetTowerCameraUserMarginAngleH(CameraTicket*);
void resetTowerCameraInputRotate(CameraTicket*, f32, s32);
CameraTicket* initSubjectiveCamera(const IUseCamera*, const ActorInitInfo&, const char*);
CameraTicket* initSubjectiveCameraNoSave(const IUseCamera*, const char*);
sead::Vector3f getSubjectiveCameraOffsetUp(const CameraTicket*);
sead::Vector3f getSubjectiveCameraOffsetFront();
void setSubjectiveCameraStartAngleH(const CameraTicket*, f32);
void validateSubjectiveCameraResetAngleH(CameraTicket*);
void requestSubjectiveCameraZoomIn(CameraTicket*);
CameraTicket* initParallelCamera(const IUseCamera*, const char*);
CameraTicket* initParallelCamera(const IUseCamera*, const ActorInitInfo&, const char*);
void setParallelCameraLookAtOffset(const CameraTicket*, const sead::Vector3f&);
void setParallelCameraDistance(const CameraTicket*, f32);
void setParallelCameraAngleH(const CameraTicket*, f32);
void setParallelCameraAngleV(const CameraTicket*, f32);
CameraTicket* initQuickTurnCamera(const IUseCamera*, const char*);
void setQuickTurnCameraFollow(CameraTicket*);
void setQuickTurnCameraRotateFast(CameraTicket*);
CameraTicket* initRaceCamera(const IUseCamera*, const ActorInitInfo&, const char*);
void setRaceCameraFrontDirPtr(const CameraTicket*, const sead::Vector3f*);
void setRaceCameraDistance(const CameraTicket*, f32);
void setRaceCameraOffsetY(const CameraTicket*, f32);
void setRaceCameraAngleDegreeV(const CameraTicket*, f32);
CameraTicket* initCartCamera(const IUseCamera*, const ActorInitInfo&, const char*);
void stopCartCamera(const CameraTicket*);
void restartCartCamera(const CameraTicket*);
CameraTicket* initActorRailParallelCamera(const LiveActor*, const ActorInitInfo&, const char*);
CameraTicket* initKinopioBrigadeCamera(const IUseCamera*, const ActorInitInfo&, const char*);
CameraTicket* initAnimCamera(const LiveActor*, const ActorInitInfo&, const char*);
void validateAnimCameraAngleSwing(CameraTicket*);
void invalidateAnimCameraAngleSwing(CameraTicket*);
void setAnimCameraBaseMtxPtr(CameraTicket*, const sead::Matrix34f*);
CameraTicket* initEntranceCamera(const IUseCamera*, const PlacementInfo&, const char*);
CameraTicket* initEntranceCamera(const IUseCamera*, const ActorInitInfo&, const char*);
CameraTicket* initEntranceCameraNoSave(const IUseCamera*, const PlacementInfo&, const char*);
void setEntranceCameraParam(CameraTicket*, f32, const sead::Vector3f&, const sead::Vector3f&);
void setEntranceCameraLookAt(CameraTicket*, const sead::Vector3f&);
void invalidateEndEntranceCamera(LiveActor*);
void invalidateEndEntranceCameraWithName(IUseCamera*, const char*);
void validateEndEntranceCamera(IUseCamera*);
bool isPlayingEntranceCamera(const IUseCamera*, s32);
void setCameraInterpoleStep(CameraTicket*, s32);
void setCameraFovyDegree(CameraTicket*, f32);
CameraInput* createSimpleCameraInput(int);
void setCameraInput(IUseCamera*, ICameraInput const*);
void setViewCameraInput(IUseCamera*, ICameraInput const*, s32);
bool isExistCameraInputAtDisableTiming(const IUseCamera*, s32);
ActorCameraTarget* createActorCameraTarget(const LiveActor*, f32);
ActorCameraTarget* createActorCameraTarget(const LiveActor*, const sead::Vector3f*);
ActorCameraTarget* createActorJointCameraTarget(const LiveActor*, const char*);
ActorCameraTarget* createActorMatrixCameraTarget(const LiveActor*, const sead::Matrix34f*);
bool isActiveCameraTarget(CameraTargetBase const*);
void setCameraTarget(IUseCamera*, CameraTargetBase*);
void resetCameraTarget(IUseCamera*, CameraTargetBase*);
void createActorCameraSubTarget(const LiveActor*, const sead::Vector3f*);
void createActorBackAroundCameraSubTarget(const LiveActor*, const sead::Vector3f*);
void createTransCameraSubTarget(const char*, const sead::Vector3f*);
void initCameraSubTargetTurnParam(CameraSubTargetBase*, const CameraSubTargetTurnParam*);
bool isActiveCameraSubTarget(const CameraSubTargetBase*);
void setCameraSubTarget(IUseCamera*, CameraSubTargetBase*);
void resetCameraSubTarget(IUseCamera*, CameraSubTargetBase*);
void setCameraPlacementSubTarget(IUseCamera*, CameraSubTargetBase*);
void resetCameraPlacementSubTarget(IUseCamera*, CameraSubTargetBase*);
CameraDistanceCurve* getCameraDistanceRocketFlowerCurve();
void setViewCameraTarget(IUseCamera*, CameraTargetBase*, s32);
void startCameraShakeByAction(const LiveActor*, const char*, const char*, s32, s32);
void startCameraShakeByHitReaction(const IUseCamera*, const char*, const char*, const char*, s32,
                                   s32);
void requestCameraLoopShakeWeak(const IUseCamera*);
bool isActiveCameraInterpole(const IUseCamera*, s32);
bool isActiveCameraInterpole(const SceneCameraInfo*, s32);
void startCameraInterpole(const IUseCamera*, s32, s32);
void requestCancelCameraInterpole(const IUseCamera*, s32);
bool tryCalcCameraPoseWithoutInterpole(sead::LookAtCamera*, const IUseCamera*, s32);
void invalidateCameraPoserVerticalAbsorber(CameraTicket*);
void requestStopCameraVerticalAbsorb(IUseCamera*);
void validateSnapShotCameraZoomFovy(CameraTicket*);
void validateSnapShotCameraRoll(CameraTicket*);
bool isSnapShotOrientationRotate90(const IUseCamera*);
bool isSnapShotOrientationRotate270(const IUseCamera*);
bool isValidCameraGyro(const IUseCamera*);
bool isInvalidChangeSubjectiveCamera(const IUseCamera*);
bool isCurrentCameraZooming(const IUseCamera*, s32);
void onCameraRideObj(const LiveActor*);
void offCameraRideObj(const LiveActor*);
bool isExistAnimCameraData(const CameraTicket*, const char*);
bool isEndAnimCamera(const CameraTicket*);
bool isAnimCameraPlaying(const CameraTicket*);
bool isAnimCameraAnimPlaying(const CameraTicket*, const char*);
s32 getAnimCameraStepMax(const CameraTicket*);
s32 getAnimCameraStep(const CameraTicket*);
s32 calcAnimCameraAnimStepMax(const CameraTicket*, const char*);
void setAnimCameraRotateBaseUp(const CameraTicket*);
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