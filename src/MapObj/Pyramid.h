#pragma once

#include "Library/LiveActor/LiveActor.h"

class Pyramid : public al::LiveActor {
public:
    void resetAtCloseAndFly();
    void resetAtOpenAndGround();
    void resetAtCloseAndGround();
    void resetAtOpenAndFly();
};
