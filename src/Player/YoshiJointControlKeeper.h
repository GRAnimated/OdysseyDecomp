#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class JointLookAtController;
class LiveActor;
}

class ActorJointControlGroundPose;
class IUsePlayerCollision;
class YoshiJointControlHeadCorrection;

class YoshiJointControlKeeper {
public:
    YoshiJointControlKeeper(const al::LiveActor* actor, const IUsePlayerCollision* collision);

    void update();
    void updateHeadCorrect();
    void updateLookAt();
    void updateGroundPose();

    void setHeadCorrectionActive(bool active) { mIsHeadCorrectionActive = active; }

    void setLookAtActive(bool active) { mIsLookAtActive = active; }

    void setLookAtPosition(const sead::Vector3f& position) { mLookAtPosition = position; }

    void setGroundPoseRate(f32 frontRate, f32 sideRate) {
        mGroundPoseRateFront = frontRate;
        mGroundPoseRateSide = sideRate;
    }

private:
    const al::LiveActor* mActor;
    const IUsePlayerCollision* mCollision;
    YoshiJointControlHeadCorrection* mHeadCorrection = nullptr;
    al::JointLookAtController* mLookAtController = nullptr;
    ActorJointControlGroundPose* mGroundPose = nullptr;
    bool mIsHeadCorrectionActive = false;
    u8 _29[3]{};
    f32 mHeadCorrectionRate = 0.0f;
    bool mIsLookAtActive = false;
    u8 _31[3]{};
    sead::Vector3f mLookAtPosition = sead::Vector3f::zero;
    f32 mGroundPoseRateFront = 0.0f;
    f32 mGroundPoseRateSide = 0.0f;
    f32 mGroundPoseRateFrontCurrent = 0.0f;
    f32 mGroundPoseRateSideCurrent = 0.0f;
};

static_assert(sizeof(YoshiJointControlKeeper) == 0x50);
