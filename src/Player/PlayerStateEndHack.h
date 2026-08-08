#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}
class IUsePlayerCollision;
class PlayerAnimator;
class PlayerActionAirMoveControl;
class PlayerConst;
class PlayerInput;
class PlayerTrigger;

class PlayerStateEndHack : public al::ActorStateBase {
public:
    PlayerStateEndHack(al::LiveActor* player, const PlayerConst* pConst,
                       const IUsePlayerCollision* collision, const PlayerInput* input,
                       PlayerTrigger* trigger, PlayerAnimator* animator);
    void appear() override;
    void exeJump();
    ~PlayerStateEndHack() override;

    void setEndVelocity(const sead::Vector3f& velocity, s32 delayFrames) {
        mEndVelocity = velocity;
        mDelayFrames = delayFrames;
    }

private:
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    const PlayerInput* mInput;
    PlayerTrigger* mTrigger;
    PlayerAnimator* mAnimator;
    PlayerActionAirMoveControl* mAirMoveControl = nullptr;
    sead::Vector3f mEndVelocity = {0.0f, 0.0f, 0.0f};
    s32 mDelayFrames = 0;
};

static_assert(sizeof(PlayerStateEndHack) == 0x60);
