#pragma once

#include "Library/LiveActor/LiveActor.h"

class SimpleActionLiveActor : public al::LiveActor {
public:
    SimpleActionLiveActor(const char* name);

    void control() override;

private:
    const char* mPendingAction = nullptr;
};
