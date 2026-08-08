#include "Player/PlayerJudgeRecoveryLifeFast.h"

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"

#include "Player/PlayerBindKeeper.h"
#include "Player/PlayerDamageKeeper.h"
#include "Player/PlayerHackKeeper.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerStateSwim.h"
#include "Player/PlayerStateWait.h"
#include "Util/JudgeUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerUtil.h"

PlayerJudgeRecoveryLifeFast::PlayerJudgeRecoveryLifeFast(
    const al::LiveActor* player, const PlayerDamageKeeper* damageKeeper,
    const PlayerBindKeeper* bindKeeper, const IUsePlayerCollision* collision,
    const PlayerInput* input, const PlayerHackKeeper* hackKeeper,
    const PlayerStateWait* stateWait, const PlayerStateSwim* stateSwim,
    const IJudge* judgeInWater)
    : mPlayer(player), mDamageKeeper(damageKeeper), mBindKeeper(bindKeeper),
      mCollision(collision), mInput(input), mHackKeeper(hackKeeper), mStateWait(stateWait),
      mStateSwim(stateSwim), mJudgeInWater(judgeInWater) {
    (void)mJudgeInWater;
}

bool PlayerJudgeRecoveryLifeFast::judge() const {
    if (mDamageKeeper->isDamageInvalid())
        return false;

    if (!mHackKeeper->getHackSensor()) {
        if (!mInput->isEnableShowTutorialInput())
            return false;
        if (mBindKeeper->getBindSensor() && mBindKeeper->sendMsgBindRecoveryLife())
            return true;
        if (rs::isCollidedGround(mCollision) && isEnableRecoveryLifeState())
            return !rs::isJudge(mJudgeInWater);
        return false;
    }

    if (mHackKeeper->isHackInvalidLifeRecovery() || !mInput->isEnableRecoveryLifeInput())
        return false;
    if (rs::isPlayerCollidedGround(mPlayer))
        return true;

    const PlayerHackKeeper* hackKeeper = mHackKeeper;
    const al::LiveActor* player = mPlayer;
    const sead::Vector3f& gravity = al::getGravity(player);
    sead::Vector3f velocity = al::getVelocity(player);
    f32 margin = hackKeeper->getHackStayGravityMargine();
    if (al::isInRange(gravity.dot(velocity), -margin, margin))
        al::verticalizeVec(&velocity, gravity, velocity);
    return al::isNearZero(velocity, 0.001f);
}

bool PlayerJudgeRecoveryLifeFast::isEnableRecoveryLifeState() const {
    return mStateWait->isEnableRecoveryLife() || mStateSwim->isEnableRecoveryLife();
}

void PlayerJudgeRecoveryLifeFast::reset() {}

void PlayerJudgeRecoveryLifeFast::update() {}
