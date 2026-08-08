#pragma once

#include <basis/seadTypes.h>

#include "Player/IJudge.h"

class PlayerConst;
class PlayerInput;

class PlayerJudgePreInputPoleClimbSwing : public IJudge {
public:
    PlayerJudgePreInputPoleClimbSwing(const PlayerConst* pConst, const PlayerInput* input);

    void reset() override;
    void update() override;
    bool judge() const override;

private:
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    s32 mCounter;
};

static_assert(sizeof(PlayerJudgePreInputPoleClimbSwing) == 0x20);
