#include "Npc/ShibakenStateJump.h"

#include <math/seadVector.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/Shibaken.h"
#include "Npc/ShibakenFunction.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(ShibakenStateJump, JumpStart);
NERVE_IMPL(ShibakenStateJump, JumpEnd);
NERVE_IMPL(ShibakenStateJump, KillByDeathArea);
NERVE_IMPL(ShibakenStateJump, Jump);
NERVES_MAKE_NOSTRUCT(ShibakenStateJump, JumpStart, JumpEnd, KillByDeathArea, Jump);
}  // namespace

ShibakenStateJump::ShibakenStateJump(const char* name, Shibaken* shibaken)
    : al::NerveStateBase(name) {
    mShibaken = shibaken;
    initNerve(&JumpStart, 0);
}

void ShibakenStateJump::appear() {
    al::NerveStateBase::appear();
    al::setNerve(this, &JumpStart);
}

bool ShibakenStateJump::update() {
    if (!al::isNerve(this, &JumpEnd)) {
        if (!al::isNerve(this, &JumpStart) || !al::isLessEqualStep(this, 1)) {
            if (al::isOnGround(mShibaken, 0) &&
                ShibakenFunction::isGroundNormal(al::getCollidedGroundNormal(mShibaken), mShibaken))
                al::setNerve(this, &JumpEnd);
        }
    }
    return al::NerveStateBase::update();
}

bool ShibakenStateJump::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                   al::HitSensor* self) {
    if (!al::isSensorEnemyBody(self))
        return false;
    if (rs::isMsgNpcCapReactionAll(msg) || rs::isMsgKillerAttackNoExplode(msg) ||
        rs::isMsgSphinxRideAttackReflect(msg) || rs::isMsgTankBulletNoReaction(msg))
        return true;
    return rs::checkMsgNpcTrampleReactionAll(msg, other, self, true);
}

bool ShibakenStateJump::isKillByDeathArea() const {
    return al::isNerve(this, &KillByDeathArea);
}

void ShibakenStateJump::exeJumpStart() {
    if (al::isFirstStep(this)) {
        al::startAction(mShibaken, "JumpStart");

        sead::Vector3f sideDir = {0, 0, 0};
        al::calcSideDir(&sideDir, mShibaken);
        al::verticalizeVec(&sideDir, sead::Vector3f::ey, sideDir);
        if (al::tryNormalizeOrZero(&sideDir))
            al::makeQuatSideUp(al::getQuatPtr(mShibaken), sideDir, sead::Vector3f::ey);

        sead::Vector3f frontDir = {0, 0, 0};
        al::calcFrontDir(&frontDir, mShibaken);

        sead::Vector3f hVel = {al::getVelocity(mShibaken).x, 0.0f, al::getVelocity(mShibaken).z};
        f32 speed = hVel.length();
        frontDir *= speed;

        Shibaken* host = mShibaken;
        f32 jumpSpeedV = ShibakenFunction::getJumpStartSpeedV(host);
        sead::Vector3f newVel = {sead::Vector3f::ey.x * jumpSpeedV + frontDir.x,
                                 sead::Vector3f::ey.y * jumpSpeedV + frontDir.y,
                                 sead::Vector3f::ey.z * jumpSpeedV + frontDir.z};
        al::setVelocity(host, newVel);
    }

    Shibaken* host = mShibaken;
    al::addVelocityToGravity(host, ShibakenFunction::getJumpGravityAccel(host));
    f32 friction = ShibakenFunction::getJumpAirFriction(host);
    al::scaleVelocityHV(host, 1.0f, friction);

    sead::Vector3f upDir = {0, 0, 0};
    al::calcUpDir(&upDir, host);
    al::lerpVec(&upDir, upDir, sead::Vector3f::ey, 0.1f);
    al::normalize(&upDir);

    sead::Vector3f frontDir = {0, 0, 0};
    al::calcFrontDir(&frontDir, host);
    al::verticalizeVec(&frontDir, upDir, frontDir);

    if (al::isInDeathArea(mShibaken))
        al::setNerve(this, &KillByDeathArea);
    else if (al::isActionEnd(mShibaken))
        al::setNerve(this, &Jump);
}

void ShibakenStateJump::exeJump() {
    if (al::isFirstStep(this))
        al::startAction(mShibaken, "Jump");

    Shibaken* host = mShibaken;
    al::addVelocityToGravity(host, ShibakenFunction::getJumpGravityAccel(host));
    f32 friction = ShibakenFunction::getJumpAirFriction(host);
    al::scaleVelocityHV(host, 1.0f, friction);

    sead::Vector3f upDir = {0, 0, 0};
    al::calcUpDir(&upDir, host);
    al::lerpVec(&upDir, upDir, sead::Vector3f::ey, 0.1f);
    al::normalize(&upDir);

    sead::Vector3f frontDir = {0, 0, 0};
    al::calcFrontDir(&frontDir, host);
    al::verticalizeVec(&frontDir, upDir, frontDir);

    if (al::isInDeathArea(mShibaken))
        al::setNerve(this, &KillByDeathArea);
}

void ShibakenStateJump::exeJumpEnd() {
    if (al::isFirstStep(this)) {
        al::startAction(mShibaken, "JumpEnd");
        ShibakenFunction::limitFallVelocityOnGround(mShibaken);

        sead::Vector3f upDir = {0, 0, 0};
        al::calcUpDir(&upDir, mShibaken);

        const sead::Vector3f& groundNormal = al::getCollidedGroundNormal(mShibaken);
        al::parallelizeVec(&upDir, groundNormal, upDir);
        if (!al::tryNormalizeOrZero(&upDir))
            al::calcUpDir(&upDir, mShibaken);

        sead::Vector3f frontDir = {0, 0, 0};
        al::calcFrontDir(&frontDir, mShibaken);
        al::verticalizeVec(&frontDir, upDir, frontDir);
        if (!al::tryNormalizeOrZero(&frontDir))
            al::calcFrontDir(&frontDir, mShibaken);

        al::makeQuatFrontUp(al::getQuatPtr(mShibaken), frontDir, upDir);
    }

    ShibakenFunction::addFallVelocityToGround(mShibaken, 0.95f);

    if (al::isActionEnd(mShibaken))
        kill();
}

void ShibakenStateJump::exeKillByDeathArea() {
    al::startHitReaction(mShibaken, "消滅");
    kill();
}
