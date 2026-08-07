#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Joint/JointControllerBase.h"

namespace al {
class LiveActor;
}

class PlayerJointControlCapThrow : public al::JointControllerBase {
public:
    PlayerJointControlCapThrow(const al::LiveActor*, sead::Matrix34f*);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f*) override;
    void start(f32 rate, s32 direction, bool isReverse);
    void forceEnd();
    void update();
    const char* getCtrlTypeName() const override;

private:
    const al::LiveActor* mPlayer;
    sead::Matrix34f* mJointMtx;
    f32 _38;
    f32 _3c;
    sead::Vector3f _40;
    s32 _4c;
    s32 _50;
    s32 _54;
    s32 _58;
    f32 _5c;
};

static_assert(sizeof(PlayerJointControlCapThrow) == 0x60);
