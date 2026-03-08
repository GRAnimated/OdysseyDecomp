#pragma once

#include "Library/Nerve/NerveStateBase.h"

class FrogStateWaitWander : public al::ActorStateBase {
public:
    FrogStateWaitWander(al::LiveActor* actor);

    void appear() override;

    void exeWait();
    void exeWalk();
    void exeFall();

private:
    s32 mWaitFrames = 180;
    s32 mWalkCounter = 0;
};

static_assert(sizeof(FrogStateWaitWander) == 0x28);
