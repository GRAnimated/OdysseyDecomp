#include "Player/HackCapJudgePreInputSeparateThrow.h"

#include "Library/Math/MathUtil.h"

#include "Player/PlayerInput.h"
#include "Player/PlayerSeparateCapFlag.h"

HackCapJudgePreInputSeparateThrow::HackCapJudgePreInputSeparateThrow(
    const PlayerInput* input, const PlayerSeparateCapFlag* separateCapFlag)
    : mInput(input), mSeparateCapFlag(separateCapFlag) {}

void HackCapJudgePreInputSeparateThrow::reset() {
    mRemainFrame = 0;
}

void HackCapJudgePreInputSeparateThrow::update() {
    const u32 flags = mSeparateCapFlag->getRawFlags();
    const u32 press = flags & 0xFF0000;
    const u32 separate = flags & 0xFF;
    if (press || !separate) {
        mRemainFrame = 0;
        return;
    }

    mRemainFrame = al::converge(mRemainFrame, 0, 1);
    if (mInput->isTriggerCapAttackSeparate())
        mRemainFrame = 5;
}

bool HackCapJudgePreInputSeparateThrow::judge() const {
    const u32 flags = mSeparateCapFlag->getRawFlags();
    const u32 press = flags & 0xFF0000;
    const u32 separate = flags & 0xFF;
    if (press || !separate)
        return false;
    return mInput->isTriggerCapAttackSeparate() || mRemainFrame > 0;
}
