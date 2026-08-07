#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class HitSensor;
class LiveActor;
}  // namespace al
class IJudge;
class IUsePlayerCollision;
class PlayerActionAirMoveControl;
class PlayerActionSlopeSlideControl;
class PlayerAnimator;
class PlayerConst;
class PlayerContinuousLongJump;
class PlayerEffect;
class PlayerInput;
class PlayerJudgePreInputCapThrow;
class PlayerJudgePreInputJump;
class PlayerJudgeStartRolling;
class PlayerSeCtrl;
class PlayerTrigger;

class PlayerStateRolling : public al::ActorStateBase {
public:
    PlayerStateRolling(al::LiveActor* player, const PlayerConst* pConst, const PlayerInput* input,
                       const IUsePlayerCollision* collision, PlayerTrigger* trigger,
                       PlayerAnimator* animator, PlayerEffect* effect,
                       PlayerJudgeStartRolling* judgeStartRolling, IJudge* judgeDirectRolling,
                       PlayerJudgePreInputJump* judgePreInputJump,
                       PlayerJudgePreInputCapThrow* judgePreInputCapThrow, IJudge* judgeStartRise,
                       PlayerContinuousLongJump* continuousLongJump, PlayerSeCtrl* seCtrl);
    ~PlayerStateRolling() override;

    void appear() override;
    void kill() override;
    bool update() override;
    void control() override;
    bool isRolling() const;
    bool isRollingJump() const;
    bool isEnableCancelNormalJump() const;
    bool isEndSquat() const;
    bool isEndStandUp() const;
    bool isEnableTrample(const al::HitSensor* self, const al::HitSensor* other) const;
    f32 getInverseKinematicsRate() const;
    PlayerEffect* getEffect() const { return mEffect; }
    void exeBoostStart();
    void updateRollingAnimFrameRate();
    void exeStart();
    bool isStartRollingBrake() const;
    void exeRolling();
    void restartRolling(bool isBoost, bool isInWater);
    void exeJump();
    void exeLand();
    void exeUnRoll();
    void exeBrake();
    void exeStandUp();
    void exeEndSquat();

private:
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    const IUsePlayerCollision* mCollision;
    PlayerTrigger* mTrigger;
    PlayerAnimator* mAnimator;
    PlayerEffect* mEffect;
    PlayerActionAirMoveControl* mAirMoveControl;
    PlayerActionSlopeSlideControl* mSlopeSlideControl;
    PlayerContinuousLongJump* mContinuousLongJump;
    PlayerJudgeStartRolling* mJudgeStartRolling;
    IJudge* mJudgeDirectRolling;
    PlayerJudgePreInputJump* mJudgePreInputJump;
    PlayerJudgePreInputCapThrow* mJudgePreInputCapThrow;
    IJudge* mJudgeStartRise;
    s32 _90;
    s32 _94;
    s32 _98;
    s32 _9c;
    s32 _a0;
    u8 _a4[4];
    PlayerSeCtrl* mSeCtrl;
    bool _b0;
    u8 _b1[7];
};

static_assert(sizeof(PlayerStateRolling) == 0xB8);
