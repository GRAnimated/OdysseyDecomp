#pragma once

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}

class HackCap;
class PlayerColliderHakoniwa;
class PlayerConst;
class PlayerInput;

class HackCapJudgeHoldHoveringJump : public IJudge {
public:
    HackCapJudgeHoldHoveringJump(const al::LiveActor* player, const PlayerColliderHakoniwa* collider,
                                 const HackCap* hackCap, const PlayerInput* input,
                                 const PlayerConst* playerConst);

    bool judge() const override;
    void reset() override;
    void update() override;

private:
    const al::LiveActor* mPlayer;
    const PlayerColliderHakoniwa* mCollider;
    const HackCap* mHackCap;
    const PlayerInput* mInput;
    const PlayerConst* mPlayerConst;
};

static_assert(sizeof(HackCapJudgeHoldHoveringJump) == 0x30);
