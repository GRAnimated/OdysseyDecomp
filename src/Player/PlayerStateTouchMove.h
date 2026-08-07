#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}
class TouchTargetKeeper;

class PlayerStateTouchMove : public al::ActorStateBase {
public:
    PlayerStateTouchMove(al::LiveActor* player, const TouchTargetKeeper*);
    ~PlayerStateTouchMove() override;

    void appear() override;
    void exeJump();
};

static_assert(sizeof(PlayerStateTouchMove) == 0x20);
