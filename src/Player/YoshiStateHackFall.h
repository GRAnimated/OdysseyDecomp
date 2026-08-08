#pragma once

#include "Player/HackerStateBase.h"

class IUsePlayerCollision;
class PlayerActionAirMoveControl;
class PlayerAnimator;
class PlayerConst;

class YoshiStateHackFall : public HackerStateBase {
public:
    YoshiStateHackFall(al::LiveActor* actor, IUsePlayerHack** playerHack,
                       const PlayerConst* playerConst, const IUsePlayerCollision* collision,
                       PlayerAnimator* animator);

    void appear() override;
    void exeFall();

private:
    const PlayerConst* mPlayerConst;
    PlayerAnimator* mAnimator;
    PlayerActionAirMoveControl* mAirMoveControl = nullptr;
};

static_assert(sizeof(YoshiStateHackFall) == 0x40);
