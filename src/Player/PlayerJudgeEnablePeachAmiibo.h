#pragma once

#include "Player/IJudge.h"

class PlayerStateDamageFire;

class PlayerJudgeEnablePeachAmiibo : public IJudge {
public:
    PlayerJudgeEnablePeachAmiibo(const PlayerStateDamageFire* stateDamageFire);

    bool judge() const override;
    void reset() override {}
    void update() override {}

private:
    const PlayerStateDamageFire* mStateDamageFire;
};

static_assert(sizeof(PlayerJudgeEnablePeachAmiibo) == 0x10);
