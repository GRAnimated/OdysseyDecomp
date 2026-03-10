#pragma once

#include "Library/LiveActor/LiveActor.h"

namespace al {
class EventFlowExecutor;
}

class MoveHomeNpc : public al::LiveActor {
public:
    MoveHomeNpc(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* self, al::HitSensor* other) override;

    void startTalk();
    void skipTalk();
    void exeTalk();
    void exeTalkEnd();

private:
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
};

static_assert(sizeof(MoveHomeNpc) == 0x110);
