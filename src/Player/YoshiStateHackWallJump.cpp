#include "Player/YoshiStateHackWallJump.h"

#include <basis/seadTypes.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerTrigger.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackInputFunction.h"
#include "Util/YoshiUtil.h"

namespace {
class PlayerActionAirMoveControlOverlay {
public:
    u8 _0[0x20];
    IUsePlayerHack** mPlayerHack;
};

NERVE_IMPL(YoshiStateHackWallJump, Jump)
NERVES_MAKE_NOSTRUCT(YoshiStateHackWallJump, Jump)
}  // namespace

YoshiStateHackWallJump::YoshiStateHackWallJump(
    al::LiveActor* actor, IUsePlayerHack** playerHack, const PlayerConst* playerConst,
    const IUsePlayerCollision* collision, const PlayerTrigger* trigger, const YoshiTongue* tongue,
    PlayerAnimator* animator)
    : HackerStateBase("壁ジャンプ", actor, playerHack), mPlayerConst(playerConst),
      mCollision(collision), mTrigger(trigger), mTongue(tongue), mAnimator(animator),
      mAirMoveControl(nullptr) {
    mAirMoveControl = new PlayerActionAirMoveControl(actor, playerConst, nullptr, collision, false);
    reinterpret_cast<PlayerActionAirMoveControlOverlay*>(mAirMoveControl)->mPlayerHack = playerHack;
    initNerve(&Jump, 0);
}

void YoshiStateHackWallJump::appear() {
    HackerStateBase::appear();
    al::setNerve(this, &Jump);
}


// NON_MATCHING: exact target/current size 764/764 but velocity expression allocates S2 where target uses S1 at 0x710049F248; next source-level hypothesis: recover the original sead scale-add expression that preserves target FP register order.
void YoshiStateHackWallJump::exeJump() {
    al::LiveActor* actor = mActor;
    sead::Vector3f up = -al::getGravity(actor);

    if (al::isFirstStep(this)) {
        mAnimator->startAnim("JumpTongue");

        sead::Vector3f front{0.0f, 0.0f, 0.0f};
        al::calcFrontDir(&front, mActor);
        front.negate();
        al::verticalizeVec(&front, up, front);
        if (!al::tryNormalizeOrZero(&front)) {
            al::calcUpDir(&front, actor);
            al::verticalizeVec(&front, up, front);
            al::normalize(&front);
        }

        const sead::Vector3f velocity = front * 15.0f + up * 15.0f;
        al::setVelocity(mActor, velocity);

        sead::Quatf quat = sead::Quatf::unit;
        al::makeQuatFrontUp(&quat, front, up);
        al::updatePoseQuat(mActor, quat);
        al::startHitReaction(actor, "壁ジャンプ");
    }

    sead::Vector3f moveVec{0.0f, 0.0f, 0.0f};
    rs::calcHackerMoveVec(&moveVec, *mPlayerHack, up);
    rs::moveDivingJump(actor, moveVec, 0.2f, 0.5f, 15.0f, 0.0f, 0.0f, 1.0f,
                       mPlayerConst->getFallSpeedMax(), 0.0f);

    sead::Quatf pullQuat = sead::Quatf::unit;
    if (rs::tryCalcTonguePullPose(&pullQuat, actor, mTongue)) {
        sead::Quatf quat = sead::Quatf::unit;
        al::calcQuat(&quat, actor);
        al::slerpQuat(&quat, quat, pullQuat, 0.2f);
        al::updatePoseQuat(actor, quat);
    }

    if (mAnimator->isAnim("JumpTongue") && mAnimator->isAnimEnd())
        mAnimator->startAnim("JumpTongueLoop");

    if (mTrigger->isOnUpperPunchHit())
        rs::reflectCeiling(actor, mPlayerConst->getReflectUpperPunchScaleH());

    if (rs::isCollidedCeiling(mCollision))
        rs::reflectCeiling(mActor, 0.0f);

    if (rs::isOnGround(actor, mCollision))
        kill();
}
