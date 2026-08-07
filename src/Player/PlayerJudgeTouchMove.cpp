#include "Player/PlayerJudgeTouchMove.h"

#include <math/seadMathCalcCommon.h>

#include "Library/Controller/InputFunction.h"

#include "Player/TouchTargetKeeper.h"

PlayerJudgeTouchMove::PlayerJudgeTouchMove(const TouchTargetKeeper* keeper)
    : mTouchTargetKeeper(keeper), mKeepFrame(60), mRemainFrame(0) {}

void PlayerJudgeTouchMove::reset() {
    mRemainFrame = 0;
}

void PlayerJudgeTouchMove::update() {
    if (al::isPadReleaseTouch()) {
        if (!mTouchTargetKeeper->isTouchTarget())
            return;
        mRemainFrame = mKeepFrame;
    } else {
        mRemainFrame = sead::Mathi::clampMin(mRemainFrame - 1, 0);
    }
}

bool PlayerJudgeTouchMove::judge() const {
    return mRemainFrame > 0;
}
