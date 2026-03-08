#pragma once

#include "Library/LiveActor/LiveActor.h"

class PochiHintPhoto : public al::LiveActor {
public:
    PochiHintPhoto(const char* name);

    void setIsLinkChild();
    void init(const al::ActorInitInfo& initInfo) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) override;
    void exeWait();

private:
    bool mIsLinkChild = false;
};
