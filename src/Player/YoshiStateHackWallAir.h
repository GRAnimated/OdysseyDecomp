#pragma once

#include <math/seadVector.h>

#include "Player/HackerStateBase.h"

namespace al {
class CollisionParts;
}

class IUsePlayerCollision;
class PlayerAnimator;
class PlayerWallActionHistory;
class YoshiStateHackWallCling;
class YoshiStateHackWallJump;

class YoshiStateHackWallAir : public HackerStateBase {
public:
    void appear() override;
    bool isCling() const;
    bool isAir() const;
    void setupCling(const al::CollisionParts* collisionParts, const sead::Vector3f& position,
                    const sead::Vector3f& normal);

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
