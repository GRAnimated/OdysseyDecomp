#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
class WaterSurfaceFinder;
}
class IUsePlayerCollision;
class PlayerActionAirMoveControl;
class PlayerActionGroundMoveControl;
class PlayerAnimator;
class PlayerConst;
class PlayerCounterForceRun;
class PlayerInput;
struct PlayerJointParamCapThrow;
class PlayerJudgeWaterSurfaceRun;
class PlayerSpinCapAttack;
class PlayerTrigger;

class PlayerStateSpinCap : public al::ActorStateBase {
public:
    PlayerStateSpinCap(al::LiveActor* player, const PlayerConst* pConst,
                       const PlayerInput* input,
                       const PlayerCounterForceRun* counterForceRun,
                       const al::WaterSurfaceFinder* waterSurfaceFinder,
                       const IUsePlayerCollision* collision, PlayerTrigger* trigger,
                       PlayerSpinCapAttack* spinCapAttack, PlayerAnimator* animator,
                       PlayerJointParamCapThrow* capThrowJoint);
    void appear() override;

    void kill() override;
    bool update() override;
    void control() override;
    bool noticeInWater();
    void resetJoint();
    void cancelPoseInterpole();
    bool isEnableCancelHipDrop() const;
    bool isEnableCancelAir() const;
    bool isSpinAttackAir() const;
    bool isEnableCancelGround() const;
    bool isEnableCancelCarry() const;
    bool isEnableReThrowCap() const;
    bool isWaterSurfaceRun() const;
    void exeSpinGround();
    bool isOnGround() const { return mIsOnGround; }
    bool isEnableIK() const { return mIsEnableIK; }
    void updateSpinGroundNerve();
    void exeSpinGroundSeparate();
    void exeSpinAir();
    void updateSpinAirNerve();
    void exeSpinAirSeparate();
    void exeFall();
    ~PlayerStateSpinCap() override;

private:
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    const PlayerCounterForceRun* mCounterForceRun;
    const al::WaterSurfaceFinder* mWaterSurfaceFinder;
    const IUsePlayerCollision* mCollision;
    PlayerTrigger* mTrigger;
    PlayerSpinCapAttack* mSpinCapAttack;
    PlayerAnimator* mAnimator;
    PlayerJointParamCapThrow* mCapThrowJoint;
    PlayerActionAirMoveControl* mAirMoveControl;
    PlayerActionGroundMoveControl* mGroundMoveControl;
    bool mIsOnGround;
    u8 _79[3];
    f32 _7c;
    f32 _80;
    f32 _84;
    bool _88;
    u8 _89[7];
    PlayerJudgeWaterSurfaceRun* mJudgeWaterSurfaceRun;
    bool _98;
    bool _99;
    u8 _9a[2];
    f32 _9c;
    f32 _a0;
    f32 _a4;
    bool mIsEnableIK;
    bool _a9;
    u8 _aa[6];
};

static_assert(sizeof(PlayerStateSpinCap) == 0xB0);
