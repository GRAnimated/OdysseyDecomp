#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}
class HackCap;
class IUsePlayerCollision;
class PlayerAnimator;
class PlayerActionAirMoveControl;
class PlayerConst;
class PlayerInput;

class PlayerStateCapCatchPop : public al::ActorStateBase {
public:
    PlayerStateCapCatchPop(al::LiveActor* player, const PlayerConst* pConst,
                           const IUsePlayerCollision* collision, const PlayerInput* input,
                           PlayerAnimator* animator, HackCap* hackCap);
    void appear() override;

    void exePop();
    ~PlayerStateCapCatchPop() override;

private:
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    PlayerActionAirMoveControl* mAirMoveControl = nullptr;
    HackCap* mHackCap;
};

static_assert(sizeof(PlayerStateCapCatchPop) == 0x48);
