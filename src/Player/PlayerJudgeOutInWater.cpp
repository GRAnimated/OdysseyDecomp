#include "Player/PlayerJudgeOutInWater.h"

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"

#include "Util/JudgeUtil.h"

PlayerJudgeOutInWater::PlayerJudgeOutInWater(const al::LiveActor* player,
                                             const IJudge* judgeInWater,
                                             const IJudge* judgeInWaterSurface)
    : mPlayer(player), mJudgeInWater(judgeInWater), mJudgeInWaterSurface(judgeInWaterSurface),
      mIsInWater(true) {
    return;
}

void PlayerJudgeOutInWater::reset() {
    mIsInWater = true;
}

void PlayerJudgeOutInWater::update() {
    if (!mIsInWater)
        return;

    bool isInWater;
    if (rs::isJudge(mJudgeInWaterSurface)) {
        const sead::Vector3f& velocity = al::getVelocity(mPlayer);
        const sead::Vector3f& gravity = al::getGravity(mPlayer);
        isInWater = velocity.dot(gravity) < 0.0f;
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
