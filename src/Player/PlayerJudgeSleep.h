#pragma once

#include "Player/IJudge.h"

class PlayerStateWait;

class PlayerJudgeSleep : public IJudge {
public:
    PlayerJudgeSleep(const PlayerStateWait* stateWait);
    void reset() override {};
    void update() override {};
    bool judge() const override;

private:
    const PlayerStateWait* mStateWait;
};

static_assert(sizeof(PlayerJudgeSleep) == 0x10);
