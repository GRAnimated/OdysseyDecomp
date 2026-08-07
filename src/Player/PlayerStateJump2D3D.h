#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}
class IPlayerModelChanger;
class IUseDimension;
class IUsePlayerCollision;
class PlayerActionAirMoveControl;
class PlayerAnimator;
class PlayerConst;
class PlayerContinuousJump;
class PlayerCounterAfterCapCatch;
class PlayerInput;
class PlayerTrigger;

class PlayerStateJump2D3D : public al::ActorStateBase {
public:
    PlayerStateJump2D3D(al::LiveActor* player, const PlayerConst* pConst,
                        const IUsePlayerCollision* collision,
                        const IPlayerModelChanger* modelChanger, const IUseDimension* dimension,
                        const PlayerInput* input, PlayerAnimator* animator);
    ~PlayerStateJump2D3D() override;

    void initContinuousJump(PlayerContinuousJump* continuousJump);
    void initUseTriggerCheck(PlayerTrigger* trigger);
    void initUseCounterCapCatch(const PlayerCounterAfterCapCatch* counter);
    void appear() override;
    void kill() override;
    f32 calcJumpPowerBorderSpeedMin() const;
    f32 calcJumpPowerBorderSpeedMax() const;
    f32 calcJumpPowerMin() const;
    f32 calcJumpPowerMax() const;
    void tryCountUpContinuousJump(PlayerContinuousJump* continuousJump);
    void exeJump();
    const char* calcJumpAnimName() const;
    f32 calcJumpGravity() const;

private:
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    const IPlayerModelChanger* mModelChanger;
    const IUseDimension* mDimension;
    PlayerAnimator* mAnimator;
    PlayerContinuousJump* mContinuousJump = nullptr;
    PlayerActionAirMoveControl* mAirMoveControl = nullptr;
    PlayerTrigger* mTrigger = nullptr;
    f32 mJumpPower = 0.0f;
    bool mIsForceJump = false;
    f32 mForceJumpPower = 0.0f;
    s32 mContinuousJumpCount = 0;
    bool mIsEnableContinuousJump = false;
    const PlayerCounterAfterCapCatch* mCounterAfterCapCatch = nullptr;
};

static_assert(sizeof(PlayerStateJump2D3D) == 0x80);
