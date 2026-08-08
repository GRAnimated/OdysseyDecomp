#pragma once

#include "Player/HackerStateBase.h"

class IUsePlayerCollision;
class PlayerActionAirMoveControl;
class PlayerAnimator;
class PlayerConst;
class PlayerTrigger;
class YoshiTongue;

class YoshiStateHackWallJump : public HackerStateBase {
public:
    YoshiStateHackWallJump(al::LiveActor* actor, IUsePlayerHack** playerHack,
                           const PlayerConst* playerConst, const IUsePlayerCollision* collision,
                           const PlayerTrigger* trigger, const YoshiTongue* tongue,
                           PlayerAnimator* animator);

    void appear() override;
    void exeJump();

private:
    const PlayerConst* mPlayerConst;
    const IUsePlayerCollision* mCollision;
    const PlayerTrigger* mTrigger;
    const YoshiTongue* mTongue;
    PlayerAnimator* mAnimator;
    PlayerActionAirMoveControl* mAirMoveControl;
};

static_assert(sizeof(YoshiStateHackWallJump) == 0x58);
