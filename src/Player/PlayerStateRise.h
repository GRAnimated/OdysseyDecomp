#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}
class IUsePlayerCollision;
class PlayerAnimator;
class PlayerActionAirMoveControl;
class PlayerConst;
class PlayerInput;

class PlayerStateRise : public al::ActorStateBase {
public:
    PlayerStateRise(al::LiveActor* player, const PlayerConst* pConst,
                    const IUsePlayerCollision* collision, const PlayerInput* input,
                    PlayerAnimator* animator);
    ~PlayerStateRise() override;

    void appear() override;
    void exeRise();

private:
    const IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    PlayerActionAirMoveControl* mAirMoveControl;
};

static_assert(sizeof(PlayerStateRise) == 0x38);
