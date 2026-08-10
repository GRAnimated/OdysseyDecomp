#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Joint/JointControllerBase.h"

namespace al {
class LiveActor;
}

class PlayerJointControlPoleClimbPose : public al::JointControllerBase {
public:
    PlayerJointControlPoleClimbPose(const al::LiveActor* player);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f* jointMtx) override;
    void calcAngle(sead::Matrix34f* jointMtx, const sead::Vector3f& angle);
    void update(const sead::Vector3f& armAngle, const sead::Vector3f& legAngle, f32 rate);
    const char* getCtrlTypeName() const override;

private:
    sead::Vector3f _28;
    sead::Vector3f _34;
    s32 mJointIndexArmL1;
    s32 mJointIndexArmR1;
    s32 mJointIndexLegL1;
    s32 mJointIndexLegR1;
};

static_assert(sizeof(PlayerJointControlPoleClimbPose) == 0x50);
