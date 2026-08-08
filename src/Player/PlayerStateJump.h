#pragma once

#include <basis/seadTypes.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
class Nerve;
}
class HackCap;
class IJudge;
class IPlayerModelChanger;
class IUseDimension;
class IUsePlayerCollision;
class PlayerActionAirMoveControl;
class PlayerActionDiveInWater;
class PlayerAnimator;
class PlayerConst;
class PlayerContinuousJump;
class PlayerCounterForceRun;
class PlayerInput;
class PlayerJointControlKeeper;
struct PlayerJumpMessageRequest;
class PlayerTrigger;

class PlayerStateJump : public al::ActorStateBase {
public:
    PlayerStateJump(al::LiveActor* player, const PlayerConst* pConst,
                    const IPlayerModelChanger* modelChanger, const IUseDimension* dimension,
                    const PlayerInput* input, const PlayerJumpMessageRequest* jumpRequest,
                    const IJudge* judgeStartRise,
                    const PlayerCounterForceRun* counterForceRun,
                    const PlayerJointControlKeeper* jointControlKeeper,
                    const IUsePlayerCollision* collision, PlayerTrigger* trigger,
                    PlayerContinuousJump* continuousJump, PlayerAnimator* animator,
                    PlayerActionDiveInWater* actionDiveInWater, HackCap* hackCap,
                    IJudge* judgeWallKeep, bool is2D);

    void appear() override;
    f32 calcJumpPowerBorderSpeedMin() const;
    f32 calcJumpPowerBorderSpeedMax() const;
    f32 calcJumpPowerMin() const;
    f32 calcJumpPowerMax() const;
    f32 calcJumpGravity() const;
    void tryCountUpContinuousJump(PlayerContinuousJump* continuousJump);
    bool isJumpCapCatch() const;
    bool isJumpSpinFlower() const;
    bool isJumpSpinGround() const;
    bool isJumpSpinGroundClockwise() const;
    bool isJumpBack() const;
    bool isHovering() const;
    bool isEndJumpDownFallLand() const;
    bool isHoldDownFall() const;
    bool isEnableHipDropStart() const;
    bool isEnableTrampleByHipDropAttack() const;
    bool isEnableReactionCapCatch() const;
    bool isEnableCancelCarryThrow() const;
    bool isFormSquat2D() const;
    void exeJump();
    bool trySubAnimJumpReaction();
    const char* calcJumpAnimName() const;
    void exeJumpSpinFlower();
    void exeJumpSpinFlowerDownFall();
    void updateNerveDownFall(const char* animationName, f32 gravity, f32 maxFallSpeed,
                             f32 moveRate, const al::Nerve* nextNerve);
    void exeJumpSpinGround();
    void exeJumpSpinGroundDownFall();
    void exeJumpTurn();
    void exeJumpBack();
    void exeHoveringJump2D();
    void exeHovering2D();
    ~PlayerStateJump() override;

private:
    const PlayerConst* mConst;
    const IPlayerModelChanger* mModelChanger;
    const IUseDimension* mDimension;
    const PlayerJumpMessageRequest* mJumpRequest;
    const IJudge* mJudgeStartRise;
    const PlayerCounterForceRun* mCounterForceRun;
    const PlayerJointControlKeeper* mJointControlKeeper;
    const IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    PlayerContinuousJump* mContinuousJump;
    PlayerTrigger* mTrigger;
    PlayerActionDiveInWater* mActionDiveInWater;
    PlayerActionAirMoveControl* mAirMoveControl;
    HackCap* mHackCap;
    IJudge* mJudgeWallKeep;
    bool mIs2D;
    u8 _99[3];
    s32 _9c;
    f32 _a0;
    f32 _a4;
    f32 _a8;
    s32 _ac;
    s32 _b0;
    u8 _b4;
    u8 _b5;
    u8 _b6;
    u8 _b7;
    u8 _b8;
    u8 _b9;
    u8 _ba;
    u8 _bb;
    sead::Vector3f _bc;
    const char* _c8;
    const char* _d0;
    const PlayerInput* mInput;
    union {
        sead::Quatf mJumpTurnQuat;
        struct {
            sead::Vector3f turnJumpAngle;
            s32 downFallConvergeCounter;
        };
    };
    bool mIsDownFallGroundCollision;
    u8 _f1[3];
    sead::Vector3f mDownFallGroundPos;
};

