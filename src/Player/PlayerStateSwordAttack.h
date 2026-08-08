#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}

class PlayerStateSwordAttack : public al::ActorStateBase {
public:
    PlayerStateSwordAttack(al::LiveActor* player, al::LiveActor* sword);
    void appear() override;

    void kill() override;
    void exeAttack();
    ~PlayerStateSwordAttack() override;

private:
    al::LiveActor* mSword;
    bool mIsPowerGlove = false;
};
