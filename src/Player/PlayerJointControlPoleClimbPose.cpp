#include "Player/PlayerJointControlPoleClimbPose.h"

#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/MatrixUtil.h"

namespace al {
s32 getJointIndex(const ModelKeeper*, const char*);
}

PlayerJointControlPoleClimbPose::PlayerJointControlPoleClimbPose(const al::LiveActor* player)
    : al::JointControllerBase(16), _28(0.0f, 0.0f, 0.0f), _34(0.0f, 0.0f, 0.0f),
      mJointIndexArmL1(0), mJointIndexArmR1(0), mJointIndexLegL1(0), mJointIndexLegR1(0) {
    const al::ModelKeeper* modelKeeper = player->getModelKeeper();
    mJointIndexArmL1 = al::getJointIndex(modelKeeper, "ArmL1");
    mJointIndexArmR1 = al::getJointIndex(modelKeeper, "ArmR1");
    mJointIndexLegL1 = al::getJointIndex(modelKeeper, "LegL1");
    mJointIndexLegR1 = al::getJointIndex(modelKeeper, "LegR1");
    appendJointId(mJointIndexArmL1);
    appendJointId(mJointIndexArmR1);
    appendJointId(mJointIndexLegL1);
    appendJointId(mJointIndexLegR1);
}

void PlayerJointControlPoleClimbPose::calcJointCallback(s32 jointIndex,
                                                         sead::Matrix34f* jointMtx) {
    if (al::isNearZero(_34) && al::isNearZero(_28))
        return;

    const sead::Vector3f armAngle = _28;
    const sead::Vector3f legAngle = _34;

    if (jointIndex == mJointIndexArmL1)
        calcAngle(jointMtx, armAngle);
    if (jointIndex == mJointIndexArmR1)
        calcAngle(jointMtx, -armAngle);
    if (jointIndex == mJointIndexLegL1)
        calcAngle(jointMtx, legAngle);
    if (jointIndex == mJointIndexLegR1)
        calcAngle(jointMtx, -legAngle);
}

void PlayerJointControlPoleClimbPose::calcAngle(sead::Matrix34f* jointMtx,
                                                 const sead::Vector3f& angle) {
    al::rotateMtxXDirDegree(jointMtx, *jointMtx, angle.x);
    al::rotateMtxYDirDegree(jointMtx, *jointMtx, angle.y);
    al::rotateMtxZDirDegree(jointMtx, *jointMtx, angle.z);
}

void PlayerJointControlPoleClimbPose::update(const sead::Vector3f& armAngle,
                                              const sead::Vector3f& legAngle, f32 rate) {
    al::lerpVec(&_28, _28, armAngle, rate);
    al::lerpVec(&_34, _34, legAngle, rate);
}

const char* PlayerJointControlPoleClimbPose::getCtrlTypeName() const {
    return "ポール登り姿勢制御";
}
