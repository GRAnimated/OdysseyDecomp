#include "Npc/FrogStateWaitWander.h"

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveExecutor.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
NERVE_IMPL(FrogStateWaitWander, Wait);
NERVE_IMPL(FrogStateWaitWander, Walk);
NERVE_IMPL(FrogStateWaitWander, Fall);

NERVES_MAKE_NOSTRUCT(FrogStateWaitWander, Wait, Walk, Fall);
}  // namespace

FrogStateWaitWander::FrogStateWaitWander(al::LiveActor* actor)
    : al::ActorStateBase(u8"状態:徘徊", actor) {
    initNerve(&Wait, 0);
}

void FrogStateWaitWander::appear() {
    mIsDead = false;
    mWalkCounter = 0;
    f32 r = al::getRandom() < 0.5f ? 1.0f : -1.0f;
    al::setNerve(this, r < 0.0f ? (const al::Nerve*)&Walk : &Wait);
}

void FrogStateWaitWander::exeWait() {
    if (al::isFirstStep(this)) {
        al::tryStartActionIfNotPlaying(mActor, "Wait");
        if (al::isOnGround(mActor, 3)) {
            sead::Vector3f groundNormal = al::getCollidedGroundNormal(mActor);
            sead::Vector3f frontDir;
            al::calcFrontDir(&frontDir, mActor);
            al::makeQuatUpFront(al::getQuatPtr(mActor), groundNormal, frontDir);
        }
        mWaitFrames = al::getRandom(150, 210);
    }

    if (al::isGreaterEqualStep(this, mWaitFrames)) {
        mWalkCounter = 0;
        al::setNerve(this, &Walk);
        return;
    }

    if (!al::isOnGround(mActor, 3)) {
        al::setNerve(this, &Fall);
        return;
    }

    al::LiveActor* actor = mActor;
    if (al::isFallNextMove(actor, al::getVelocity(actor), 50.0f, 200.0f)) {
        al::scaleVelocity(mActor, -1.0f);
        al::setVelocityY(mActor, 0.0f);
    }
    al::scaleVelocity(mActor, 0.7f);
    al::addVelocityToGravityFittedGround(mActor, 2.0f, 3);
}

void FrogStateWaitWander::exeWalk() {
    if (al::isFirstStep(this))
        al::tryStartActionIfNotPlaying(mActor, "Run");

    s32 count = mWalkCounter;
    mWalkCounter = count + 1;
    if (count >= 150) {
        al::setNerve(this, &Wait);
        return;
    }

    if (!al::isOnGround(mActor, 3)) {
        al::setNerve(this, &Fall);
        return;
    }

    if (al::isActionEnd(mActor)) {
        al::startAction(mActor, "Run");
        al::setNerve(this, &Walk);
        return;
    }

    al::rotateQuatYDirDegree(mActor, 1.0f);

    sead::Vector3f frontDir = sead::Vector3f::zero;
    al::calcFrontDir(&frontDir, mActor);

    al::LiveActor* actor = mActor;
    const sead::Vector3f& vel = al::getVelocity(actor);
    sead::Vector3f nextVel = {frontDir.x + frontDir.x + vel.x,
                              frontDir.y + frontDir.y + vel.y,
                              frontDir.z + frontDir.z + vel.z};

    if (al::isFallNextMove(actor, nextVel, 50.0f, 200.0f)) {
        al::scaleVelocity(mActor, -1.0f);
        al::setVelocityY(mActor, 0.0f);
    } else {
        al::addVelocityToDirection(mActor, frontDir, 2.0f);
    }
    al::scaleVelocity(mActor, 0.7f);
    al::addVelocityToGravityFittedGround(mActor, 2.0f, 3);
}

void FrogStateWaitWander::exeFall() {
    if (al::isFirstStep(this))
        al::startAction(mActor, "Fall");
    if (al::isOnGround(mActor, 3)) {
        al::setNerve(this, &Wait);
        return;
    }
    al::scaleVelocity(mActor, 0.98f);
    al::addVelocityToGravity(mActor, 2.0f);
}
