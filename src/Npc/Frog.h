#pragma once

#include <math/seadQuat.h>

#include "Library/Joint/JointQuatController.h"
#include "Library/LiveActor/LiveActor.h"

class Frog : public al::LiveActor {
public:
    Frog(const char* name);

    void onDynamics();
    void offDynamics();
    void updateVelocity();

    u8 _108[0x8C];
    sead::Quatf mJointQuat;
    u8 _1a4[4];
    al::JointQuatController* mJointController;
};
