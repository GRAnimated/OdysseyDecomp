#pragma once

#include "Player/IJudge.h"

class PlayerInput;

class PlayerJudgeStartJumpHakoniwa : public IJudge {
public:
    PlayerJudgeStartJumpHakoniwa(const PlayerInput* input, IJudge* judge);

    void update() override;
    void reset() override;
    bool judge() const override;

private:
    const PlayerInput* mInput;
    IJudge* mJudge;
    bool mIsTrigger = false;
};

