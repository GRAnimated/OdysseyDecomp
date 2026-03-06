#pragma once

#include "Library/LiveActor/LiveActor.h"

class TalkNpcFreeze : public al::LiveActor {
public:
    TalkNpcFreeze(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void makeActorAlive() override;
    void movement() override;
    void calcAnim() override;
};

static_assert(sizeof(TalkNpcFreeze) == 0x108);
