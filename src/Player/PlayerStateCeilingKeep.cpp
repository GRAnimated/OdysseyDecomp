#include "Player/PlayerStateCeilingKeep.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
NERVE_IMPL(PlayerStateCeilingKeep, Keep)
NERVE_IMPL(PlayerStateCeilingKeep, Leave)
NERVES_MAKE_NOSTRUCT(PlayerStateCeilingKeep, Keep, Leave)
}  // namespace

PlayerStateCeilingKeep::PlayerStateCeilingKeep(al::LiveActor* player)
    : al::ActorStateBase("", player) {
    initNerve(&Keep, 0);
}

void PlayerStateCeilingKeep::appear() {
    al::NerveStateBase::appear();
    mCeilingNormal = al::getCollidedCeilingNormal(mActor);
    al::setNerve(this, &Keep);
}

void PlayerStateCeilingKeep::exeKeep() {
    if (al::isFirstStep(this)) {
        al::startAction(mActor, "Wait");
        al::calcFrontDir(&mFront, mActor);
        al::setVelocityZero(mActor);
    }

    if (!al::isCollidedCeiling(mActor)) {
        al::setNerve(this, &Leave);
        return;
    }

    f32 velocityAlongNormal = al::getVelocity(mActor).dot(mCeilingNormal);
    if (velocityAlongNormal < 0.0f)
        *al::getVelocityPtr(mActor) -= velocityAlongNormal * mCeilingNormal;

    al::addVelocityToDirection(mActor, -mCeilingNormal, 20.0f);
    al::rotateAndKeepColliderPosRate(mActor, mCeilingNormal, mFront, 0.2f);

    if (!al::isLessEqualStep(this, 30)) {
        al::setVelocityZero(mActor);
        al::setNerve(this, &Leave);
    }
}

void PlayerStateCeilingKeep::exeLeave() {
    al::addVelocityToGravity(mActor, 1.0f);

    sead::Quatf targetQuat;
    al::makeQuatUpFront(&targetQuat, -al::getGravity(mActor), mFront);
    al::slerpQuat(al::getQuatPtr(mActor), al::getQuat(mActor), targetQuat, 0.2f);

    sead::Vector3f up;
    al::calcUpDir(&up, mActor);
    if (al::isNearAngleDegree(-up, al::getGravity(mActor), 5.0f)) {
        al::slerpQuat(al::getQuatPtr(mActor), al::getQuat(mActor), targetQuat, 1.0f);
        kill();
    }
}

PlayerStateCeilingKeep::~PlayerStateCeilingKeep() = default;
