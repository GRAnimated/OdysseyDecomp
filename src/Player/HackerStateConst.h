#pragma once

#include <basis/seadTypes.h>

class HackerStateConst {
public:
    HackerStateConst();

    f32 _0;
    f32 _4;
    f32 _8;
    f32 _c;
    f32 _10;
    f32 _14;
    f32 _18;
    f32 _1c;
    f32 _20;
    s32 _24;
    s32 _28;
    s32 _2c;
    union {
        f32 _30;
        s32 _30bits;
    };
    f32 _34;
    f32 _38;
    f32 _3c;
    f32 _40;
    f32 _44;
    f32 _48;
    f32 _4c;
    f32 _50;
    f32 _54;
    s32 _58;
    f32 _5c;
};

static_assert(sizeof(HackerStateConst) == 0x60);
