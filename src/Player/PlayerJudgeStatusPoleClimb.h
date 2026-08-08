#pragma once

#include "Player/IJudge.h"

class PlayerStatePoleClimb;

class PlayerJudgeStatusPoleClimb : public IJudge {
public:
    PlayerJudgeStatusPoleClimb(const IJudge* judge, const PlayerStatePoleClimb* statePoleClimb);

    bool judge() const override;
    void reset() override {}
    void update() override {}

private:
    const IJudge* mJudge;
    const PlayerStatePoleClimb* mStatePoleClimb;
};

static_assert(sizeof(PlayerJudgeStatusPoleClimb) == 0x18);
