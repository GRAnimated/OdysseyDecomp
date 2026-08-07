#include "Player/YoshiJudgeFallFromGround.h"

#include "Library/Math/MathUtil.h"

#include "Util/PlayerCollisionUtil.h"

YoshiJudgeFallFromGround::YoshiJudgeFallFromGround(const al::LiveActor* actor,
                                                   const PlayerConst* playerConst,
                                                   const IUsePlayerCollision* collision)
    : mActor(actor), mPlayerConst(playerConst), mCollision(collision) {}

void YoshiJudgeFallFromGround::reset() {
    mCount = 0;
}

// NON_MATCHING: the target keeps distinct grounded and converged stores, while Clang merges them through W0; next find the natural source discriminator without volatile or other matching-only qualifiers.
void YoshiJudgeFallFromGround::update() {
    if (rs::isOnGroundRunAngle(mActor, mCollision, mPlayerConst))
        mCount = 0;
    else
        mCount = al::converge(mCount, 5, 1);
}

bool YoshiJudgeFallFromGround::judge() const {
    return mCount > 4;
}
