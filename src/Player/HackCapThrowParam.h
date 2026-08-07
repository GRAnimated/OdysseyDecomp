#pragma once

#include <basis/seadTypes.h>

#include "Library/LiveActor/ActorParamHolder.h"

namespace al {
struct ActorParamF32;
struct ActorParamS32;
class LiveActor;
}  // namespace al

class HackCapThrowParam {
public:
    HackCapThrowParam(al::LiveActor* actor);

    f32 getHackThrowHeight() const { return mHackThrowHeight->value; }
    f32 getMaxVel() const { return mMaxVel->value; }
    f32 getContinuousThrowSpeed() const { return mContinuousThrowSpeed->value; }
    s32 getBreakTime() const { return mBreakTime->value; }
    s32 getBreakTime(bool isWater) const {
        const al::ActorParamS32* breakTime = mWaterBreakTime;
        if (!isWater)
            breakTime = mBreakTime;
        return breakTime->value;
    }
    s32 getBreakTimeNormal(bool isNormal) const {
        const al::ActorParamS32* waterBreakTime = mWaterBreakTime;
        const al::ActorParamS32* breakTime = mBreakTime;
        const al::ActorParamS32* selected = isNormal ? breakTime : waterBreakTime;
        return selected->value;
    }
    f32 getMaxDist() const { return mMaxDist->value; }
    f32 getMaxDist(bool isWater) const { return (isWater ? mWaterDist : mMaxDist)->value; }
    f32 getMaxDistNormal(bool isNormal) const {
        const al::ActorParamF32* waterDist = mWaterDist;
        const al::ActorParamF32* maxDist = mMaxDist;
        const al::ActorParamF32* selected = isNormal ? maxDist : waterDist;
        return selected->value;
    }
    s32 getEndpointStopTime() const { return mEndpointStopTime->value; }
    s32 getMaxEndpointStopTime() const { return mMaxEndpointStopTime->value; }
    f32 getReturnStrength() const { return mReturnStrength->value; }
    f32 getMaxRetSpeed() const { return mMaxRetSpeed->value; }
    f32 getTurnAngleLimit() const { return mTurnAngleLimit->value; }
    f32 getWaterMaxSpeed() const { return mWaterMaxSpeed->value; }
    f32 getWaterDist() const { return mWaterDist->value; }
    s32 getWaterBreakTime() const { return mWaterBreakTime->value; }
    f32 getWaterMaxRetSpeed() const { return mWaterMaxRetSpeed->value; }
    f32 getTornadoDist() const { return mTornadoDist->value; }
    f32 getTornadoMaxDist() const { return mTornadoMaxDist->value; }
    s32 getTornadoReflectTime() const { return mTornadoReflectTime->value; }
    f32 getRollSpeed() const { return mRollSpeed->value; }
    f32 getRollDistTop() const { return mRollDistTop->value; }
    f32 getRollDistBottom() const { return mRollDistBottom->value; }
    f32 getRollDist(bool isTop) const {
        return (isTop ? mRollDistTop : mRollDistBottom)->value;
    }
    s32 getRollBrakeTimeTop() const { return mRollBrakeTimeTop->value; }
    s32 getRollBrakeTimeBottom() const { return mRollBrakeTimeBottom->value; }
    s32 getRollBrakeTime(bool isTop) const {
        return (isTop ? mRollBrakeTimeTop : mRollBrakeTimeBottom)->value;
    }
    f32 getRollGroundGroundedPoseTrack() const { return mRollGroundGroundedPoseTrack->value; }
    f32 getRollGroundAerialPoseTrack() const { return mRollGroundAerialPoseTrack->value; }

private:
    al::ActorParamF32* mHackThrowHeight;
    al::ActorParamF32* mMaxVel;
    al::ActorParamF32* mContinuousThrowSpeed;
    al::ActorParamS32* mBreakTime;
    al::ActorParamF32* mMaxDist;
    al::ActorParamS32* mEndpointStopTime;
    al::ActorParamS32* mMaxEndpointStopTime;
    al::ActorParamF32* mReturnStrength;
    al::ActorParamF32* mMaxRetSpeed;
    al::ActorParamF32* mTurnAngleLimit;
    al::ActorParamF32* mWaterMaxSpeed;
    al::ActorParamF32* mWaterDist;
    al::ActorParamS32* mWaterBreakTime;
    al::ActorParamF32* mWaterMaxRetSpeed;
    al::ActorParamF32* mTornadoDist;
    al::ActorParamF32* mTornadoMaxDist;
    al::ActorParamS32* mTornadoReflectTime;
    al::ActorParamF32* mRollSpeed;
    al::ActorParamF32* mRollDistTop;
    al::ActorParamF32* mRollDistBottom;
    al::ActorParamS32* mRollBrakeTimeTop;
    al::ActorParamS32* mRollBrakeTimeBottom;
    al::ActorParamF32* mRollGroundGroundedPoseTrack;
    al::ActorParamF32* mRollGroundAerialPoseTrack;
};

static_assert(sizeof(HackCapThrowParam) == 0xC0);
