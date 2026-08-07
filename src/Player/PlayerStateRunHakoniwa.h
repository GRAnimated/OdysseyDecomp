#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

class IJudge;
class IUsePlayerCollision;
class PlayerAnimator;
class PlayerConst;
class PlayerCounterForceRun;
class PlayerCounterQuickTurnJump;
class PlayerEffect;
class PlayerInput;
struct PlayerJointParamCenterDynamics;
class PlayerTrigger;

class PlayerStateRunHakoniwa : public al::ActorStateBase {
public:
    PlayerStateRunHakoniwa(al::LiveActor* player, const PlayerConst* pConst,
                           const PlayerInput* input, const IUsePlayerCollision* collision,
                           const PlayerCounterForceRun* counterForceRun,
                           const PlayerCounterQuickTurnJump* counterQuickTurnJump,
                           PlayerTrigger* trigger, PlayerAnimator* animator, PlayerEffect* effect,
                           PlayerJointParamCenterDynamics* centerDynamics,
                           const IJudge* judgeWaterSurfaceRun, bool isMoon);
    bool isEnableLookAt() const;
    bool isRunDashFast() const;
    bool tryTurnJump(sead::Vector3f* direction);
    f32 getCenterTiltRateMax() const;
    f32 getCapDynamicsRate() const;

    f32 getTurnTiltRate() const { return mTurnTiltRate; }
    f32 getInverseKinematicsRate() const { return mInverseKinematicsRate; }

private:
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    const IUsePlayerCollision* mCollision;
    const PlayerCounterForceRun* mCounterForceRun;
    const PlayerCounterQuickTurnJump* mCounterQuickTurnJump;
    PlayerTrigger* mTrigger;
    PlayerAnimator* mAnimator;
    PlayerEffect* mEffect;
    PlayerJointParamCenterDynamics* mCenterDynamics;
    const IJudge* mJudgeWaterSurfaceRun;
    bool mIsInWaterSurface;
    u8 _71[0x37];
    f32 mInverseKinematicsRate;
    f32 mTurnTiltRate;
    u8 _b0[0x20];
};

static_assert(sizeof(PlayerStateRunHakoniwa) == 0xD0);
