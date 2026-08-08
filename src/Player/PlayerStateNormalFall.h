#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}
class IUsePlayerCollision;
class PlayerActionAirMoveControl;
class PlayerAnimator;
class PlayerConst;
class PlayerInput;

class PlayerStateNormalFall : public al::ActorStateBase {
public:
    PlayerStateNormalFall(al::LiveActor* player, const PlayerConst* pConst,
                          const IUsePlayerCollision* collision, const PlayerInput* input,
                          PlayerAnimator* animator);
    void appear() override;

    void exeFall();
    ~PlayerStateNormalFall() override;

private:
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    PlayerActionAirMoveControl* mAirMoveControl = nullptr;
};

