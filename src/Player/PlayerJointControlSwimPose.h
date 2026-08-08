#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>

#include "Library/Joint/JointControllerBase.h"

#include "Player/PlayerJointParamSwim.h"

namespace al {
class LiveActor;
}

class PlayerJointControlSwimPose : public al::JointControllerBase {
public:
    PlayerJointControlSwimPose(const al::LiveActor* player);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f* jointMtx) override;
    const char* getCtrlTypeName() const override;

    void setParam(const PlayerJointParamSwim& param) { mParam = param; }

private:
    s32 mJointIndexAllRoot;
    s32 mJointIndexSpine1;
    sead::Vector3f _30;
    PlayerJointParamSwim mParam;
    u8 _4c[4];
};

static_assert(sizeof(PlayerJointControlSwimPose) == 0x50);
