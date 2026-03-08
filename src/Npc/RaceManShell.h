#pragma once

#include "Library/LiveActor/LiveActor.h"

class RaceManShell : public al::LiveActor {
public:
    RaceManShell(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void control() override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) override;
};
