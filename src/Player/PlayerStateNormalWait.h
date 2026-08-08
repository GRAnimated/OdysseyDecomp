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
    void appear() override;
    void exeWait();
~PlayerStateNormalWait() override;

private:
    const PlayerConst* mConst;
    const char* mActionName;
    const IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
};

