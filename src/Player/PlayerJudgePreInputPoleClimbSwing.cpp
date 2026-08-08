#include "Player/PlayerJudgePreInputPoleClimbSwing.h"

#include "Player/PlayerConst.h"
#include "Player/PlayerInput.h"

PlayerJudgePreInputPoleClimbSwing::PlayerJudgePreInputPoleClimbSwing(const PlayerConst* pConst,
                                                                     const PlayerInput* input)
    : mConst(pConst), mInput(input), mCounter(0) {}

void PlayerJudgePreInputPoleClimbSwing::reset() {
    mCounter = 0;
}

void PlayerJudgePreInputPoleClimbSwing::update() {
    s32 counter = mCounter - 1;
    if (counter < 0)
        counter = 0;
    mCounter = counter;
    if (mInput->isTriggerSwingPoleClimbFast())
        mCounter = mConst->getPoleClimbPreInputSwing();
}

bool PlayerJudgePreInputPoleClimbSwing::judge() const {
    return mInput->isTriggerSwingPoleClimbFast() || mCounter > 0;
}
