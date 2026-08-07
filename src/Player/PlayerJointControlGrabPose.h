#pragma once

#include <basis/seadTypes.h>
#include <container/seadSafeArray.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Joint/JointControllerBase.h"

namespace al {
class LiveActor;
}

class PlayerJointControlGrabPose : public al::JointControllerBase {
public:
    explicit PlayerJointControlGrabPose(const al::LiveActor*);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f*) override;
    void calcArmMtx(sead::Matrix34f*, const sead::SafeArray<sead::Matrix34f, 2>&,
                    const sead::SafeArray<s32, 2>&, const sead::Matrix34f&, f32,
                    const sead::Vector3f&, const sead::Vector3f&, s32,
                    const sead::Vector3f&);
    void update(f32 poseRate, const sead::Vector3f& direction, f32 interpolateRate);
    const char* getCtrlTypeName() const override;

private:
    const al::LiveActor* mPlayer;
    f32 mPoseRate;
    s32 mJointIndexAllRoot;
    s32 mJointIndexShoulderL;
    s32 mJointIndexShoulderR;
    s32 mJointIndexHandL;
    s32 mJointIndexArmL2Sub;
    s32 mJointIndexHandR;
    s32 mJointIndexArmR2Sub;
    sead::Vector3f mDirection;
    sead::Vector3f _5c;
    f32 mInterpolateRate;
    f32 _6c;
    f32 _70;
    f32 _74;
    f32 _78;
    f32 _7c;
    sead::Matrix34f _80;
    sead::Matrix34f _b0;
    sead::Vector3f _e0;
    sead::Vector3f _ec;
    sead::SafeArray<sead::Matrix34f, 2> _f8;
    sead::SafeArray<s32, 2> _158;
    sead::SafeArray<sead::Matrix34f, 2> _160;
    sead::SafeArray<s32, 2> _1c0;
};

static_assert(sizeof(PlayerJointControlGrabPose) == 0x1C8);
