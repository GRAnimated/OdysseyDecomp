#pragma once

#include <basis/seadTypes.h>

struct PlayerJointParamCapThrow {
    bool isStart;
    bool isEnd;
    u8 padding_02[2];
    s32 direction;
    f32 rate;
};

static_assert(sizeof(PlayerJointParamCapThrow) == 0xC);
