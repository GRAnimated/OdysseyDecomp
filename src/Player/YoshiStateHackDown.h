#pragma once

#include <math/seadVector.h>

#include "Player/HackerStateBase.h"

class IUsePlayerCollision;
class PlayerActionAirMoveControl;
class PlayerAnimator;
class PlayerConst;

class YoshiStateHackDown : public HackerStateBase {
public:
    YoshiStateHackDown(al::LiveActor* actor, IUsePlayerHack** playerHack, const PlayerConst* playerConst,
                       const IUsePlayerCollision* collision, PlayerAnimator* animator);
    void appear() override;

    bool isLand() const;

    bool isEnableCancel() const;
    void exeDown();

    void exeLand();
    ~YoshiStateHackDown() override;

private:
    const PlayerConst* mPlayerConst;
    const IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    PlayerActionAirMoveControl* mAirMoveControl;
    sead::Vector3f mGroundNormal;
};

static_assert(sizeof(YoshiStateHackDown) == 0x58);
