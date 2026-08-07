#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
class WaterSurfaceFinder;
}
class IJudge;
class IUseDimension;
class IUsePlayerCollision;
class PlayerAnimator;
class PlayerConst;
class PlayerCounterForceRun;
class PlayerCounterQuickTurnJump;
class PlayerEffect;
class PlayerInput;
class PlayerJudgeStartGroundSpin;
class PlayerJudgeWaterSurfaceRun;
class PlayerStateGroundSpin;
class PlayerStateRun2D;
class PlayerStateRunHakoniwa;
struct PlayerJointParamCenterDynamics;
class PlayerTrigger;

class PlayerStateRunHakoniwa2D3D : public al::ActorStateBase {
public:
    PlayerStateRunHakoniwa2D3D(al::LiveActor* player, const PlayerConst* pConst,
                               const IUseDimension* dimension, const PlayerInput* input,
                               const IUsePlayerCollision* collision,
                               const al::WaterSurfaceFinder* waterSurfaceFinder,
                               const PlayerCounterForceRun* counterForceRun,
                               const PlayerCounterQuickTurnJump* counterQuickTurnJump,
                               PlayerTrigger* trigger, PlayerAnimator* animator,
                               PlayerEffect* effect,
                               PlayerJointParamCenterDynamics* centerDynamics, bool is2D);
    ~PlayerStateRunHakoniwa2D3D() override;

    void appear() override;
    bool update() override;
    bool isEnableLookAt() const;
    bool isGroundSpin() const;
    bool isSpinClockwise() const;
    bool isRunDashFast() const;
    bool isRunWaterSurface() const;
    bool isBrake2D() const;
    bool tryTurnJump(IJudge* judge, sead::Vector3f* direction);
    f32 getTurnTiltRate() const;
    f32 getCenterTiltRate() const;
    f32 getInverseKinematicsRate() const;
    f32 getCapDynamicsRate() const;
    void exeRun3D();
    void exeGroundSpin3D();
    void exeRun2D();

private:
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    const IUseDimension* mDimension;
    const IUsePlayerCollision* mCollision;
    const al::WaterSurfaceFinder* mWaterSurfaceFinder;
    const PlayerCounterForceRun* mCounterForceRun;
    PlayerStateRunHakoniwa* mRun3D;
    PlayerStateGroundSpin* mGroundSpin3D;
    PlayerStateRun2D* mRun2D;
    PlayerJudgeStartGroundSpin* mJudgeStartGroundSpin;
    PlayerJudgeWaterSurfaceRun* mJudgeWaterSurfaceRun;
};

static_assert(sizeof(PlayerStateRunHakoniwa2D3D) == 0x78);
