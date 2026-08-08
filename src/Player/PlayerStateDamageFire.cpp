#include "Player/PlayerStateDamageFire.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerActionGroundMoveControl.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerFunction.h"
#include "Player/PlayerInput.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(PlayerStateDamageFire, Jump)
NERVE_IMPL(PlayerStateDamageFire, Dead)
NERVE_IMPL(PlayerStateDamageFire, Run)
NERVE_IMPL(PlayerStateDamageFire, Fall)
NERVE_IMPL(PlayerStateDamageFire, Jump2nd)
NERVES_MAKE_STRUCT(PlayerStateDamageFire, Jump, Dead, Run, Fall)
NERVES_MAKE_NOSTRUCT(PlayerStateDamageFire, Jump2nd)
}  // namespace

PlayerStateDamageFire::PlayerStateDamageFire(al::LiveActor* player, const PlayerConst* pConst,
                                             const IUsePlayerCollision* collision,
                                             const PlayerInput* input, PlayerAnimator* animator,
                                             IJudge* judgeSpeedCheckFall)
    : al::ActorStateBase("ダメージ", player), mConst(pConst), mCollision(collision), mInput(input),
      mAirMoveControl(nullptr), mGroundMoveControl(nullptr), mAnimator(animator),
      mJudgeSpeedCheckFall(judgeSpeedCheckFall), mIsDamageFireGround(false),
      mIsDamageFireWall(false), mIsDamageFireCeiling(false), mControlCounter(0), _60(0) {
    mAirMoveControl = new PlayerActionAirMoveControl(player, pConst, input, collision, false);
    mGroundMoveControl = new PlayerActionGroundMoveControl(player, mConst, mInput, mCollision);
    mGroundMoveControl->_bb = true;
    mGroundMoveControl->_b8 = true;
    mGroundMoveControl->_b9 = true;
    initNerve(&NrvPlayerStateDamageFire.Jump, 0);
}

void PlayerStateDamageFire::appear() {
    al::ActorStateBase::appear();

    mGroundMoveControl->setup(mConst->getDamageFireRunSpeed(), mConst->getDamageFireRunSpeed(), 1,
                              9999, 9999, mConst->getGravityMove(), 10000.0f, 10000);
    mControlCounter = 0;

    if (PlayerFunction::isPlayerDeadStatus(mActor)) {
        al::setVelocityZero(mActor);
        al::setNerve(this, &NrvPlayerStateDamageFire.Dead);
        return;
    }

    if (rs::isJustLand(mCollision)) {
        sead::Vector3f velocityH(0.0f, 0.0f, 0.0f);
        const sead::Vector3f& velocity = al::getVelocity(mActor);
        {
            const sead::Vector3f negGravity = -al::getGravity(mActor);
            al::alongVectorNormalH(&velocityH, velocity, negGravity,
                                   rs::getCollidedGroundNormal(mCollision));
        }
        al::LiveActor* actor = mActor;
        const sead::Vector3f& groundNormal = rs::getCollidedGroundNormal(mCollision);
        al::setVelocity(actor, velocityH - mConst->getGravity() * groundNormal);
    }

    mIsDamageFireGround = rs::isCollisionCodeDamageFireGround(mCollision);
    mIsDamageFireWall = rs::isCollisionCodeDamageFireWall(mCollision);
    mIsDamageFireCeiling = rs::isCollisionCodeDamageFireCeiling(mCollision);
    al::setNerve(this, &NrvPlayerStateDamageFire.Jump);
}

void PlayerStateDamageFire::control() {
    if (al::isNerve(this, &NrvPlayerStateDamageFire.Run) ||
        al::isNerve(this, &NrvPlayerStateDamageFire.Fall)) {
        if (mControlCounter >= mConst->getDamageFireRunTime())
            kill();
        mControlCounter++;
    }
}

bool PlayerStateDamageFire::isEndFirstLand() const {
    return !al::isNerve(this, &NrvPlayerStateDamageFire.Jump);
}

bool PlayerStateDamageFire::isEnableJump() const {
    return al::isNerve(this, &NrvPlayerStateDamageFire.Run);
}

bool PlayerStateDamageFire::isEnablePeachAmiibo() const {
    return isDead() || !al::isNerve(this, &NrvPlayerStateDamageFire.Jump);
}

s32 PlayerStateDamageFire::getEnableCancelCollisionSnapFrame() const {
    return 20;
}

void PlayerStateDamageFire::exeJump2nd() {
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("FireRunStart");
        al::startHitReaction(mActor, "あちちジャンプ(バウンド)");

        sead::Vector3f groundNormal(0.0f, 0.0f, 0.0f);
        rs::calcGroundNormalOrGravityDir(&groundNormal, mActor, mCollision);

        sead::Vector3f moveDir(0.0f, 0.0f, 0.0f);
        mInput->calcMoveDirection(&moveDir, groundNormal);
        if (!al::isNearZero(moveDir, 0.001f)) {
            sead::Vector3f velocityVertical(0.0f, 0.0f, 0.0f);
            sead::Vector3f velocityParallel(0.0f, 0.0f, 0.0f);
            al::separateVectorParallelVertical(&velocityParallel, &velocityVertical, groundNormal,
                                               al::getVelocity(mActor));
            al::setVelocity(mActor, velocityParallel + velocityVertical.length() * moveDir);
        }

        mAirMoveControl->setup(mConst->getDamageFireJumpMoveSpeed(),
                               mConst->getDamageFireJumpMoveSpeed(), 0,
                               mConst->getDamageFireJumpPower2nd(), mConst->getDamageFireGravity(), 0,
                               mConst->getJumpInertiaRate());
    }

    mAirMoveControl->update();
    if (rs::isOnGround(mActor, mCollision)) {
        mControlCounter = 0;
        mGroundMoveControl->appear();
        al::setNerve(this, &NrvPlayerStateDamageFire.Run);
    }
}

void PlayerStateDamageFire::exeFall() {
    if (al::isFirstStep(this)) {
        mAirMoveControl->setup(mConst->getDamageFireJumpMoveSpeed(),
                               mConst->getDamageFireJumpMoveSpeed(), 0, 0.0f,
                               mConst->getGravityAir(), 0, mConst->getJumpInertiaRate());
    }

    mAirMoveControl->update();
    if (rs::isOnGround(mActor, mCollision)) {
        mGroundMoveControl->appear();
        al::setNerve(this, &NrvPlayerStateDamageFire.Run);
    }
}

void PlayerStateDamageFire::exeDead() {
    if (al::isFirstStep(this)) {
        if (mAnimator->isSubAnimPlaying())
            mAnimator->endSubAnim();
        mAnimator->startAnim("DeadFire");
        al::LiveActor* actor = mActor;
        rs::slerpUp(actor, -al::getGravity(actor), 1.0f, 0.0f);
        rs::faceToCamera(actor);
    }

    if (mAnimator->isAnimEnd())
        kill();
}

PlayerStateDamageFire::~PlayerStateDamageFire() = default;
