#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

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
    void initContinuousJump(PlayerContinuousJump* continuousJump);
    void initUseTriggerCheck(PlayerTrigger* trigger);
    void initUseCounterCapCatch(const PlayerCounterAfterCapCatch* counterCapCatch);
    void appear() override;
    f32 calcJumpPowerBorderSpeedMin() const;
    f32 calcJumpPowerBorderSpeedMax() const;
    f32 calcJumpPowerMin() const;
    f32 calcJumpPowerMax() const;
    void kill() override;
    void tryCountUpContinuousJump(PlayerContinuousJump* continuousJump);
    void exeJump();
    const char* calcJumpAnimName() const;
    f32 calcJumpGravity() const;

    ~PlayerStateJump2D3D() override;

private:
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    const IPlayerModelChanger* mModelChanger;
    const IUseDimension* mDimension;
    PlayerAnimator* mAnimator;
    PlayerContinuousJump* mContinuousJump;
    PlayerActionAirMoveControl* mAirMoveControl;
    PlayerTrigger* mTrigger;
    f32 mJumpPower;
    bool mIsJumpPowerOverridden;
    u8 _65[3];
    f32 mOverrideJumpPower;
    s32 mContinuousJumpCount;
    bool mIsCanCountContinuousJump;
    u8 _71[7];
    const PlayerCounterAfterCapCatch* mCounterCapCatch;
};

