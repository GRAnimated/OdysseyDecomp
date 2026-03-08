#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class HitSensor;
class SensorMsg;
}  // namespace al

class ActorStateReactionBase;
class Shibaken;

class ShibakenStateSit : public al::HostStateBase<Shibaken> {
public:
    ShibakenStateSit(const char* name, Shibaken* shibaken, ActorStateReactionBase* reaction,
                     bool alwaysSit);

    void appear() override;
    void tryStartSitEnd();
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self);

    void exeSitStart();
    void exeSit();
    void exeSitBow();
    void exeSitEnd();
    void exeReaction();
    void exeReactionAfter();

private:
    ActorStateReactionBase* mReaction = nullptr;
    bool mAlwaysSit = false;
};

static_assert(sizeof(ShibakenStateSit) == 0x30);
