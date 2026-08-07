#include "Player/PlayerJointControlSwimPose.h"

#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/MatrixUtil.h"

namespace al {
s32 getJointIndex(const ModelKeeper*, const char*);
}

PlayerJointControlSwimPose::PlayerJointControlSwimPose(const al::LiveActor* player)
    : al::JointControllerBase(16), mJointIndexAllRoot(0), mJointIndexSpine1(0),
      _30(0.0f, 80.0f, 0.0f), mParam{0.0f, 0.0f, 0.0f, 0.0f} {
    const al::ModelKeeper* modelKeeper = player->getModelKeeper();
    mJointIndexAllRoot = al::getJointIndex(modelKeeper, "AllRoot");
    mJointIndexSpine1 = al::getJointIndex(modelKeeper, "Spine1");
    appendJointId(mJointIndexAllRoot);
    appendJointId(mJointIndexSpine1);
}

void PlayerJointControlSwimPose::calcJointCallback(s32 jointIndex, sead::Matrix34f* jointMtx) {
    if (jointIndex == mJointIndexAllRoot) {
        if (!al::isNearZero(mParam._0))
            al::rotateMtxCenterPosXDirDegree(jointMtx, *jointMtx, _30, mParam._0);
        if (!al::isNearZero(mParam._4))
            al::rotateMtxCenterPosYDirDegree(jointMtx, *jointMtx, _30, mParam._4);
        if (!al::isNearZero(mParam._8))
            al::rotateMtxCenterPosZDirDegree(jointMtx, *jointMtx, _30, mParam._8);
    } else if (jointIndex == mJointIndexSpine1 && !al::isNearZero(mParam._4)) {
        al::rotateMtxYDirDegree(jointMtx, *jointMtx, mParam._c);
    }
}

const char* PlayerJointControlSwimPose::getCtrlTypeName() const {
    return "水中姿勢制御";
}
