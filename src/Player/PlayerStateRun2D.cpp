#include "Player/PlayerStateRun2D.h"

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionFunction.h"
#include "Player/PlayerActionGroundMoveControl.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerJudgeStartDash.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(PlayerStateRun2D, Run)
NERVE_IMPL(PlayerStateRun2D, Brake)
NERVE_IMPL(PlayerStateRun2D, Turn)
NERVES_MAKE_STRUCT(PlayerStateRun2D, Run, Brake, Turn)
}  // namespace

PlayerStateRun2D::PlayerStateRun2D(al::LiveActor* player, const PlayerConst* pConst,
                                   const PlayerInput* input,
                                   const IUsePlayerCollision* collision,
                                   PlayerAnimator* animator)
    : al::ActorStateBase("走り2D", player), mConst(pConst), mInput(input),
      mCollision(collision), mAnimator(animator) {
    auto* moveControl = new PlayerActionGroundMoveControl(player, pConst, input, collision);
    mMoveControl = moveControl;
    auto* judgeStartDash = new PlayerJudgeStartDash(mInput);
    moveControl->initDash(judgeStartDash, mConst->getDashMaxSpeed2D(),
                          mConst->getDashAccelFrame2D());
    mMoveControl->setIs2D(true);
    initNerve(&NrvPlayerStateRun2D.Run, 0);
}

void PlayerStateRun2D::appear() {
    if (mAnimator->isSubAnimPlaying())
        mAnimator->endSubAnim();

    const PlayerConst* pConst = mConst;
    PlayerActionGroundMoveControl* moveControl = mMoveControl;
    moveControl->setup(pConst->getNormalMaxSpeed2D(), pConst->getNormalMinSpeed2D(),
                       pConst->getNormalAccelFrame2D(), pConst->getStickOnBrakeFrame2D(),
                       pConst->getNormalBrakeFrame2D(), pConst->getGravityMove(),
                       pConst->getNormalMaxSpeed2D() * 1.3f, 10);
    moveControl->setupDash(pConst->getDashMaxSpeed2D(), pConst->getDashAccelFrame2D());
    mMoveControl->appear();
    al::setNerve(this, &NrvPlayerStateRun2D.Run);
    al::ActorStateBase::appear();
}

bool PlayerStateRun2D::isBrake() const {
    if (isDead())
        return false;
    return al::isNerve(this, &NrvPlayerStateRun2D.Brake) ||
           al::isNerve(this, &NrvPlayerStateRun2D.Turn);
}


void PlayerStateRun2D::exeRun() {
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("Move");
        mMoveControl->reset(mMoveControl->getGroundNormal());
    }

    f32 speed = mMoveControl->update();
    f32 animSpeedRange = mConst->getDashMaxSpeed2D() - mConst->getNormalMaxSpeed2D();
    f32 maxAnimSpeedRange = mConst->getAnimFrameRateRange2D();
    if (animSpeedRange > maxAnimSpeedRange)
        animSpeedRange = maxAnimSpeedRange;
    f32 speedAboveRun = speed - mConst->getNormalMaxSpeed2D();
    f32 animRate = al::easeOut((speedAboveRun < 0.0f ? 0.0f : speedAboveRun) / animSpeedRange);
    f32 finalAnimRate = al::lerpValue(mConst->getAnimFrameRateMinRun2D(),
                                      mConst->getAnimFrameRateMaxRun2D(), animRate);
    mAnimator->setAnimRate(finalAnimRate);

    if (mMoveControl->isBrake2D())
        al::setNerve(this, &NrvPlayerStateRun2D.Brake);
    else if (mMoveControl->isStopped())
        kill();
}

void PlayerStateRun2D::exeBrake() {
    if (al::isFirstStep(this))
        mAnimator->startAnim("Brake");

    sead::Vector3f moveVelocity = {0.0f, 0.0f, 0.0f};
    mMoveControl->updateNormalAndSnap(&moveVelocity);
    f32 speedSquared = moveVelocity.squaredLength();
    sead::Vector3f groundNormal = mMoveControl->getGroundNormal();
    f32 speed = sead::Mathf::sqrt(speedSquared);
    f32 brakeSpeed = PlayerActionFunction::brake(
        speed, mConst->getNormalBrakeFrame2D(), mConst->getNormalMaxSpeed2D());
    if (brakeSpeed < 0.0f)
        brakeSpeed = 0.0f;

    rs::slerpUp(mActor, groundNormal, mConst->getSlerpQuatRate(),
                mConst->getHillPoseDegreeMax());

    sead::Vector3f moveDir = {0.0f, 0.0f, 0.0f};
    al::tryNormalizeOrZero(&moveDir, moveVelocity);
    al::setVelocity(mActor, moveDir * brakeSpeed - groundNormal * mConst->getGravityMove());

    sead::Vector3f inputDir = {0.0f, 0.0f, 0.0f};
    mInput->calcMoveInput(&inputDir, groundNormal);
    al::tryNormalizeOrZero(&inputDir);
    if (!al::isNearZero(inputDir, 0.001f)) {
        f32 inputDot = moveDir.dot(inputDir);
        if (!(inputDot <= 0.0f)) {
            al::setNerve(this, &NrvPlayerStateRun2D.Run);
            return;
        }
        if (al::isGreaterEqualStep(this, mConst->getBrakeTurnStartFrame2D())) {
            al::setNerve(this, &NrvPlayerStateRun2D.Turn);
            return;
        }
    }

    if (!al::isLessStep(this, mConst->getNormalBrakeFrame2D()))
        kill();
}

// NON_MATCHING: target is 608 bytes while current is 632; next source-level hypothesis is correcting gravity/front vector association and end-turn quaternion/register ordering.
void PlayerStateRun2D::exeTurn() {
    sead::Vector3f moveVelocity = {0.0f, 0.0f, 0.0f};
    mMoveControl->updateNormalAndSnap(&moveVelocity);
    f32 speedSquared = moveVelocity.squaredLength();
    sead::Vector3f groundNormal = mMoveControl->getGroundNormal();
    f32 speed = sead::Mathf::sqrt(speedSquared);
    f32 brakeSpeed = PlayerActionFunction::brake(
        speed, mConst->getNormalBrakeFrame2D(), mConst->getNormalMaxSpeed2D());
    if (brakeSpeed < 0.0f)
        brakeSpeed = 0.0f;

    rs::slerpUp(mActor, groundNormal, mConst->getSlerpQuatRate(),
                mConst->getHillPoseDegreeMax());

    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    f32 gravityMove = -mConst->getGravityMove();
    al::calcFrontDir(&front, mActor);
    al::setVelocity(mActor, groundNormal * gravityMove + front * brakeSpeed);

    if (!al::isLessStep(this, mConst->getNormalBrakeFrame2D())) {
        f32 speedEnd = mConst->getNormalMaxSpeed2D();
        al::setVelocity(mActor, groundNormal * gravityMove -
                                   front * speedEnd * mConst->getTurnEndSpeedRate2D());

        sead::Vector3f up;
        al::calcUpDir(&up, mActor);
        sead::Quatf quat;
        al::makeQuatFrontUp(&quat, -front, up);
        al::updatePoseQuat(mActor, quat);
        rs::resetCollisionPose(mCollision, quat);
        kill();
    }
}

PlayerStateRun2D::~PlayerStateRun2D() = default;
