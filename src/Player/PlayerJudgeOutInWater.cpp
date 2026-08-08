#include "Player/PlayerJudgeOutInWater.h"

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"

#include "Util/JudgeUtil.h"

PlayerJudgeOutInWater::PlayerJudgeOutInWater(const al::LiveActor* player,
                                             const IJudge* judgeInWater,
                                             const IJudge* judgeInWaterSurface)
    : mPlayer(player), mJudgeInWater(judgeInWater), mJudgeInWaterSurface(judgeInWaterSurface) {}

void PlayerJudgeOutInWater::reset() {
    mIsInWater = true;
}

void PlayerJudgeOutInWater::update() {
    if (!mIsInWater)
        return;

    bool isInWater;
    if (rs::isJudge(mJudgeInWaterSurface)) {
        isInWater = al::getVelocity(mPlayer).dot(al::getGravity(mPlayer)) < 0.0f;
    } else {
        isInWater = false;
    }
    mIsInWater = isInWater;
}

bool PlayerJudgeOutInWater::judge() const {
    if (mIsInWater)
        return false;
    return rs::isJudge(mJudgeInWater);
}
