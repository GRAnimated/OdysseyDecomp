#pragma once

#include "Library/LiveActor/LiveActor.h"

class RaceManThrowCap : public al::LiveActor {
public:
    RaceManThrowCap(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void control() override;

private:
    f32 mYRotation = 0.0f;
};

static_assert(sizeof(RaceManThrowCap) == 0x110);
