#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}  // namespace al
class IJudge;
class IUsePlayerCollision;
class PlayerAnimator;
class PlayerConst;
class PlayerInput;

class PlayerStateRun : public al::ActorStateBase {
public:
    PlayerStateRun(al::LiveActor* player, const PlayerConst* pConst, const PlayerInput* input,
                   const IUsePlayerCollision* collision, PlayerAnimator* animator, IJudge* judge);
    ~PlayerStateRun() override = default;

    void appear() override;

    bool tryTurnJump(IJudge*);
    void exePivot();
    void exeRun();
    bool tryChangeRunAnim(const char*);
    void exeBrake();
    void exeTurn();

private:
    u8 _20[0x38];
};

static_assert(sizeof(PlayerStateRun) == 0x58);
