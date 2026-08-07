#pragma once

#include <basis/seadTypes.h>

namespace al {
class JointSpringController;
class LiveActor;
}
class PlayerConst;

class PlayerJointControlGrabDynamics {
public:
    PlayerJointControlGrabDynamics(const al::LiveActor*, const PlayerConst*);

    void update(f32 controlRate, f32 convergenceRate);

private:
    const PlayerConst* mConst;
    al::JointSpringController* mHipSpring;
    f32 mControlRate;
    u32 mPadding14;
};

static_assert(sizeof(PlayerJointControlGrabDynamics) == 0x18);
