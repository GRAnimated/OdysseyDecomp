#pragma once

#include <math/seadVector.h>

#include "Player/HackerStateBase.h"

namespace al {
class CollisionParts;
}

class IUsePlayerCollision;
class PlayerAnimator;
class PlayerConst;
class PlayerTrigger;
class PlayerWallActionHistory;
class YoshiTongue;
class YoshiStateHackWallCling;
class YoshiStateHackWallJump;

class YoshiStateHackWallAir : public HackerStateBase {
public:
    YoshiStateHackWallAir(al::LiveActor* actor, IUsePlayerHack** playerHack, const PlayerConst* playerConst,
                          const IUsePlayerCollision* collision, const PlayerTrigger* trigger,
                          const YoshiTongue* tongue, PlayerWallActionHistory* wallActionHistory,
                          PlayerAnimator* animator);

    void appear() override;
    bool isCling() const;
    bool isAir() const;
    void setupCling(const al::CollisionParts* collisionParts, const sead::Vector3f& position,
                    const sead::Vector3f& normal);
    void startShrink();

    void exeCling();
    void exeJump();

private:
    const IUsePlayerCollision* mCollision;
    PlayerWallActionHistory* mWallActionHistory;
    PlayerAnimator* mAnimator;
    YoshiStateHackWallCling* mStateCling;
    YoshiStateHackWallJump* mStateJump;
};

static_assert(sizeof(YoshiStateHackWallAir) == 0x50);
