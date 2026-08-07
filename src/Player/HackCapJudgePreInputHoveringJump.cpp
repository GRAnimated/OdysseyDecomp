#include "Player/HackCapJudgePreInputHoveringJump.h"

#include "Library/Math/MathUtil.h"

#include "Player/HackCap.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerInputFunction.h"
#include "Util/PlayerCollisionUtil.h"

HackCapJudgePreInputHoveringJump::HackCapJudgePreInputHoveringJump(
    const al::LiveActor* player, const IUsePlayerCollision* collision, const HackCap* hackCap,
    const PlayerInput* input)
    : mPlayer(player), mCollision(collision), mHackCap(hackCap), mInput(input) {}

void HackCapJudgePreInputHoveringJump::reset() {
    mRemainFrame = 0;
    mIsDisabled = false;
}

void HackCapJudgePreInputHoveringJump::update() {
    if (rs::isSeparatePlay(mHackCap)) {
        mRemainFrame = al::converge(mRemainFrame, 0, 1);
        if (mInput->isTriggerCapSeparateJump())
            mRemainFrame = 5;
    } else {
        mRemainFrame = 0;
    }

    if (rs::isOnGround(mPlayer, mCollision))
        mIsDisabled = false;
}

bool HackCapJudgePreInputHoveringJump::judge() const {
    if (!rs::isSeparatePlay(mHackCap) || mIsDisabled)
        return false;
    if (mInput->isTriggerCapSeparateJump())
        return true;
    return mRemainFrame > 0;
}
