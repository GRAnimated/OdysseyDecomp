#pragma once

#include "Player/HackerStateBase.h"

class PlayerActionAirMoveControl;
class PlayerAnimator;
class PlayerConst;

class YoshiStateHackFall : public HackerStateBase {
public:
    void appear() override;
    void exeFall();

private:
    const PlayerConst* mPlayerConst;
    PlayerAnimator* mAnimator;
    PlayerActionAirMoveControl* mAirMoveControl;
};

static_assert(sizeof(YoshiStateHackFall) == 0x40);
