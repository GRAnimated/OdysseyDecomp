#include "Player/PlayerJudgeStartRunOld.h"

#include "Library/LiveActor/ActorCollisionFunction.h"

#include "Player/PlayerInput.h"
#include "Util/PlayerCollisionUtil.h"

PlayerJudgeStartRunOld::PlayerJudgeStartRunOld(const al::LiveActor* player,
                                               const IUsePlayerCollision* collision,
                                               const PlayerInput* input)
    : mPlayer(player), mCollision(collision), mInput(input) {}

void PlayerJudgeStartRunOld::reset() {
    mIsStartRun = false;
}

void PlayerJudgeStartRunOld::update() {
    mIsStartRun = false;

    if (mCollision) {
        if (!rs::isCollided(mCollision))
            return;
    } else if (!al::isCollided(mPlayer)) {
        return;
    }

    if (mInput)
        mIsStartRun = mInput->isMove();
}

bool PlayerJudgeStartRunOld::judge() const {
    return mIsStartRun;
}
