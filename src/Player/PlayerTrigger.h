#pragma once

#include <basis/seadTypes.h>
#include <prim/seadBitFlag.h>

class PlayerTrigger {
public:
    enum ECollisionTrigger : u32 {
        // used when hip-drop lands in a puddle
        ECollisionTrigger_val0 = 0,
        // used in PlayerStateHipDrop::isLandTrigger
        ECollisionTrigger_val1 = 1,
        ECollisionTrigger_val2 = 2,
        ECollisionTrigger_val3 = 3,
        ECollisionTrigger_val4 = 4,
        ECollisionTrigger_val5 = 5,
        // used in PlayerStateDamageLife::exeDead
        ECollisionTrigger_val6 = 6,
        ECollisionTrigger_val7 = 7,
        ECollisionTrigger_val8 = 8,
        // used in PlayerJudgeWallHitDown::judge
        ECollisionTrigger_val9 = 9,
        ECollisionTrigger_val10 = 10,
        // used in PlayerTrigger::isOnYoshiHackEnd
        ECollisionTrigger_val1024 = 1024,
    };

    enum EAttackSensorTrigger : u32 {
        // used in PlayerCounterAfterCapCatch::isCapCatch
        EAttackSensorTrigger_val0 = 0,
        EAttackSensorTrigger_val1 = 1,
        EAttackSensorTrigger_val2 = 2,
        EAttackSensorTrigger_val3 = 3,
        EAttackSensorTrigger_val4 = 4,
    };

    enum EActionTrigger : u32 {
        // used when wall-air enters a spin-cap attack
        EActionTrigger_val0 = 0,
        EActionTrigger_val1 = 1,
        EActionTrigger_val2 = 2,
        // used in PlayerStateHipDrop::exeStart
        EActionTrigger_val3 = 3,
        EActionTrigger_val4 = 4,
        EActionTrigger_IceWaterDamage = EActionTrigger_val4,
        // used when damage transitions into swimming
        EActionTrigger_val5 = 5,
        // used in PlayerStateDamageLife::appear
        EActionTrigger_val6 = 6,
        EActionTrigger_val7 = 7,
        EActionTrigger_NoOxygenDamage = EActionTrigger_val7,
        // used when transitioning out of a completed distance jump
        EActionTrigger_val8 = 8,
        EActionTrigger_Rolling = EActionTrigger_val8,
        // used when entering swim from cap-catch pop
        EActionTrigger_val9 = 9,
        EActionTrigger_DiveInWater = EActionTrigger_val9,
        // used when swim wall-hit-down enters damage
        EActionTrigger_val10 = 10,
        EActionTrigger_WallHitDown = EActionTrigger_val10,
        // used in PlayerJudgeForceLand::judge
        EActionTrigger_val11 = 11,
        // used in PlayerStateSquat::appear
        EActionTrigger_val12 = 12,
        // used while fire damage forbids squat cancellation
        EActionTrigger_val13 = 13,
        // used when head sliding enters water without a dive animation
        EActionTrigger_val15 = 15,
        // used when head sliding ends on ground
        EActionTrigger_val16 = 16,
        // used when cancelling hip-drop into rolling
        EActionTrigger_val17 = 17,
        // used when cancelling hip-drop into a spin-cap attack
        EActionTrigger_val18 = 18,
        // used when hip-drop enters water
        EActionTrigger_val19 = 19,
        // used when entering water from slope movement
        EActionTrigger_val29 = 29,
        EActionTrigger_StartSwim = EActionTrigger_val29,
        // used when returning to swim after damage
        EActionTrigger_val20 = 20,
        EActionTrigger_DamageSwim = EActionTrigger_val20,
        // used when rolling ends standing up
        EActionTrigger_val25 = 25,
        // used when cancelling swim damage by paddle input
        EActionTrigger_val26 = 26,
        EActionTrigger_DamageSwimCancel = EActionTrigger_val26,
        // used in PlayerJudgeWallCatch::update, PlayerJudgeWallKeep::update
        EActionTrigger_val14 = 14,
        EActionTrigger_val21 = 21,
        EActionTrigger_val22 = 22,
        EActionTrigger_val23 = 23,
        EActionTrigger_val24 = 24,
        EActionTrigger_val27 = 27,
        EActionTrigger_val28 = 28,
        EActionTrigger_val30 = 30,
        // used once fire damage permits collision snap cancellation
        EActionTrigger_val31 = 31,
        // used when wall catch finishes by falling
        EActionTrigger_val32 = 32,
        // used when rethrowing during water-surface spin-cap movement
        EActionTrigger_val33 = 33,
        // used in PlayerCounterQuickTurnJump::isEnableTurnJump
        EActionTrigger_QuickTurn = 34,
    };

    enum EReceiveSensorTrigger : u32 {
        EReceiveSensorTrigger_val0 = 0,
        EReceiveSensorTrigger_val1 = 1,
        EReceiveSensorTrigger_val2 = 2,
        EReceiveSensorTrigger_val3 = 3,
    };

    enum EPreMovementTrigger : u32 {
        EPreMovementTrigger_val0 = 0,
        EPreMovementTrigger_val1 = 1,
        // used in PlayerStateDamageLife::appear
        EPreMovementTrigger_val2 = 2,
        EPreMovementTrigger_val3 = 3,
        EPreMovementTrigger_val4 = 4,
    };

    enum EDemoEndTrigger : u32 {
        EDemoEndTrigger_val0 = 0,
    };

    enum EMaterialChangeTrigger : u32 {
        EMaterialChangeTrigger_val0 = 0,
    };

    PlayerTrigger();
    void set(ECollisionTrigger flag);
    void set(EAttackSensorTrigger flag);
    void set(EActionTrigger flag);
    void set(EReceiveSensorTrigger flag);
    void set(EPreMovementTrigger flag);
    void set(EDemoEndTrigger flag);
    void set(EMaterialChangeTrigger flag);
    void setRecMaterialTrigger(const char* materialTrigger);
    void clearCollisionTrigger();
    void clearAttackSensorTrigger();
    void clearActionTrigger();
    void clearReceiveSensorTrigger();
    void clearPreMovementTrigger();
    void clearDemoEndTrigger();
    void clearMaterialChangeTrigger();
    bool isOn(ECollisionTrigger flag) const;
    bool isOn(EAttackSensorTrigger flag) const;
    bool isOn(EActionTrigger flag) const;
    bool isOn(EReceiveSensorTrigger flag) const;
    bool isOn(EPreMovementTrigger flag) const;
    bool isOn(EDemoEndTrigger flag) const;
    bool isOn(EMaterialChangeTrigger flag) const;
    bool isOnUpperPunchHit() const;
    bool isOnUpperPunchHitToss() const;
    bool isOnAnyDamage() const;
    bool isOnDamageFire() const;
    bool isOnEndHackWithDamage() const;
    bool isOnNoDamageDown() const;
    bool isOnSpinMoveCapThrow() const;
    bool isOnHipDropCancelThrow() const;
    bool isOnYoshiHackEnd() const;
    bool isOnCollisionExpandCheck() const;
    bool tryGetRecMaterialCode(const char** dest) const;

private:
    sead::BitFlag32 mCollisionTrigger = 0;
    sead::BitFlag32 mAttackSensorTrigger;
    sead::BitFlag64 mActionTrigger = 0;
    sead::BitFlag32 mReceiveSensorTrigger = 0;
    sead::BitFlag32 mPreMovementTrigger = 0;
    sead::BitFlag32 mDemoEndTrigger = 0;
    sead::BitFlag32 mMaterialChangeTrigger = 0;
    const char* mRecMaterialTrigger = nullptr;
};

static_assert(sizeof(PlayerTrigger) == 0x28);
