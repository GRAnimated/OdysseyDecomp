#include "Player/PlayerAnimControlSwimWalk.h"

#include "Library/Math/MathUtil.h"

#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"

PlayerAnimControlSwimWalk::PlayerAnimControlSwimWalk(PlayerAnimator* animator,
                                                     const PlayerConst* playerConst)
    : mAnimator(animator), mPlayerConst(playerConst) {}

void PlayerAnimControlSwimWalk::update(f32 speed) {
    const f32 rate = al::calcRate01(speed, mPlayerConst->getSwimWalkAnimSpeedMin(),
                                    mPlayerConst->getSwimWalkAnimSpeedMax());
    const f32 animRate = al::lerpValue(mPlayerConst->getSwimWalkAnimFrameRateMin(),
                                       mPlayerConst->getSwimWalkAnimFrameRateMax(), rate);
    mAnimator->setAnimRate(animRate);
}
