#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Joint/JointLookAtController.h"

#include "Player/PlayerJointControlCapThrow.h"
#include "Player/PlayerJointControlGrabDynamics.h"
#include "Player/PlayerJointControlGrabPose.h"
#include "Player/PlayerJointControlGroundPose.h"
#include "Player/PlayerJointControlPartsDynamics.h"
#include "Player/PlayerJointControlPartsFitting.h"
#include "Player/PlayerJointControlPoleClimbPose.h"
#include "Player/PlayerJointControlSwimPose.h"
#include "Player/PlayerJointParamCapThrow.h"
#include "Player/PlayerJointParamCenterDynamics.h"
#include "Player/PlayerJointParamGrab.h"
#include "Player/PlayerJointParamGroundPose.h"
#include "Player/PlayerJointParamHandLegAngle.h"
#include "Player/PlayerJointParamSwim.h"

namespace al {
class HitSensor;
class LiveActor;
}  // namespace al
class PlayerConst;

class PlayerJointControlKeeper {
public:
    PlayerJointControlKeeper(al::LiveActor*, const PlayerConst*, bool, bool, bool);

    void update();
    void updateLookAt();
    void updateSwimPose();
    void updateCenter();
    void updateGroundPose();
    void updateCapThrow();
    void updatePoleClimb();
    void updateGrab();
    void updatePartsDynamics();
    void calcLookAtEyePos(sead::Vector3f* out) const;
    void resetPartsDynamics();
    void updateLookAtTarget(al::HitSensor* sensor);
    void calcGroundPoseUp(sead::Vector3f* out) const;
    f32 getHeadTiltRate() const;
    void setPuppetMode(bool isPuppet) { mIsPuppetMode = isPuppet; }
    void noticeCarryStart() { _96 = true; }
    void setInverseKinematicsRate(f32 groundRate, f32 bodyRate) {
        mGroundPoseRate = groundRate;
        mBodyPoseRate = bodyRate;
    }
    void setTurnTiltRate(f32 rate) { mTurnTiltRate = rate; }
    void setCapDynamicsRate(f32 rate) { mCapDynamicsRate = rate; }
    void setNoseDynamicsRate(f32 rate) { mNoseDynamicsRate = rate; }
    void setGrabCeilPose(bool enabled) { mIsGrabCeilPose = enabled; }
    void setLookAtEnabled(bool enabled) {
        mIsLookAtEnabled = enabled;
        mIsLookAtTargetPosition = false;
    }
    void setLookAtTargetPosition(const sead::Vector3f& target) {
        mIsLookAtEnabled = false;
        mIsLookAtTargetPosition = true;
        mLookAtTargetPosition = target;
    }
    al::HitSensor* getLookAtTargetSensor() const { return mLookAtTargetSensor; }

    PlayerJointParamCapThrow* getCapThrowParam() const { return mCapThrowParam; }
    PlayerJointParamGrab* getGrabParam() const { return mGrabParam; }
    PlayerJointParamHandLegAngle* getHandLegAngleParam() const { return mHandLegAngleParam; }
    PlayerJointParamCenterDynamics* getCenterDynamicsParam() const {
        return mCenterDynamicsParam;
    }
    PlayerJointParamSwim* getSwimParam() const { return mSwimParam; }
    PlayerJointParamGroundPose* getGroundPoseParam() const { return mGroundPoseParam; }
    const sead::Matrix34f* getEffectMtx() const { return &mCapThrowMtx; }

private:
    al::LiveActor* mPlayer;
    PlayerJointControlCapThrow* mCapThrow;
    PlayerJointControlGrabDynamics* mGrabDynamics;
    PlayerJointControlGrabPose* mGrabPose;
    PlayerJointControlGroundPose* mGroundPose;
    al::JointLookAtController* mLookAt;
    PlayerJointControlPartsDynamics* mPartsDynamics;
    PlayerJointControlPartsFitting* mPartsFitting;
    PlayerJointControlPoleClimbPose* mPoleClimbPose;
    PlayerJointControlSwimPose* mSwimPose;
    PlayerJointParamCapThrow* mCapThrowParam;
    PlayerJointParamGrab* mGrabParam;
    PlayerJointParamHandLegAngle* mHandLegAngleParam;
    PlayerJointParamCenterDynamics* mCenterDynamicsParam;
    PlayerJointParamSwim* mSwimParam;
    PlayerJointParamGroundPose* mGroundPoseParam;
    f32 mGroundPoseRate;
    f32 mBodyPoseRate;
    f32 mTurnTiltRate;
    f32 mCapDynamicsRate;
    f32 mNoseDynamicsRate;
    bool mIsLookAtEnabled;
    union {
        bool mIsLookAtTargetPosition;
        bool mIsPuppetMode;
    };
    bool _96;
    bool mIsGrabCeilPose;
    al::HitSensor* mLookAtTargetSensor;
    u8 _a0[0x0C];
    sead::Vector3f mLookAtTargetPosition;
    s32 _b8;
    s32 mJointIndexHead;
    sead::Matrix34f mCapThrowMtx;
};

static_assert(sizeof(PlayerJointControlKeeper) == 0xF0);
