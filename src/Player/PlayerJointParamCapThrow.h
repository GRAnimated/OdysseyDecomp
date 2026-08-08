#pragma once

#include <basis/seadTypes.h>

struct PlayerJointParamCapThrow {
    bool isStart;
    bool isEnd;
    u8 padding_02;
    bool isReverse;
    s32 direction;
    f32 rate;
};

