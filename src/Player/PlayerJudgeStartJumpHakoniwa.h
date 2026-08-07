#pragma once

#include "Player/IJudge.h"

class PlayerInput;

class PlayerJudgeStartJumpHakoniwa : public IJudge {
public:
    PlayerJudgeStartJumpHakoniwa(const PlayerInput* input, IJudge* judge);

    void reset() override;
    void update() override;
    bool judge() const override;

private:
    const PlayerInput* mInput;
    IJudge* mJudge;
    bool mIsTrigger;
};

static_assert(sizeof(PlayerJudgeStartJumpHakoniwa) == 0x20);
