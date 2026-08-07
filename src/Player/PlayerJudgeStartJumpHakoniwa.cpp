#include "Player/PlayerJudgeStartJumpHakoniwa.h"

#include "Player/PlayerInput.h"
#include "Util/JudgeUtil.h"

PlayerJudgeStartJumpHakoniwa::PlayerJudgeStartJumpHakoniwa(const PlayerInput* input, IJudge* judge)
    : mInput(input), mJudge(judge), mIsTrigger(false) {}

void PlayerJudgeStartJumpHakoniwa::update() {
    if (rs::updateJudgeAndResult(mJudge))
        mIsTrigger = false;
    else
        mIsTrigger = mInput->isTriggerJump();
}

void PlayerJudgeStartJumpHakoniwa::reset() {
    mIsTrigger = false;
}

bool PlayerJudgeStartJumpHakoniwa::judge() const {
    return mIsTrigger;
}
