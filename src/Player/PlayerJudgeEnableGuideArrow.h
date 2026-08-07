#pragma once

#include "Player/IJudge.h"

class PlayerPuppet;

class PlayerJudgeEnableGuideArrow : public IJudge {
public:
    PlayerJudgeEnableGuideArrow(const PlayerPuppet* puppet);

    void reset() override {}
    void update() override {}
    bool judge() const override;

private:
    const PlayerPuppet* mPuppet;
};

static_assert(sizeof(PlayerJudgeEnableGuideArrow) == 0x10);
