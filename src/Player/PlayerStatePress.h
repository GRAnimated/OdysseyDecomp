#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}
class IUsePlayerCollision;
class PlayerAnimator;

class PlayerStatePress : public al::ActorStateBase {
public:
    PlayerStatePress(al::LiveActor* player, const IUsePlayerCollision* collision,
                     PlayerAnimator* animator);
    ~PlayerStatePress() override;

    void appear() override;
    void exePress();

private:
    const IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    sead::Vector3f mOriginalScale;
    sead::Vector3f mScale;
    sead::Vector3f mOriginalTrans;
    sead::Vector3f mTrans;
};

static_assert(sizeof(PlayerStatePress) == 0x60);
