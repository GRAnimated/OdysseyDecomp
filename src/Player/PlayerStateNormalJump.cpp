#include "Player/PlayerStateNormalJump.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerActionFunction.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerContinuousJump.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(PlayerStateNormalJump, Jump)
NERVES_MAKE_STRUCT(PlayerStateNormalJump, Jump)
}  // namespace
PlayerStateNormalJump::PlayerStateNormalJump(al::LiveActor* player, const PlayerConst* pConst,
                                             const IUsePlayerCollision* collision,
                                             const PlayerInput* input,
                                             PlayerAnimator* animator)
    : al::ActorStateBase("ジャンプ", player), mConst(pConst), mCollision(collision) {
    mAnimator = animator;
    if (input)
        mAirMoveControl = new PlayerActionAirMoveControl(player, pConst, input, collision, false);
    initNerve(&NrvPlayerStateNormalJump.Jump, 0);
}

void PlayerStateNormalJump::initContinuousJump(PlayerContinuousJump* continuousJump) {
    mContinuousJump = continuousJump;
}

// NON_MATCHING: target/current are both 336 bytes; remaining differences are the inlined continuous-jump selector block order and duplicated getter tails. A finite ordinary case-order sweep did not recover the target; next hypothesis is a distinct selector source shape rather than textual case reordering.
void PlayerStateNormalJump::appear() {
    f32 jumpPower = PlayerActionFunction::calcJumpSpeed(
        al::calcSpeedH(mActor), mConst->getNormalMinSpeed(), mConst->getNormalMaxSpeed(),
        calcJumpPowerMin(), calcJumpPowerMax());
    if (mIsForceJump) {
        jumpPower = mForceJumpPower;
        if (mContinuousJump)
            mContinuousJump->clear();
    }
    mJumpPower = jumpPower;
    mIsForceJump = false;
    mForceJumpPower = 0.0f;
    al::NerveStateBase::appear();
    al::setNerve(this, &NrvPlayerStateNormalJump.Jump);
}

// NON_MATCHING: target/current are both 80 bytes; target compares count 2 then 1 before the zero/default split, while current hoists count 0. Ordinary case-order permutations did not recover the target; next hypothesis is a selector form that preserves the shared 1/2 getter after both comparisons.
f32 PlayerStateNormalJump::calcJumpPowerMin() {
    if (!mContinuousJump)
        return mConst->getJumpPowerMin();

    switch (mContinuousJump->getCount()) {
    case 1:
        return mConst->getContinuousJumpPowerMin();
    case 2:
        return mConst->getContinuousJumpPowerMin();
    case 0:
        return mConst->getJumpPowerMin();
    default:
        return mConst->getJumpPowerMax();
    }
}

// NON_MATCHING: target/current are both 76 bytes; target compares count 2 then 1, while current lowers the same switch with a different branch target layout. Ordinary case-order permutations were ineffective; next hypothesis is a branch-local selector form.
f32 PlayerStateNormalJump::calcJumpPowerMax() {
    if (!mContinuousJump)
        return mConst->getJumpPowerMax();

    switch (mContinuousJump->getCount()) {
    case 1:
        return mConst->getJumpPowerMax2nd();
    case 2:
        return mConst->getJumpPowerMax3rd();
    default:
        return mConst->getJumpPowerMax();
    }
}

void PlayerStateNormalJump::kill() {
    if (isDead()) {
        mIsForceJump = false;
        mForceJumpPower = 0.0f;
    }
    al::NerveStateBase::kill();
}

// NON_MATCHING: exact size; first divergent branch lands 0x20 late because the inlined gravity selector block order differs.
void PlayerStateNormalJump::exeJump() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        if (mAnimator)
            mAnimator->startAnim(calcJumpAnimName());
        else
            al::startAction(actor, calcJumpAnimName());

        if (mAirMoveControl)
            mAirMoveControl->setup(100.0f, mConst->getJumpMoveSpeedMin(),
                                   mConst->getExtendFrame(), mJumpPower, calcJumpGravity(), 0,
                                   mConst->getJumpInertiaRate());
    }

    if (mAirMoveControl)
        mAirMoveControl->update();

    bool isOnGround =
        mCollision ? rs::isOnGround(actor, mCollision) : al::isOnGround(actor, 0);
    if (isOnGround)
        kill();
}

const char* PlayerStateNormalJump::calcJumpAnimName() {
    if (!mContinuousJump)
        return "Jump";

    u32 count = mContinuousJump->getCount();
    if (count != 2) {
        if (count != 1)
            return "Jump";
        return "Jump2";
    }

    if (mJumpPower < mConst->getJumpPowerMax3rd() * 0.99f) {
        mContinuousJump->clear();
        return "Jump";
    }
    return "Jump3";
}

// NON_MATCHING: target/current are both 76 bytes; target compares count 2 then 1, while current lowers the same switch with a different branch target layout. Ordinary case-order permutations were ineffective; next hypothesis is a branch-local selector form.
f32 PlayerStateNormalJump::calcJumpGravity() {
    if (!mContinuousJump)
        return mConst->getJumpGravity();

    switch (mContinuousJump->getCount()) {
    case 1:
        return mConst->getJumpGravity2nd();
    case 2:
        return mConst->getJumpGravity3rd();
    default:
        return mConst->getJumpGravity();
    }
}

bool PlayerStateNormalJump::sendMsgUpperPunch() {
    return false;
}

PlayerStateNormalJump::~PlayerStateNormalJump() = default;

