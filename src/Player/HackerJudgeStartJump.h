#pragma once

#include "Util/HackerJudge.h"

class IUsePlayerHack;

class HackerJudgeStartJump : public HackerJudge {
public:
    HackerJudgeStartJump(IUsePlayerHack** hacker);
    bool judge() const override;
    void reset() override;
    void update() override;
};

static_assert(sizeof(HackerJudgeStartJump) == 0x10);
