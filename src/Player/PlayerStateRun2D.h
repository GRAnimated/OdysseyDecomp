#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}  // namespace al
class IUsePlayerCollision;
class PlayerAnimator;
class PlayerConst;
class PlayerInput;

class PlayerStateRun2D : public al::ActorStateBase {
public:
    PlayerStateRun2D(al::LiveActor* player, const PlayerConst* pConst, const PlayerInput* input,
                     const IUsePlayerCollision* collision, PlayerAnimator* animator);

private:
    u8 _20[0x28];
};

static_assert(sizeof(PlayerStateRun2D) == 0x48);
