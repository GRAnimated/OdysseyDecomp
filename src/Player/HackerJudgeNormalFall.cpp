#include "Player/HackerJudgeNormalFall.h"

#include "Library/LiveActor/ActorCollisionFunction.h"

#include "Util/PlayerCollisionUtil.h"

HackerJudgeNormalFall::HackerJudgeNormalFall(const al::LiveActor* parent, s32 fallTime)
    : mActor(parent), mFallTime(fallTime) {}

void HackerJudgeNormalFall::reset() {
    mAirTime = 0;
}

void HackerJudgeNormalFall::update() {
    const bool isGrounded = mPlayerCollision ? rs::isCollidedGround(mPlayerCollision)
                                              : al::isCollidedGround(mActor);
    if (isGrounded) {
        mAirTime = 0;
        return;
    }

    mAirTime = sead::Mathi::clampMax(mAirTime + 1, mFallTime);
}

bool HackerJudgeNormalFall::judge() const {
    return mAirTime >= mFallTime;
}
