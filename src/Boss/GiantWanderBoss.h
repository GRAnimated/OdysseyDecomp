#pragma once

#include "Library/LiveActor/LiveActor.h"

class GiantWanderBoss : public al::LiveActor {
public:
    static bool isOnSwitchAll(GiantWanderBoss*);
    void startDemoWorldEnter();
};
