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
    HackerStateJump(al::LiveActor*, IUsePlayerHack**, bool, bool);

    void appear() override;
    void setupTurnControlParam(f32, f32, f32, f32, s32, s32, s32);
    void setupForceJumpExtend(bool);
    bool isHoldJumpExtend() const;
    void updateJumpPower(f32, f32);
    void exeJumpStart();
    void updateJump();
    void exeJump();
    void doLanding();

private:
    const char* mJumpAnim;
    const char* mJumpStartAnim;
    HackerStateConst* mConst;
    HackerActionAirMoveControl* mAirMoveControl;
    union {
        s32 _48;
        f32 _48f;
    };
    f32 mJumpSpeed;
    f32 mJumpPowerMin;
    f32 mJumpPowerMax;
    f32 _58;
    f32 _5c;
    f32 _60;
    f32 _64;
    f32 _68;
    f32 _6c;
};

static_assert(sizeof(HackerStateJump) == 0x70);
