#pragma once

#include <basis/seadTypes.h>

class HackerStateConst {
public:
    HackerStateConst();

    f32 _0 = 35.0f;
    f32 _4 = 0.15f;
    f32 _8 = 0.5f;
    f32 _c = 1.0f;
    f32 _10 = 0.3f;
    f32 _14 = 1.0f;
    f32 _18 = 135.0f;
    f32 _1c = 0.0f;
    f32 _20 = 6.0f;
    s32 _24 = 20;
    s32 _28 = 1;
    s32 _2c = 10;
    union {
        f32 _30 = 24.0f;
        s32 _30bits;
    };
    f32 _34 = 11.0f;
    f32 _38 = 30.0f;
    f32 _3c = 17.0f;
    f32 _40 = 19.5f;
    f32 _44 = 3.0f;
    f32 _48 = 14.0f;
    f32 _4c = 3.0f;
    f32 _50 = 1.5f;
    f32 _54 = 0.7f;
    s32 _58 = 10;
    f32 _5c = 0.25f;
};

static_assert(sizeof(HackerStateConst) == 0x60);
