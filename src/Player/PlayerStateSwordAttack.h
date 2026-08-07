#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}

class PlayerStateSwordAttack : public al::ActorStateBase {
public:
    PlayerStateSwordAttack(al::LiveActor* player, al::LiveActor* sword);
    ~PlayerStateSwordAttack() override;

    void appear() override;
    void kill() override;
    void exeAttack();

private:
    al::LiveActor* mSword;
    bool mIsPowerGrove;
};

static_assert(sizeof(PlayerStateSwordAttack) == 0x30);
