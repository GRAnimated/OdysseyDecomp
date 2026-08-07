#include "Player/PlayerJudgePreInputHackAction.h"

#include <math/seadMathCalcCommon.h>

#include "Player/PlayerConst.h"
#include "Player/PlayerInput.h"

PlayerJudgePreInputHackAction::PlayerJudgePreInputHackAction(const PlayerConst* playerConst,
                                                             const PlayerInput* input)
    : mConst(playerConst), mInput(input) {}

void PlayerJudgePreInputHackAction::reset() {
    mRemainFrame = 0;
}

void PlayerJudgePreInputHackAction::update() {
    mRemainFrame = sead::Mathi::clampMin(mRemainFrame - 1, 0);
    if (mInput->isTriggerHackAction())
        mRemainFrame = mConst->getContinuousJumpPreInputFrame();
}

bool PlayerJudgePreInputHackAction::judge() const {
    if (mInput->isTriggerHackAction())
        return true;
    return mRemainFrame > 0;
}
