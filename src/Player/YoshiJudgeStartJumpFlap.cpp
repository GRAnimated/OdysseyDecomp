#include "Player/YoshiJudgeStartJumpFlap.h"

#include "Library/LiveActor/ActorMovementFunction.h"

#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackInputFunction.h"

YoshiJudgeStartJumpFlap::YoshiJudgeStartJumpFlap(IUsePlayerHack** hacker,
                                                 const al::LiveActor* actor,
                                                 const PlayerConst* playerConst,
                                                 const IUsePlayerCollision* collision)
    : HackerJudge(hacker), mActor(actor), mPlayerConst(playerConst), mCollision(collision) {}

void YoshiJudgeStartJumpFlap::reset() {
    mIsStarted = false;
}

void YoshiJudgeStartJumpFlap::update() {
    if (mIsStarted && rs::isOnGroundRunAngle(mActor, mCollision, mPlayerConst))
        mIsStarted = false;
}

bool YoshiJudgeStartJumpFlap::judge() const {
    if (mIsStarted || al::calcSpeedV(mActor) >= 0.0f)
        return false;
    if (rs::isTriggerHackJump(*getHacker()))
        return true;
    return rs::isHoldHackJump(*getHacker());
}
