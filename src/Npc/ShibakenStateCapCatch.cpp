#include "Npc/ShibakenStateCapCatch.h"

#include <math/seadMathCalcCommon.h>
#include <math/seadVector.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/Shibaken.h"
#include "Npc/ShibakenFunction.h"
#include "Npc/ShibakenStateJump.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(ShibakenStateCapCatch, Catch);
NERVE_IMPL(ShibakenStateCapCatch, Jump);
NERVE_IMPL(ShibakenStateCapCatch, Chase);
NERVE_IMPL(ShibakenStateCapCatch, Back);
NERVES_MAKE_NOSTRUCT(ShibakenStateCapCatch, Catch, Jump, Chase, Back);
}  // namespace

// NON_MATCHING: store ordering of _28 and _2c relative to STP is swapped
ShibakenStateCapCatch::ShibakenStateCapCatch(const char* name, Shibaken* shibaken)
    : al::NerveStateBase(name) {
    mShibaken = shibaken;
    initNerve(&Catch, 1);
    mStateJump = new ShibakenStateJump("ジャンプ", mShibaken);
    al::initNerveState(this, mStateJump, &Jump, "ジャンプ");
}

void ShibakenStateCapCatch::appear() {
    al::NerveStateBase::appear();
    _2c = 0;
}

void ShibakenStateCapCatch::kill() {
    al::NerveStateBase::kill();
    _28 = false;
}

bool ShibakenStateCapCatch::tryStart() {
    sead::Vector3f capPos = {0.0f, 0.0f, 0.0f};
    if (!rs::tryGetFlyingCapPos(&capPos, mShibaken))
        return false;
    al::setNerve(this, &Chase);
    return true;
}

bool ShibakenStateCapCatch::tryStartByReceiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                                 al::HitSensor* self) {
    if (!rs::isMsgCapStartLockOn(msg))
        return false;

    sead::Vector3f capPos = {0.0f, 0.0f, 0.0f};
    if (!rs::tryGetFlyingCapPos(&capPos, mShibaken))
        return false;

    const sead::Vector3f& playerPos = rs::getPlayerPos(mShibaken);
    f32 dx = capPos.x - playerPos.x;
    f32 dz = capPos.z - playerPos.z;
    f32 hDist = sead::Mathf::sqrt(dx * dx + dz * dz);
    if (hDist < 300.0f)
        return false;

    _28 = true;
    if (!al::isNerve(this, &Jump))
        al::setNerve(this, &Catch);
    return true;
}

// NON_MATCHING: regswaps (x19/x20 for this/msg)
bool ShibakenStateCapCatch::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                       al::HitSensor* self) {
    if (rs::isMsgCapKeepLockOn(msg) || rs::isMsgCapIgnoreCancelLockOn(msg))
        return !isDead();
    return tryStartByReceiveMsg(msg, other, self);
}

void ShibakenStateCapCatch::exeChase() {
    if (al::isFirstStep(this))
        al::tryStartActionIfNotPlaying(mShibaken, "Move");

    if (ShibakenFunction::tryStartJump(reinterpret_cast<al::HostStateBase<Shibaken>*>(this), &Jump))
        return;

    sead::Vector3f capPos = {0.0f, 0.0f, 0.0f};
    if (!rs::tryGetFlyingCapPos(&capPos, mShibaken)) {
        kill();
        return;
    }

    const sead::Vector3f& playerPos = rs::getPlayerPos(mShibaken);
    f32 dx = capPos.x - playerPos.x;
    f32 dz = capPos.z - playerPos.z;
    f32 hDist = sead::Mathf::sqrt(dx * dx + dz * dz);
    if (hDist < 300.0f) {
        ShibakenFunction::chaseToTargetRun(mShibaken, capPos);
        return;
    }

    sead::Vector3f upDir = {0.0f, 0.0f, 0.0f};
    al::calcUpDir(&upDir, mShibaken);

    const sead::Vector3f& trans = al::getTrans(mShibaken);
    f32 tdx = capPos.x - trans.x;
    f32 tdy = capPos.y - trans.y;
    f32 tdz = capPos.z - trans.z;
    f32 height = upDir.x * tdx + upDir.y * tdy + upDir.z * tdz;
    if (!(height > 100.0f)) {
        ShibakenFunction::chaseToTargetRun(mShibaken, capPos);
        return;
    }

    const sead::Vector3f& trans2 = al::getTrans(mShibaken);
    sead::Vector3f delta = {capPos.x - trans2.x, capPos.y - trans2.y, capPos.z - trans2.z};
    sead::Vector3f frontDir = {0.0f, 0.0f, 0.0f};
    al::verticalizeVec(&delta, upDir, delta);

    al::calcFrontDir(&frontDir, mShibaken);

    if (al::isNearZero(delta, 0.001f)) {
        ShibakenFunction::chaseToTargetRun(mShibaken, capPos);
        return;
    }

    f32 deltaLen = sead::Mathf::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    if (!(deltaLen < 250.0f)) {
        ShibakenFunction::chaseToTargetRun(mShibaken, capPos);
        return;
    }

    f32 angle = al::calcAngleDegree(frontDir, delta);
    if (!(angle < 45.0f)) {
        ShibakenFunction::chaseToTargetRun(mShibaken, capPos);
        return;
    }

    al::setNerve(this, &Jump);
}

void ShibakenStateCapCatch::exeCatch() {
    if (al::isFirstStep(this))
        al::startAction(mShibaken, "CatchCap");

    ShibakenFunction::addFallVelocityToGroundAndFitPoseOnGround(mShibaken, 0.94f);

    if (al::isActionEnd(mShibaken))
        al::setNerve(this, &Back);
}

// NON_MATCHING: cmp #3,b.ge vs cmp #2,b.gt (compiler normalization)
void ShibakenStateCapCatch::exeBack() {
    if (al::isFirstStep(this))
        al::startAction(mShibaken, "Move");

    if (rs::isPlayerInputTriggerRide(mShibaken))
        _2c++;

    if (_2c >= 3 || ShibakenFunction::chaseToPlayerAndTryStop(mShibaken)) {
        kill();
        return;
    }

    ShibakenFunction::tryStartJump(reinterpret_cast<al::HostStateBase<Shibaken>*>(this), &Jump);
}

void ShibakenStateCapCatch::exeJump() {
    if (!al::updateNerveState(this))
        return;

    if (_28)
        al::setNerve(this, &Back);
    else if (rs::isPlayerCapSpinOrFlying(mShibaken))
        al::setNerve(this, &Chase);
    else
        kill();
}
