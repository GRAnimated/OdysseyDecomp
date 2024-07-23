#pragma once

#include "Library/LiveActor/LiveActor.h"

class RaceAudienceNpc : public al::LiveActor {
public:
    RaceAudienceNpc(const char* actorName);

    virtual void init(const al::ActorInitInfo& info) override;

    void exeWait();
    void exeDance();
    void exeDanceRandom();
    void exeJump();

private:
    s32 field_108 = 0;
    s32 field_110 = 0;
};
