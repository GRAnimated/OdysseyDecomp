#include "Npc/ShibakenStateBark.h"

#include <math/seadVector.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/ActorStateReactionBase.h"
#include "Npc/Shibaken.h"
#include "Npc/ShibakenFunction.h"
#include "Npc/ShibakenStateTurn.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(ShibakenStateBark, BarkSign);
NERVE_IMPL(ShibakenStateBark, Turn);
NERVE_IMPL(ShibakenStateBark, Reaction);
NERVE_IMPL(ShibakenStateBark, Walk);
NERVE_IMPL(ShibakenStateBark, Bark);
NERVE_IMPL(ShibakenStateBark, Wait);

NERVES_MAKE_NOSTRUCT(ShibakenStateBark, Bark, Wait, BarkSign, Turn, Reaction, Walk)
}  // namespace

ShibakenStateBark::ShibakenStateBark(const char* name, Shibaken* shibaken,
                                     ActorStateReactionBase* reaction)
    : al::NerveStateBase(name), mShibaken(shibaken), mStateReaction(reaction) {
    initNerve(&BarkSign, 2);
    mStateTurn = new ShibakenStateTurn("ターン", mShibaken);
    al::initNerveState(this, mStateTurn, &Turn, "ターン");
    al::initNerveState(this, mStateReaction, &Reaction, "リアクション");
}

void ShibakenStateBark::kill() {
    al::NerveStateBase::kill();
    mEnemySensor = nullptr;
}

void ShibakenStateBark::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (!al::isSensorNpc(self))
        return;
    if (!al::isSensorEnemyBody(other))
        return;

    f32 distance = al::calcDistance(self, other);
    f32 radius = al::getSensorRadius(other);
    if (!(distance < radius + 300.0f))
        return;

    if (rs::sendMsgNpcScareByEnemy(other, self, -1))
        mEnemySensor = other;
}

bool ShibakenStateBark::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                   al::HitSensor* self) {
    return ShibakenFunction::tryStartReaction(this, mStateReaction, &Reaction, msg, other, self);
}

bool ShibakenStateBark::tryStart() {
    if (mEnemySensor == nullptr)
        return false;

    if (ShibakenFunction::isPlayingMoveAction(mShibaken))
        al::setNerve(this, &Walk);
    else
        al::setNerve(this, &BarkSign);
    return true;
}

void ShibakenStateBark::exeBarkSign() {
    if (al::isFirstStep(this))
        al::startAction(mShibaken, "BarkSign");
    ShibakenFunction::addFallVelocityToGround(mShibaken, -1.0f);
    if (al::isActionEnd(mShibaken))
        al::setNerve(this, &Bark);
}

void ShibakenStateBark::exeBark() {
    if (al::isFirstStep(this))
        al::startAction(mShibaken, "Bark");
    ShibakenFunction::addFallVelocityToGround(mShibaken, -1.0f);

    if (!al::isActionEnd(mShibaken))
        return;

    auto* enemy = mEnemySensor;
    auto* host = al::getSensorHost(enemy);
    if (!al::isDead(host) && al::isSensorValid(enemy) &&
        !(al::calcDistance(mShibaken, al::getActorTrans(enemy)) < 300.0f)) {
        al::setNerve(this, &Wait);
    } else {
        kill();
    }
}

void ShibakenStateBark::exeWait() {
    if (al::isFirstStep(this))
        al::startAction(mShibaken, "Wait");
    ShibakenFunction::addFallVelocityToGround(mShibaken, -1.0f);

    if (!al::isGreaterEqualStep(this, 30))
        return;

    if (mStateTurn->tryStartTurn(al::getActorTrans(mEnemySensor)))
        al::setNerve(this, &Turn);
    else
        al::setNerve(this, &Walk);
}

void ShibakenStateBark::exeWalk() {
    if (al::isFirstStep(this))
        al::tryStartActionIfNotPlaying(mShibaken, "Move");

    ShibakenFunction::chaseToTargetWalk(mShibaken, al::getActorTrans(mEnemySensor));

    sead::Vector3f upDir = {0.0f, 0.0f, 0.0f};
    sead::Vector3f frontDir = {0.0f, 0.0f, 0.0f};

    const sead::Vector3f& enemyPos = al::getActorTrans(mEnemySensor);
    const sead::Vector3f& myPos = al::getTrans(mShibaken);
    sead::Vector3f delta = {enemyPos.x - myPos.x, enemyPos.y - myPos.y, enemyPos.z - myPos.z};

    al::calcUpDir(&upDir, mShibaken);
    al::calcFrontDir(&frontDir, mShibaken);
    al::verticalizeVec(&delta, upDir, delta);

    if (al::isNearZero(delta, 0.001f) || al::isNearAngleDegree(frontDir, delta, 45.0f))
        al::setNerve(this, &BarkSign);
    else if (al::isGreaterEqualStep(this, 300))
        kill();
}

void ShibakenStateBark::exeTurn() {
    if (!al::updateNerveState(this))
        return;

    auto* enemy = mEnemySensor;
    auto* host = al::getSensorHost(enemy);
    if (!al::isDead(host) && al::isSensorValid(enemy) &&
        !(al::calcDistance(mShibaken, al::getActorTrans(enemy)) < 300.0f)) {
        al::setNerve(this, &BarkSign);
    } else {
        kill();
    }
}

void ShibakenStateBark::exeReaction() {
    if (!ShibakenFunction::executeReactionNerve(
            reinterpret_cast<al::HostStateBase<Shibaken>*>(this)))
        return;

    if (!(al::calcDistance(mShibaken, al::getActorTrans(mEnemySensor)) < 300.0f))
        kill();
    else
        al::setNerve(this, &Wait);
}
