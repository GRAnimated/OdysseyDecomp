#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}
class TouchTargetKeeper;

class PlayerStateTouchMove : public al::ActorStateBase {
public:
    PlayerStateTouchMove(al::LiveActor* player, const TouchTargetKeeper*);
    void appear() override;
    void exeJump();
    ~PlayerStateTouchMove() override;

};

