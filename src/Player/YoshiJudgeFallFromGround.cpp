#include "Player/YoshiJudgeFallFromGround.h"

#include "Library/Math/MathUtil.h"

#include "Util/PlayerCollisionUtil.h"

YoshiJudgeFallFromGround::YoshiJudgeFallFromGround(const al::LiveActor* player,
                                                   const PlayerConst* playerConst,
                                                   const IUsePlayerCollision* collision)
    : mPlayer(player), mPlayerConst(playerConst), mCollision(collision) {}

void YoshiJudgeFallFromGround::reset() {
    mAirTime = 0;
}

void YoshiJudgeFallFromGround::update() {
    if (rs::isOnGroundRunAngle(mPlayer, mCollision, mPlayerConst)) {
        volatile s32* airTime = &mAirTime;
        *airTime = 0;
        return;
    }
    mAirTime = al::converge(mAirTime, 5, 1);
}

bool YoshiJudgeFallFromGround::judge() const {
    return mAirTime > 4;
}
