#pragma once

#include <basis/seadTypes.h>

#include "Player/HackerStateBase.h"

namespace al {
class LiveActor;
}

class HackerActionAirMoveControl;
class HackerStateConst;
class IUsePlayerHack;

class HackerStateJump : public HackerStateBase {
public:
    HackerStateJump(al::LiveActor* actor, IUsePlayerHack** playerHack, bool is2D, bool isHack);

    void appear() override;
    void setupTurnControlParam(f32 turnAngleStart, f32 turnAngleFast, f32 turnAngleLimit, f32 turnAngleFastLimit,
                               s32 turnAccelFrame, s32 turnAccelFrameFast, s32 turnBrakeFrame);
    void setupForceJumpExtend(bool value);
    bool isHoldJumpExtend() const;
    void updateJumpPower(f32 jumpPowerMin, f32 jumpPowerMax);
    void exeJumpStart();
    void updateJump();
    void exeJump();
    void doLanding();

private:
    const char* mJumpAnim = "Jump";
    const char* mJumpStartAnim = "JumpStart";
    HackerStateConst* mConst = nullptr;
    HackerActionAirMoveControl* mAirMoveControl = nullptr;
    s32 mExtendFrame = 0;
    f32 mJumpSpeed = 0.0f;
    f32 mJumpPowerMin = 0.0f;
    f32 mJumpPowerMax = 0.0f;
    f32 mInertiaAdd = 0.0f;
    f32 mSpeedMax = 0.0f;
    f32 mAccelFront = 0.0f;
    f32 mAccelBack = 0.0f;
    f32 mAccelTurn = 0.0f;
    f32 mGravityAccel = 0.0f;
};

static_assert(sizeof(HackerStateJump) == 0x70);
