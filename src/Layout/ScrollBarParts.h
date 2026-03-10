#pragma once

#include <math/seadVector.h>

#include "Library/Layout/LayoutActor.h"
#include "Library/Layout/LayoutInitInfo.h"

class ScrollBarParts : public al::LayoutActor {
public:
    ScrollBarParts(al::LayoutActor* layoutActor, const al::LayoutInitInfo& initInfo,
                   bool isVertical);

    void control();
    void setupDataNum(s32 visibleCount, s32 totalCount);
    void updateIdx(s32 idx);

    void exeAppear();
    void updatePos();
    void exeWait();
    void exeEnd();
    void exeHide();
    void exeInvalid();

private:
    f32 mBarSize = -1.0f;
    f32 _0x134 = -1.0f;
    sead::Vector2f mInitialPos = sead::Vector2f::zero;
    s32 _0x140[7] = {-1, -1, -1, -1, -1, -1, -1};
    bool mIsVertical = false;
};

static_assert(sizeof(ScrollBarParts) == 0x160);
