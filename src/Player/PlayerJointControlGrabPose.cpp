#include "Player/PlayerJointControlGrabPose.h"

#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"

namespace al {
bool isExistJoint(const ModelKeeper* modelKeeper, const char* jointName);
s32 getJointIndex(const ModelKeeper* modelKeeper, const char* jointName);
s32 getParentJointIndex(const ModelKeeper* modelKeeper, s32 jointIndex);
}  // namespace al

PlayerJointControlGrabPose::PlayerJointControlGrabPose(const al::LiveActor* player)
    : al::JointControllerBase(16), mPlayer(player), mPoseRate(0.0f), mJointIndexAllRoot(0),
      mJointIndexShoulderL(0), mJointIndexShoulderR(0), mJointIndexHandL(0),
      mJointIndexArmL2Sub(0), mJointIndexHandR(0), mJointIndexArmR2Sub(0),
      mDirection(sead::Vector3f::ey), _5c(0.0f, 0.0f, 0.0f), mInterpolateRate(0.0f), _6c(0.0f),
      _70(0.0f), _74(0.0f), _78(0.0f), _7c(0.0f), _80(sead::Matrix34f::ident),
      _b0(sead::Matrix34f::ident), _e0(0.0f, 0.0f, 0.0f), _ec(0.0f, 0.0f, 0.0f) {
    const al::ModelKeeper* modelKeeper = player->getModelKeeper();
    mJointIndexAllRoot = al::getJointIndex(modelKeeper, "AllRoot");
    mJointIndexShoulderL = al::getJointIndex(modelKeeper, "ShoulderL");
    mJointIndexShoulderR = al::getJointIndex(modelKeeper, "ShoulderR");
    mJointIndexHandL = al::getJointIndex(modelKeeper, "HandL");
    mJointIndexArmL2Sub = al::isExistJoint(modelKeeper, "ArmL2Sub") ?
                             al::getJointIndex(modelKeeper, "ArmL2Sub") :
                             -1;
    mJointIndexHandR = al::getJointIndex(modelKeeper, "HandR");
    mJointIndexArmR2Sub = al::isExistJoint(modelKeeper, "ArmR2Sub") ?
                             al::getJointIndex(modelKeeper, "ArmR2Sub") :
                             -1;

    _158[1] = al::getParentJointIndex(modelKeeper, mJointIndexHandL);
    _158[0] = al::getParentJointIndex(modelKeeper, _158[1]);
    _1c0[1] = al::getParentJointIndex(modelKeeper, mJointIndexHandR);
    _1c0[0] = al::getParentJointIndex(modelKeeper, _1c0[1]);

    appendJointId(mJointIndexAllRoot);
    appendJointId(mJointIndexShoulderL);
    appendJointId(mJointIndexShoulderR);
    appendJointId(mJointIndexHandL);
    if (mJointIndexArmL2Sub != -1)
        appendJointId(mJointIndexArmL2Sub);
    appendJointId(mJointIndexHandR);
    if (mJointIndexArmR2Sub != -1)
        appendJointId(mJointIndexArmR2Sub);
}

void PlayerJointControlGrabPose::update(f32 poseRate, const sead::Vector3f& direction,
                                        f32 interpolateRate) {
    mPoseRate = poseRate;
    mDirection = direction;
    mInterpolateRate = interpolateRate;

    if (al::isNearZero(poseRate, 0.001f))
        return;

    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    al::calcFrontDir(&front, mPlayer);
    sead::Vector3f up = {0.0f, 0.0f, 0.0f};
    al::calcUpDir(&up, mPlayer);

    _6c = al::calcAngleOnPlaneDegree(up, direction, front);
    const f32 angleRate = -_6c * mPoseRate;
    _70 = sead::Mathf::clampMin(angleRate, 0.0f);
    _74 = sead::Mathf::clampMax(angleRate, 0.0f);
    _78 = sead::Mathf::sin(sead::Mathf::deg2rad(_6c)) * -0.0f * mPoseRate;
    _7c = sead::Mathf::sin(sead::Mathf::deg2rad(sead::Mathf::abs(_6c))) * 30.0f * mPoseRate;
    _5c.setCross(front, direction);
    al::tryNormalizeOrZero(&_5c);
}

const char* PlayerJointControlGrabPose::getCtrlTypeName() const {
    return "壁つかまり姿勢制御";
}
