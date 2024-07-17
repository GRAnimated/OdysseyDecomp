#include "MapObj/TrampleBush.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseKeeper.h"
#include "Library/LiveActor/ActorSensorFunction.h"
#include "Library/LiveActor/ActorSensorMsgFunction.h"
#include "Library/LiveActor/LiveActorUtil.h"
#include "Library/Math/MathLengthUtil.h"
#include "Library/Math/MathRandomUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Util/ItemGenerator.h"
#include "Util/Sensor.h"

namespace {
NERVE_IMPL(TrampleBush, Wait);
NERVE_IMPL(TrampleBush, Reaction);
NERVE_IMPL(TrampleBush, Trample);

NERVES_MAKE_STRUCT(TrampleBush, Wait, Reaction, Trample);
}  // namespace

TrampleBush::TrampleBush(const char* actorName) : al::LiveActor(actorName) {}

void TrampleBush::init(const al::ActorInitInfo& info) {
    al::initMapPartsActor(this, info, nullptr);
    mItemGenerator = new ItemGenerator(this, info);
    al::initNerve(this, &NrvTrampleBush.Wait, 0);
    makeActorAlive();
}

void TrampleBush::initAfterPlacement() {
    if (mItemGenerator->isNone()) {
        sead::Quatf randomRotation;
        al::calcQuat(&randomRotation, this);
        al::rotateQuatYDirDegree(&randomRotation, randomRotation, al::getRandomDegree());
        al::updatePoseQuat(this, randomRotation);
    }
}

void TrampleBush::attackSensor(al::HitSensor* source, al::HitSensor* target) {
    if (rs::sendMsgPushToPlayer(target, source))
        al::sendMsgPush(target, source);
}

bool TrampleBush::receiveMsg(const al::SensorMsg* message, al::HitSensor* source,
                             al::HitSensor* target) {
    if (rs::isMsgPlayerDisregardHomingAttack(message) ||
        rs::isMsgPlayerDisregardTargetMarker(message) || rs::isMsgPlayerDisregard(message))
        return true;

    if (!al::isNerve(this, &NrvTrampleBush.Wait) && !al::isNerve(this, &NrvTrampleBush.Reaction))
        return false;

    if (rs::isMsgFrogHackTrample(message) ||
        (rs::isMsgCapAttack(message) && al::isNerve(this, &NrvTrampleBush.Wait) &&
         al::isGreaterEqualStep(this, 10)) ||
        al::isMsgEnemyTouch(message) || al::isMsgPlayerTouch(message) ||
        (al::isMsgPlayerObjTouch(message) &&
         !al::isNear(mActorTrans, al::getActorTrans(source), 3.0f) &&
         al::isNerve(this, &NrvTrampleBush.Wait) && al::isGreaterEqualStep(this, 10))) {
        al::setNerve(this, &NrvTrampleBush.Reaction);
        return true;
    }
    if (rs::isMsgPressDown(message)) {
        al::setNerve(this, &NrvTrampleBush.Trample);
        return true;
    }
    return false;
}

void TrampleBush::exeWait() {}

void TrampleBush::exeReaction() {
    if (al::isActionEnd(this))
        al::setNerve(this, &NrvTrampleBush.Wait);
}

void TrampleBush::exeTrample() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Trample");
        al::invalidateHitSensors(this);
        return;
    }
    if (al::isActionEnd(this)) {
        al::startHitReaction(this, "消滅");
        if (!mItemGenerator->isNone()) {
            sead::Vector3f frontDir;
            al::calcFrontDir(&frontDir, this);
            mItemGenerator->generate(al::getTrans(this), frontDir);
        }
        kill();
    }
}
