#pragma once

#include "Library/LiveActor/LiveActor.h"

class GoalMark : public al::LiveActor {
public:
    GoalMark(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void activate();

private:
    void* _108 = nullptr;
    bool _110 = false;
    f32 _114 = -1.0f;
    void* _118 = nullptr;
    f32 _120 = 0.01f;
    s32 _124 = 0;
    s32 _128 = 0;
    s32 _12c = 0;
    u16 _130 = 0;
};

static_assert(sizeof(GoalMark) == 0x138);
