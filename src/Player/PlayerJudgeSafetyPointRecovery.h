#pragma once

#include "Player/IJudge.h"

class PlayerStateAbyss;

class PlayerJudgeSafetyPointRecovery : public IJudge {
public:
    PlayerJudgeSafetyPointRecovery(IJudge* judge, const PlayerStateAbyss* stateAbyss);

    bool judge() const override;
    void reset() override {}
    void update() override {}

private:
    IJudge* mJudge;
    const PlayerStateAbyss* mStateAbyss;
};

static_assert(sizeof(PlayerJudgeSafetyPointRecovery) == 0x18);
