#include "Player/PlayerJudgeStartSwimJump.h"

#include "Library/Math/MathUtil.h"
#include "Library/Nature/WaterSurfaceFinder.h"

#include "Player/PlayerConst.h"
#include "Player/PlayerInput.h"

PlayerJudgeStartSwimJump::PlayerJudgeStartSwimJump(
    const PlayerInput* input, const PlayerConst* pConst,
    const al::WaterSurfaceFinder* waterSurfaceFinder)
    : mInput(input), mConst(pConst), mWaterSurfaceFinder(waterSurfaceFinder), mRemainFrame(0) {}

void PlayerJudgeStartSwimJump::reset() {
    mRemainFrame = 0;
}

void PlayerJudgeStartSwimJump::update() {
    if (mInput->isTriggerJump())
        mRemainFrame = mConst->getSwimSurfacePreInputJumpFrame();
    mRemainFrame = al::converge(mRemainFrame, 0, 1);
}

bool PlayerJudgeStartSwimJump::judge() const {
    return (mInput->isTriggerJump() || mRemainFrame >= 1) &&
           mWaterSurfaceFinder->isFoundSurface() &&
           mWaterSurfaceFinder->getDistance() <= mConst->getSwimSurfaceEnableJumpHeight();
}
