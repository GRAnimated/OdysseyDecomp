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
      mPlayerConst(playerConst) {}

// NON_MATCHING: target/current are 240 bytes with 6/6 semantic calls; spelling the speed gate as !(speed > 0) now reproduces target B.GT/NaN semantics, while the terminal true/false block layout still differs. Next source-level hypothesis: recover the original result-temporary lifetime without growing the CFG.
bool HackCapJudgeHoldHoveringJump::judge() const {
    bool result = false;
    if (rs::isSeparatePlay(mHackCap) && rs::isPlayer2D(mHackCap) &&
        mHackCap->isEnableSpinAttack() && !(al::calcSpeedV(mPlayer) > 0.0f) &&
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

void HackCapJudgeHoldHoveringJump::reset() {}

void HackCapJudgeHoldHoveringJump::update() {}

