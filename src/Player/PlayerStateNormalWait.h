#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}
class IUsePlayerCollision;
class PlayerAnimator;
class PlayerConst;

class PlayerStateNormalWait : public al::ActorStateBase {
public:
    PlayerStateNormalWait(al::LiveActor* player, const PlayerConst* pConst, const char* actionName,
                          const IUsePlayerCollision* collision, PlayerAnimator* animator);
    ~PlayerStateNormalWait() override;

    void appear() override;
    void exeWait();

private:
    const PlayerConst* mConst;
    const char* mActionName;
    const IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
};

static_assert(sizeof(PlayerStateNormalWait) == 0x40);
