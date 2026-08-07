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
    HackCapJudgeHoldHoveringJump(const al::LiveActor*, const PlayerColliderHakoniwa*,
                                 const HackCap*, const PlayerInput*, const PlayerConst*);

    void reset() override;
    void update() override;
    bool judge() const override;

private:
    const al::LiveActor* mPlayer;
    const PlayerColliderHakoniwa* mCollider;
    const HackCap* mHackCap;
    const PlayerInput* mInput;
    const PlayerConst* mPlayerConst;
};

static_assert(sizeof(HackCapJudgeHoldHoveringJump) == 0x30);
