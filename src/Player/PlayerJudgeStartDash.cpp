#include "Player/PlayerJudgeStartDash.h"

#include "Player/PlayerInput.h"

PlayerJudgeStartDash::PlayerJudgeStartDash(const PlayerInput* input) : mInput(input) {}

void PlayerJudgeStartDash::reset() {
    mIsEnable = false;
}

void PlayerJudgeStartDash::update() {
    mIsEnable = mInput->isEnableDashInput();
}

bool PlayerJudgeStartDash::judge() const {
    return mIsEnable;
}
