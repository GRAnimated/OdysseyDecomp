#include "Npc/FrogStateWaitJump.h"

#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveStateBase.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/Frog.h"

namespace {
NERVE_IMPL(FrogStateWaitJump, Wait);
NERVE_HOST_TYPE_IMPL(FrogStateWaitJump, JumpStart);
NERVE_IMPL(FrogStateWaitJump, Turn);
NERVE_IMPL(FrogStateWaitJump, Jump);
NERVE_IMPL(FrogStateWaitJump, Land);
NERVE_END_IMPL(FrogStateWaitJump, Fall);

struct {
    FrogStateWaitJumpNrvWait Wait;
    HostTypeNrvJumpStart JumpStart;
    FrogStateWaitJumpNrvTurn Turn;
    FrogStateWaitJumpNrvJump Jump;
    FrogStateWaitJumpNrvLand Land;
    FrogStateWaitJumpNrvFall Fall;
} NrvFrogStateWaitJump;

void updateJumpRotation(Frog* frog, f32 jumpSpeed) {
    sead::Vector3f velDir = al::getVelocity(frog);
    if (!al::tryNormalizeOrZero(&velDir)) {
        al::calcQuat(&frog->mJointQuat, frog);
        return;
    }

    f32 speedV = al::calcSpeedV(frog);
    if (speedV > 0.0f) {
        al::calcQuat(&frog->mJointQuat, frog);
        return;
    }

    f32 rate = sead::Mathf::clamp(sead::Mathf::abs(speedV / jumpSpeed), 0.0f, 0.6f);

    sead::Quatf currentQuat;
    al::calcQuat(&currentQuat, frog);

    sead::Quatf resultQuat = sead::Quatf::unit;
    al::turnQuatZDirRate(&resultQuat, currentQuat, velDir, rate);
    al::slerpQuat(&frog->mJointQuat, frog->mJointQuat, resultQuat, 0.1f);
}
}  // namespace

FrogStateWaitJump::FrogStateWaitJump(Frog* frog)
    : al::NerveStateBase(u8"状態:徘徊"), mActor(frog), mWaitFrames(240), mTurnAngle(0.0f),
      mJumpSpeed(0.0f) {
    initNerve(&NrvFrogStateWaitJump.Wait, 0);
}

void FrogStateWaitJump::appear() {
    al::NerveStateBase::appear();
    if (al::isOnGround(mActor, 3)) {
        f32 r = al::getRandom() < 0.5f ? 1.0f : -1.0f;
        al::setNerve(this, r < 0.0f ? (const al::Nerve*)&NrvFrogStateWaitJump.JumpStart :
                                      &NrvFrogStateWaitJump.Wait);
    } else {
        al::setNerve(this, &NrvFrogStateWaitJump.Fall);
    }
}

// NON_MATCHING: regalloc — target caches mActor in x20, we reload each time
void FrogStateWaitJump::kill() {
    mActor->offDynamics();
    al::calcQuat(&mActor->mJointQuat, mActor);
    mActor->mJointController->mIsControl = false;
    al::setColliderOffsetY(mActor, al::getColliderRadius(mActor));
    al::NerveStateBase::kill();
}

// NON_MATCHING: regalloc — target caches mActor in x20, we reload each time
void FrogStateWaitJump::endFall() {
    mActor->offDynamics();
    al::calcQuat(&mActor->mJointQuat, mActor);
    mActor->mJointController->mIsControl = false;
    al::setColliderOffsetY(mActor, al::getColliderRadius(mActor));
}

void FrogStateWaitJump::exeWait() {
    if (al::isFirstStep(this)) {
        al::validateClipping(mActor);
        al::tryStartActionIfNotPlaying(mActor, "Wait");
        if (al::isOnGround(mActor, 3)) {
            sead::Vector3f groundNormal = al::getCollidedGroundNormal(mActor);
            sead::Vector3f frontDir;
            al::calcFrontDir(&frontDir, mActor);
            al::makeQuatUpFront(al::getQuatPtr(mActor), groundNormal, frontDir);
        }
        mWaitFrames = al::getRandom(180, 300);
    }

    if (al::isGreaterEqualStep(this, mWaitFrames)) {
        al::setNerve(this, &NrvFrogStateWaitJump.Turn);
        return;
    }

    if (!al::isOnGround(mActor, 3))
        al::setNerve(this, &NrvFrogStateWaitJump.Fall);
}

// NON_MATCHING: regswap x20/x21
void FrogStateWaitJump::exeTurn() {
    if (al::isFirstStep(this)) {
        if (al::getRandom() >= 0.5f) {
            al::setNerve(this, &NrvFrogStateWaitJump.JumpStart);
            return;
        }
        mTurnAngle = al::getRandom(-60.0f, 60.0f);
        al::startAction(mActor, "Turn");
    }

    f32 angle = mTurnAngle;
    al::rotateQuatYDirDegree(mActor, angle / al::getActionFrameMax(mActor, "Turn"));

    if (al::isActionEnd(mActor))
        al::setNerve(this, &NrvFrogStateWaitJump.JumpStart);
}

void FrogStateWaitJump::exeJumpStart() {
    if (al::isFirstStep(this)) {
        al::invalidateClipping(mActor);
        al::startAction(mActor, "NpcJumpStart");
    }

    if (al::isActionEnd(mActor)) {
        mActor->onDynamics();
        al::setNerve(this, &NrvFrogStateWaitJump.Jump);
    } else if (!al::isOnGround(mActor, 3)) {
        al::setNerve(this, &NrvFrogStateWaitJump.Fall);
    }
}

// NON_MATCHING: regalloc — target caches mActor in x20, we reload each time
void FrogStateWaitJump::exeJump() {
    if (al::isFirstStep(this)) {
        al::setColliderOffsetY(mActor, 0.0f);
        al::startAction(mActor, "NpcJump");
        mJumpSpeed = al::getRandom() < 0.7f ? 65.0f : 40.0f;
        al::addVelocityJump(mActor, mJumpSpeed);
        mActor->mJointController->mIsControl = true;
        al::calcQuat(&mActor->mJointQuat, mActor);
    }

    mActor->updateVelocity();
    updateJumpRotation(mActor, mJumpSpeed);

    const sead::Vector3f& vel = al::getVelocity(mActor);
    const sead::Vector3f& gravity = al::getGravity(mActor);
    f32 dot = vel.x * gravity.x + vel.y * gravity.y + vel.z * gravity.z;

    if (dot > 0.0f) {
        if (al::isOnGround(mActor, 3)) {
            al::startAction(mActor, "NpcLand");
            al::startHitReaction(mActor, u8"着地");
            mActor->offDynamics();
            al::setNerve(this, &NrvFrogStateWaitJump.Land);
        } else if (al::isActionPlaying(mActor, "NpcJump") && al::isActionEnd(mActor)) {
            al::setNerve(this, &NrvFrogStateWaitJump.Fall);
        }
    }
}

// NON_MATCHING: regalloc — target caches mActor in x20, we reload each time
void FrogStateWaitJump::exeFall() {
    if (al::isFirstStep(this))
        al::startAction(mActor, "NpcFall");

    mActor->updateVelocity();
    updateJumpRotation(mActor, mJumpSpeed);

    if (al::isOnGround(mActor, 3)) {
        al::startAction(mActor, "NpcLand");
        al::startHitReaction(mActor, u8"着地");
        mActor->offDynamics();
        al::setNerve(this, &NrvFrogStateWaitJump.Land);
    }
}

void FrogStateWaitJump::exeLand() {
    mActor->updateVelocity();
    if (al::isActionEnd(mActor))
        al::setNerve(this, &NrvFrogStateWaitJump.Wait);
}
