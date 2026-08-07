#include "Player/HackCapJudgeHoldHoveringJump.h"

#include "Library/LiveActor/ActorMovementFunction.h"

#include "Player/HackCap.h"
#include "Player/PlayerColliderHakoniwa.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerInputFunction.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerUtil.h"

HackCapJudgeHoldHoveringJump::HackCapJudgeHoldHoveringJump(const al::LiveActor* player,
                                                           const PlayerColliderHakoniwa* collider,
                                                           const HackCap* hackCap,
                                                           const PlayerInput* input,
                                                           const PlayerConst* playerConst)
    : mPlayer(player), mCollider(collider), mHackCap(hackCap), mInput(input),
      mPlayerConst(playerConst) {
    return;
}

void HackCapJudgeHoldHoveringJump::reset() {
    return;
}

void HackCapJudgeHoldHoveringJump::update() {
    return;
}

// NON_MATCHING: exact 240-byte size, but the terminal true/false block layout differs at 0x40AF20; next try a differently scoped boolean temporary while preserving the outer condition chain.
bool HackCapJudgeHoldHoveringJump::judge() const {
    bool result = false;
    if (rs::isSeparatePlay(mHackCap) && rs::isPlayer2D(mHackCap) &&
        mHackCap->isEnableSpinAttack() && al::calcSpeedV(mPlayer) <= 0.0f &&
        mInput->isHoldCapSeparateJump()) {
        if (mCollider->isEnableStandUp()) {
            result = true;
        } else if (!rs::isCollidedGround(mCollider)) {
            if (!mCollider->isAboveGround()) {
                result = true;
            } else if (mCollider->getCeilCheckHeight() + mCollider->getGroundHeight() >=
                       mPlayerConst->getTall()) {
                result = true;
            }
        }
    }
    return result;
}
