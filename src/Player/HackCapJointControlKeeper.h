#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}

class HackCapJointControlKeeper {
public:
    HackCapJointControlKeeper();

    void initCapJointControl(al::LiveActor* actor);
    void initDisplayCapJointControl(al::LiveActor* actor);
    void updateRotateThrowZ(f32 target, f32 spring);
    void updateRotateStayZ(f32 rate);
    void updateRotateY(f32 rate);

    void setRotateY(f32 value) { mRotateY = value; }
    void resetRotateY() { mRotateY = 0.0f; }

    void reset() {
        mRotateY = 0.0f;
        mRotateYVelocity = 0.0f;
        mRotateZ = 0.0f;
        mRotateZVelocity = 0.0f;
    }

    void resetRotateZ() {
        mRotateZ = 0.0f;
        mRotateZVelocity = 0.0f;
    }

private:
    f32 mRotateY = 0.0f;
    f32 mRotateYVelocity = 0.0f;
    f32 mRotateZ = 0.0f;
    f32 mRotateZVelocity = 0.0f;
};

static_assert(sizeof(HackCapJointControlKeeper) == 0x10);
