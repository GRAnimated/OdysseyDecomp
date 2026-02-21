#pragma once

#include "Library/LiveActor/LiveActor.h"

namespace al {
class ParabolicPath;
}

class BossRaidWheel : public al::LiveActor {
public:
    BossRaidWheel(const char* name);
    void init(const al::ActorInitInfo& info) override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                    al::HitSensor* self) override;
    void control() override;

    void exeLand();
    void exeRun();

    void shotGround(const sead::Vector3f& pos, const sead::Vector3f& front, f32 ratio);

private:
    al::ParabolicPath* mPath = nullptr;
    sead::Vector3f mFallSignPos = sead::Vector3f::zero;  // position used for FallSign effect
    s32 mRunOffset = 0;     // used to shift the nerve step window in exeRun; set via lerpValue in shotGround
    f32 mRotateSpeed = 5.0f;  // rotation speed (deg/frame); 6.0f when appearing, 5.0f when shot
    f32 mRotateAngle = 0.0f;  // accumulated rotation angle for joint X; wrapped to [0,360) each frame
    f32 mRollAngle = 0.0f;    // side-tilt angle for joint Z; lerped toward -angle*16 in exeRun
};

static_assert(sizeof(BossRaidWheel) == 0x130);
