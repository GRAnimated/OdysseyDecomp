#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class HitSensor;
class SensorMsg;
}  // namespace al

class ActorStateReactionBase;
class Shibaken;
class ShibakenStateTurn;

class ShibakenStateBark : public al::NerveStateBase {
public:
    ShibakenStateBark(const char* name, Shibaken* shibaken,
                      ActorStateReactionBase* reaction);

    void kill() override;

    void attackSensor(al::HitSensor* self, al::HitSensor* other);
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self);
    bool tryStart();

    void exeBarkSign();
    void exeBark();
    void exeWait();
    void exeWalk();
    void exeTurn();
    void exeReaction();

private:
    Shibaken* mShibaken = nullptr;
    ShibakenStateTurn* mStateTurn = nullptr;
    ActorStateReactionBase* mStateReaction = nullptr;
    al::HitSensor* mEnemySensor = nullptr;
};

static_assert(sizeof(ShibakenStateBark) == 0x38);
