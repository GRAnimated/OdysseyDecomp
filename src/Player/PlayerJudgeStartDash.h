#pragma once

#include "Player/IJudge.h"

class PlayerInput;

class PlayerJudgeStartDash : public IJudge {
public:
    PlayerJudgeStartDash(const PlayerInput* input);

    void reset() override;
    void update() override;
    bool judge() const override;

private:
    const PlayerInput* mInput;
    bool mIsEnable = false;
};

static_assert(sizeof(PlayerJudgeStartDash) == 0x18);
