#pragma once

#include <basis/seadTypes.h>

class NpcStateTurnParam {
public:
    NpcStateTurnParam(f32, f32, f32, f32, bool, bool, s32);

private:
    f32 _0;
    f32 _4;
    f32 _8;
    f32 _c;
    bool _10;
    bool _11;
    s32 _14;
};

static_assert(sizeof(NpcStateTurnParam) == 0x18);
