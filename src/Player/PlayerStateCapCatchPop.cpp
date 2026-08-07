#include "Player/PlayerStateCapCatchPop.h"

#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/HackCap.h"
#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"

namespace {
NERVE_IMPL(PlayerStateCapCatchPop, Pop)
NERVES_MAKE_STRUCT(PlayerStateCapCatchPop, Pop)
}  // namespace

PlayerStateCapCatchPop::PlayerStateCapCatchPop(al::LiveActor* player, const PlayerConst* pConst,
                                               const IUsePlayerCollision* collision,
                                               const PlayerInput* input,
                                               PlayerAnimator* animator, HackCap* hackCap)
    : al::ActorStateBase("", player), mConst(pConst), mCollision(collision), mAnimator(animator),
      mAirMoveControl(nullptr), mHackCap(hackCap) {
    mAirMoveControl = new PlayerActionAirMoveControl(player, pConst, input, collision, false);
    initNerve(&NrvPlayerStateCapCatchPop.Pop, 0);
}

PlayerStateCapCatchPop::~PlayerStateCapCatchPop() = default;

void PlayerStateCapCatchPop::appear() {
    al::NerveStateBase::appear();
    al::setNerve(this, &NrvPlayerStateCapCatchPop.Pop);
}

void PlayerStateCapCatchPop::exePop() {
    if (al::isFirstStep(this)) {
        if (mAnimator->isSubAnimPlaying())
            mAnimator->endSubAnim();
        mAnimator->startAnim("JumpCapCatchAir");
        mHackCap->startCatch("JumpCapCatch", false, sead::Vector3f::zero);
        mAirMoveControl->setup(mConst->getSpinAirSpeedMax(), mConst->getSpinAirSpeedMax(), 0,
                               mConst->getCapCatchPopPower(), mConst->getCapCatchPopGravity(), 0,
                               0.0f);
    }

    mAirMoveControl->update();
    if (mAnimator->isAnimEnd() && !al::isLessStep(this, 30))
        kill();
}
