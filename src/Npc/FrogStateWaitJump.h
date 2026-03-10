#pragma once

#include "Library/Nerve/NerveStateBase.h"

class Frog;

class FrogStateWaitJump : public al::NerveStateBase {
public:
    FrogStateWaitJump(Frog* frog);

    void appear() override;
    void kill() override;

    void endFall();

    void exeWait();
    void exeTurn();
    void exeJumpStart();
    void exeJump();
    void exeFall();
    void exeLand();

private:
    Frog* mActor;
    s32 mWaitFrames;
    f32 mTurnAngle;
    f32 mJumpSpeed;
};

static_assert(sizeof(FrogStateWaitJump) == 0x30);
