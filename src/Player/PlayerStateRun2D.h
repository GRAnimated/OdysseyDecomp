#pragma once

#include "Library/Nerve/NerveStateBase.h"

class IUsePlayerCollision;
class PlayerActionGroundMoveControl;
class PlayerAnimator;
class PlayerConst;
class PlayerInput;

class PlayerStateRun2D : public al::ActorStateBase {
public:
    PlayerStateRun2D(al::LiveActor* player, const PlayerConst* pConst, const PlayerInput* input,
                     const IUsePlayerCollision* collision, PlayerAnimator* animator);
    void appear() override;

    bool isBrake() const;
    void exeRun();
    void exeBrake();
    void exeTurn();
    ~PlayerStateRun2D() override;

private:
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    const IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    PlayerActionGroundMoveControl* mMoveControl = nullptr;
};

static_assert(sizeof(PlayerStateRun2D) == 0x48);
