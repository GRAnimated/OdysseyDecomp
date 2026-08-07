#pragma once

#include <basis/seadTypes.h>

#include "Player/IJudge.h"

class PlayerInput;
class PlayerSeparateCapFlag;

class HackCapJudgePreInputSeparateJump : public IJudge {
public:
    HackCapJudgePreInputSeparateJump(const PlayerInput* input,
                                     const PlayerSeparateCapFlag* separateCapFlag);

    void reset() override;
    void update() override;
    bool judge() const override;

private:
    const PlayerInput* mInput;
    const PlayerSeparateCapFlag* mSeparateCapFlag;
    s32 mRemainFrame = 0;
};

static_assert(sizeof(HackCapJudgePreInputSeparateJump) == 0x20);
