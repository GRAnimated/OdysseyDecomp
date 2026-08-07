#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}
class IUsePlayerCollision;
class PlayerActionAirMoveControl;
class PlayerAnimator;
class PlayerConst;
class PlayerContinuousJump;
class PlayerInput;

class PlayerStateNormalJump : public al::ActorStateBase {
public:
    PlayerStateNormalJump(al::LiveActor* player, const PlayerConst* pConst,
                          const IUsePlayerCollision* collision, const PlayerInput* input,
                          PlayerAnimator* animator);
    ~PlayerStateNormalJump() override;

    void initContinuousJump(PlayerContinuousJump* continuousJump);
    void appear() override;
    void kill() override;
    void exeJump();
    f32 calcJumpPowerMin();
    f32 calcJumpPowerMax();
    const char* calcJumpAnimName();
    f32 calcJumpGravity();
    bool sendMsgUpperPunch();

private:
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    PlayerContinuousJump* mContinuousJump;
    PlayerActionAirMoveControl* mAirMoveControl;
    f32 mJumpPower = 0.0f;
    bool mIsForceJump = false;
    f32 mForceJumpPower = 0.0f;
};

static_assert(sizeof(PlayerStateNormalJump) == 0x58);
