#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class HitSensor;
class LiveActor;
class WaterSurfaceFinder;
}  // namespace al
class IUsePlayerCollision;
class PlayerActionTurnControl;
class PlayerAnimControlSwimWalk;
class PlayerAnimator;
class PlayerCarryKeeper;
class PlayerConst;
class PlayerEffect;
class PlayerExternalVelocity;
class PlayerInput;
class PlayerJudgeReduceOxygen;
class PlayerJudgeStartSwimJump;
struct PlayerJointParamSwim;
class PlayerModelHolder;
class PlayerSpinCapAttack;
class PlayerTrigger;

class PlayerStateSwim : public al::ActorStateBase {
public:
    PlayerStateSwim(al::LiveActor* player, const PlayerConst* pConst,
                    const IUsePlayerCollision* collision, const PlayerInput* input,
                    const PlayerTrigger* trigger, const PlayerCarryKeeper* carryKeeper,
                    const PlayerModelHolder* modelHolder,
                    const PlayerExternalVelocity* externalVelocity, PlayerAnimator* animator,
                    PlayerSpinCapAttack* spinCapAttack,
                    const al::WaterSurfaceFinder* waterSurfaceFinder, PlayerEffect* effect,
                    PlayerJointParamSwim* swimJoint);
    ~PlayerStateSwim() override;

    void appear() override;
    void tryReactionWaterIn();
    void kill() override;
    void control() override;
    bool attackHipDropKnockDown(al::HitSensor* self, al::HitSensor* other);
    f32 calcEyeHeadTiltRate() const;
    bool isEndSwimJump() const;
    bool isEndSwimJumpPop() const;
    bool isEndSwimJumpHipDrop() const;
    bool isEnableHipDropAttack() const;
    bool isEnableCapThrow() const;
    bool isEnableCapThrowSurface() const;
    bool isSurface() const;
    bool isEnableTalkSwim() const;
    bool isEnableLookAt() const;
    bool isEnableReactionCapCatch() const;
    bool isEnableCarryAction() const;
    bool isEnableWallHitDown() const;
    bool isHeadSliding() const;
    bool isEnableSurfaceShadow() const;
    bool isEnableRecoveryLife() const;
    bool isSpinCapSurface() const;
    bool isReduceOxygen() const;
    bool isWaitGround() const;
    bool isEnableTrample() const;
    bool isEnableCapTouchJump() const;
    void startCapThrow();
    void startCapThrowSurface();
    bool tryReactionWaterOut();
    void exeSwimSurface();
    void exeSwimMove();
    void exeSwimPaddle();
    void exeSwimLand();
    void exeSwimWalk();
    void exeSwimDive();
    void updateNerveDownFall();
    void exeSwimFallIn();
    void exeSwimHeadIn();
    void exeSwimDiveIn();
    void exeSwimHipDropStart();
    void exeSwimHipDropLoop();
    void exeSwimHipDropLand();
    void exeSwimHipDropHeadSliding();
    void exeSwimHipDropHeadSlidingEnd();
    void exeSwimHipDropJump();
    void exeSwimSpinCap();
    void exeSwimSpinCapSurface();
    void exeSwimTrample();
    void exeSwimReflectDownFall();
    void exeEndSwimJump();
    void exeEndSwimJumpPop();
    void exeEndSwimJumpHipDrop();
    bool tryChangeHipDropLand(s32* state, s32* step);

private:
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    const PlayerInput* mInput;
    const PlayerTrigger* mTrigger;
    const PlayerCarryKeeper* mCarryKeeper;
    const PlayerModelHolder* mModelHolder;
    const PlayerExternalVelocity* mExternalVelocity;
    PlayerAnimator* mAnimator;
    PlayerEffect* mEffect;
    PlayerJointParamSwim* mSwimJoint;
    PlayerSpinCapAttack* mSpinCapAttack;
    PlayerActionTurnControl* mTurnControl;
    PlayerAnimControlSwimWalk* mAnimControlSwimWalk;
    const al::WaterSurfaceFinder* mWaterSurfaceFinder;
    PlayerJudgeReduceOxygen* mJudgeReduceOxygen;
    PlayerJudgeStartSwimJump* mJudgeStartSwimJump;
    s32 _a0;
    f32 _a4;
    bool _a8;
    u8 _a9[3];
    s32 _ac;
    f32 _b0;
    s32 _b4;
    s32 _b8;
    s32 _bc;
    sead::Vector3f _c0;
    s32 _cc;
    u8 _d0[8];
    sead::Vector3f _d8;
    sead::Vector3f _e4;
    s32 _f0;
    s32 _f4;
    s32 _f8;
    s32 _fc;
    s32 _100;
    bool _104;
    u8 _105[3];
    s32 _108;
    s32 _10c;
    bool _110;
    u8 _111[7];
};

static_assert(sizeof(PlayerStateSwim) == 0x118);
