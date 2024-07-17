#include "MapObj/TrampleSwitchTimer.h"
#include "Library/Collision/PartsConnector.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/LiveActor/ActorPoseKeeper.h"
#include "Library/LiveActor/ActorSensorFunction.h"
#include "Library/LiveActor/ActorSensorMsgFunction.h"
#include "Library/Math/MathLengthUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Obj/CollisionObj.h"
#include "Library/Obj/PartsFunction.h"
#include "Library/Placement/PlacementFunction.h"
#include "MapObj/AppearSwitchTimer.h"
#include "Util/Sensor.h"

namespace {
NERVE_IMPL(TrampleSwitchTimer, Freeze);
NERVE_IMPL(TrampleSwitchTimer, OffWait);
NERVE_IMPL(TrampleSwitchTimer, OnWait);
NERVE_IMPL(TrampleSwitchTimer, Off);
NERVE_IMPL(TrampleSwitchTimer, On);

NERVE_MAKE(TrampleSwitchTimer, OnWait);
NERVES_MAKE_STRUCT(TrampleSwitchTimer, Freeze, OffWait, Off, On);
}  // namespace

TrampleSwitchTimer::TrampleSwitchTimer(const char* actorName) : LiveActor(actorName) {}

void TrampleSwitchTimer::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, "TrampleSwitch", nullptr);
    bool isNoReaction = false;
    bool tryGetArg = al::tryGetArg(&isNoReaction, info, "IsNoReaction");
    if (!isNoReaction || !tryGetArg) {
        al::initNerve(this, &NrvTrampleSwitchTimer.OffWait, 0);
        mAppearSwitchTimer = new AppearSwitchTimer();
        mAppearSwitchTimer->init(info, this, this, this, this);
        mIsFacingUp = al::isNearZeroOrLess(al::calcQuatUpY(al::getQuat(this)), 0.001f);
        mCollisionObj = al::createCollisionObj(this, info, "TrampleSwitch_Body",
                                               al::getHitSensor(this, "PPanel"), nullptr, nullptr);
        mCollisionObj->makeActorAlive();
        al::validateCollisionParts(mCollisionObj);
        mMtxConnector = al::createMtxConnector(this);
    } else {
        al::startAction(this, "OffWait");
        al::initNerve(this, &NrvTrampleSwitchTimer.Freeze, 0);
    }

    makeActorAlive();
}

void TrampleSwitchTimer::initAfterPlacement() {
    al::LiveActor::initAfterPlacement();

    if (mMtxConnector)
        al::attachMtxConnectorToCollision(mMtxConnector, this, false);
}

void TrampleSwitchTimer::control() {
    if (!al::isNerve(this, &NrvTrampleSwitchTimer.Freeze)) {
        if (mMtxConnector)
            al::connectPoseQT(this, mMtxConnector);
        mAppearSwitchTimer->updateNerve();
    }
}

void TrampleSwitchTimer::exeOn() {
    if (al::isFirstStep(this))
        al::startAction(this, "On");

    if (al::isActionEnd(this))
        al::setNerve(this, &OnWait);
}

void TrampleSwitchTimer::exeOnWait() {
    if (al::isFirstStep(this)) {
        al::invalidateHitSensors(this);
        al::startAction(this, "OnWait");
        mAppearSwitchTimer->onSwitch();
    }
    if (!mAppearSwitchTimer->isSwitchOn())
        al::setNerve(this, &NrvTrampleSwitchTimer.Off);
}

void TrampleSwitchTimer::exeOff() {
    if (al::isFirstStep(this))
        al::startAction(this, "Off");
    if (al::isActionEnd(this))
        al::setNerve(this, &NrvTrampleSwitchTimer.OffWait);
}

void TrampleSwitchTimer::exeOffWait() {
    if (al::isFirstStep(this)) {
        al::validateClipping(this);
        al::validateHitSensors(this);
        al::startAction(this, "OffWait");
    }
}

void TrampleSwitchTimer::exeFreeze() {}

bool TrampleSwitchTimer::receiveMsg(const al::SensorMsg* message, al::HitSensor* source,
                                    al::HitSensor* target) {
    if (al::isNerve(this, &NrvTrampleSwitchTimer.Freeze))
        return rs::isMsgPlayerDisregardHomingAttack(message);

    if (al::isNerve(this, &NrvTrampleSwitchTimer.Off))
        return false;

    if (al::isSensorName(target, "PlayerRegard")) {
        if (!mIsFacingUp && rs::isMsgPlayerDisregardTargetMarker(message))
            return true;
        else
            return false;
    }

    if (mIsFacingUp && !al::isSensorName(target, "PPanel"))
        return false;

    bool v10 = (rs::isMsgCapTouchWall(message) || rs::isMsgCapAttackCollide(message)) &&
               (mIsFacingUp || !al::isNearZeroOrGreater(al::getActorVelocity(source).y, 0.001));
    bool v11 = rs::isMsgCapHipDrop(message);
    bool v12 = al::isMsgPlayerTouch(message);
    if (v12 || v10 || v11) {
        if (al::isNerve(this, &NrvTrampleSwitchTimer.OffWait)) {
            al::invalidateClipping(this);
            al::setNerve(this, &NrvTrampleSwitchTimer.On);
            return true;
        }
    }
    return false;
}
