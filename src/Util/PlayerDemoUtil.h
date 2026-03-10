#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class HitSensor;
class LiveActor;
}  // namespace al

namespace rs {
void startActionDemoPlayer(const al::LiveActor* actor, const char* actionName);
bool isActionEndDemoPlayer(const al::LiveActor* actor);
void setActionFrameRateDemoPlayer(const al::LiveActor* actor, f32 frameRate);
void setActionFrameDemoPlayer(const al::LiveActor* actor, f32 frame);
void clearDemoAnimInterpolatePlayer(const al::LiveActor* actor);
void hideDemoPlayer(const al::LiveActor* actor);
void showDemoPlayer(const al::LiveActor* actor);
void hideDemoPlayerSilhouette(const al::LiveActor* actor);
void showDemoPlayerSilhouette(const al::LiveActor* actor);
void addDemoLockOnCap(const al::LiveActor* actor);
void forcePutOnDemoCap(const al::LiveActor* actor);
void forcePutOffMarioHeadCap(const al::LiveActor* actor);
void hideDemoCap(const al::LiveActor* actor);
void showDemoCap(const al::LiveActor* actor);
void hideDemoCapSilhouette(const al::LiveActor* actor);
void showDemoCapSilhouette(const al::LiveActor* actor);
void killAllEffectPlayerAndCap(const al::LiveActor* actor);
const sead::Vector3f& getDemoPlayerTrans(const al::LiveActor* actor);
void setDemoPlayerQuat(const al::LiveActor* actor, const sead::Quatf& quat);
const sead::Quatf& getDemoPlayerQuat(const al::LiveActor* actor);
void replaceDemoPlayer(const al::LiveActor* actor, const sead::Vector3f& trans,
                       const sead::Quatf& quat);
void validateIK(const al::LiveActor* actor);
void invalidateIK(const al::LiveActor* actor);
void validateWatchTarget(const al::LiveActor* actor, const sead::Vector3f& target);
void invalidateWatchTarget(const al::LiveActor* actor);
void hideDemoPlayerAndStartDemoResetAction(const al::LiveActor* actor);
void calcDemoMarioJointPosAllRoot(sead::Vector3f* outTrans, const al::LiveActor* actor);
void invalidateMarioDitherFrame(const al::LiveActor* actor, s32 frames);
void startMarioCapEyeAction(const al::LiveActor* actor, const char* actionName);
void killMarioCapEye(const al::LiveActor* actor);
void startMarioRightHandAction(const al::LiveActor* actor, const char* actionName);
f32 getMarioActionFrameMax(const al::LiveActor* actor);
void hideMarioGroundDepthShadow(const al::LiveActor* actor);
void showMarioGroundDepthShadow(const al::LiveActor* actor);
void setMarioGroundDepthShadowMapLength(const al::LiveActor* actor, f32 length);
void changeMarioDepthShadowMapSizeHight(const al::LiveActor* actor);
void changeMarioDepthShadowMapSizeNormal(const al::LiveActor* actor);
void setMarioDirectionalShadowMaskTypeNone(const al::LiveActor* actor);
void setMarioDirectionalShadowMaskTypeSelf(const al::LiveActor* actor);
void resetMarioDynamics(const al::LiveActor* actor);
void keepMarioCapVisibilityEndDemo(const al::LiveActor* actor);
void clearMarioFootPrint(const al::LiveActor* actor);
void tryRegisterSphinxQuizRouteKillSensorAfterPlacement(al::HitSensor* sensor);
}  // namespace rs

namespace PlayerDemoFunction {
void startCapCheckpointWarpMode(const al::LiveActor* actor);
void endMarioShadowMainShine(const al::LiveActor* actor);
void prepareSphinxQuizRouteKill(const al::LiveActor* actor);
void clearMarioStain(al::LiveActor* actor);
}  // namespace PlayerDemoFunction
