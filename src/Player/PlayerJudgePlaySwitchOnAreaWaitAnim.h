#pragma once

#include "Player/IJudge.h"

class PlayerStateWait;

class PlayerJudgePlaySwitchOnAreaWaitAnim : public IJudge {
public:
    PlayerJudgePlaySwitchOnAreaWaitAnim(const PlayerStateWait* stateWait);

    bool judge() const override;
    void reset() override {}
    void update() override {}

private:
    const PlayerStateWait* mStateWait;
};

static_assert(sizeof(PlayerJudgePlaySwitchOnAreaWaitAnim) == 0x10);
