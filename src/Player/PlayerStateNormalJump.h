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
    void initContinuousJump(PlayerContinuousJump* continuousJump);
    void appear() override;
    f32 calcJumpPowerMin();
    f32 calcJumpPowerMax();
    void kill() override;
    void exeJump();
    const char* calcJumpAnimName();
    f32 calcJumpGravity();
    bool sendMsgUpperPunch();
    ~PlayerStateNormalJump() override;

private:
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    PlayerContinuousJump* mContinuousJump = nullptr;
    PlayerActionAirMoveControl* mAirMoveControl = nullptr;
    f32 mJumpPower = 0.0f;
    bool mIsForceJump = false;
    f32 mForceJumpPower = 0.0f;
};

