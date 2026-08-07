#pragma once

#include "Player/HackerStateBase.h"

class IUsePlayerCollision;
class PlayerActionAirMoveControl;
class PlayerAnimator;
class PlayerConst;
class PlayerTrigger;

class YoshiStateHackJumpFlap : public HackerStateBase {
public:
    void appear() override;
    void kill() override;

    void exeRise();
    void exeFall();

private:
    const PlayerConst* mPlayerConst;
    const IUsePlayerCollision* mCollision;
    const PlayerTrigger* mTrigger;
    al::LiveActor* mHackActor;
    PlayerAnimator* mAnimator;
    PlayerActionAirMoveControl* mAirMoveControl;
    s32 _58;
};

static_assert(sizeof(YoshiStateHackJumpFlap) == 0x60);
