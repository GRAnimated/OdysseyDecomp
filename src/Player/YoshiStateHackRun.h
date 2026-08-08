#pragma once

#include <math/seadVector.h>

#include "Player/HackerStateBase.h"

class IUsePlayerCollision;
class PlayerActionGroundMoveControl;
class PlayerActionPivotTurnControl;
class PlayerAnimControlRun;
class PlayerAnimator;
class PlayerConst;
class PlayerCounterForceRun;
class PlayerEffect;

class YoshiStateHackRun : public HackerStateBase {
public:
    YoshiStateHackRun(al::LiveActor* player, IUsePlayerHack** playerHack, al::LiveActor* hackActor,
                      const PlayerConst* playerConst, const IUsePlayerCollision* collision,
                      const PlayerCounterForceRun* counterForceRun, PlayerAnimator* animator,
                      PlayerEffect* effect);
    void appear() override;
    void kill() override;
    void endRun();

    bool tryTurnJump(sead::Vector3f* front);
    void invalidateTurn();
    void validateTurn();

    f32 getPoseRate() const { return _88; }

    void exePivot();
    void exeRun();
    void exeBrake();
    void exeTurn();
    ~YoshiStateHackRun() override = default;

private:
    al::LiveActor* mHackActor;
    const PlayerConst* mPlayerConst;
    const IUsePlayerCollision* mCollision;
    const PlayerCounterForceRun* mCounterForceRun;
    PlayerAnimator* mAnimator;
    PlayerEffect* mEffect;
    PlayerAnimControlRun* mAnimControlRun = nullptr;
    PlayerActionPivotTurnControl* mPivotTurnControl = nullptr;
    PlayerActionGroundMoveControl* mGroundMoveControl = nullptr;
    bool mIsTurnJump = false;
    sead::Vector3f _74 = {0.0f, 0.0f, 0.0f};
    f32 _80 = 0.0f;
    f32 _84 = 0.0f;
    f32 _88 = 0.0f;
};

static_assert(sizeof(YoshiStateHackRun) == 0x90);
