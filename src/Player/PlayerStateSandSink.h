#pragma once

#include "Library/Nerve/NerveStateBase.h"

#include "Player/PlayerActionPivotTurnControl.h"

class IJudge;
class IUsePlayerCollision;
class PlayerActionAirMoveControl;
class PlayerActionGroundMoveControl;
class PlayerAnimator;
class PlayerConst;
class PlayerInput;
class PlayerJudgePreInputJump;
class PlayerTrigger;

class PlayerStateSandSink : public al::ActorStateBase {
public:
    PlayerStateSandSink(al::LiveActor*, const PlayerConst*, const PlayerInput*,
                        const PlayerTrigger*, IUsePlayerCollision*, PlayerAnimator*, IJudge*,
                        PlayerJudgePreInputJump*);

    void appear() override;
    void kill() override;
    void setSandSinkDead();
    bool isJump() const;
    bool isDeadStatus() const;
    void exeWait();
    void exePivot();
    void exeRun();
    void exeJump();
    void exeDead();

    void setSandSinkDeathRate(f32 rate) { mSandSinkDeathRate = rate; }

private:
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    const PlayerTrigger* mTrigger;
    IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    PlayerActionAirMoveControl* mActionAirMoveControl;
    PlayerActionPivotTurnControl* mActionPivotTurnControl;
    PlayerActionGroundMoveControl* mActionGroundMoveControl;
    IJudge* mJudge;
    PlayerJudgePreInputJump* mJudgePreInputJump;
    f32 mSandSinkDeathRate;
};

static_assert(sizeof(PlayerStateSandSink) == 0x78);
