#include "MapObj/FireHydrant.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"

#include "Player/CapTargetInfo.h"
#include "Util/Hack.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(FireHydrant, SquirtLoop)
NERVE_IMPL(FireHydrant, Wait)
NERVE_IMPL(FireHydrant, SquirtStart)
NERVE_IMPL(FireHydrant, ForbidSquirt)
NERVE_IMPL(FireHydrant, SquirtStop)

NERVES_MAKE_NOSTRUCT(FireHydrant, SquirtLoop, Wait, SquirtStart, ForbidSquirt, SquirtStop)
}  // namespace

FireHydrant::FireHydrant(const char* name) : al::LiveActor(name) {}

void FireHydrant::init(const al::ActorInitInfo& info) {
    al::initActor(this, info);
    mCapTargetInfo = rs::createCapTargetInfo(this, nullptr);
    mCapTargetInfo->set74(true);
    al::initNerve(this, &Wait, 0);

    s32 placeType = 0;
    al::tryGetArg(&placeType, info, "PlaceType");
    if (placeType) {
        mSquirtStartAction = sead::SafeStringBase<char>("SquirtStartNight");
        mSquirtLoopAction = sead::SafeStringBase<char>("SquirtNight");
    } else {
        mSquirtStartAction = sead::SafeStringBase<char>("SquirtStart");
        mSquirtLoopAction = sead::SafeStringBase<char>("Squirt");
    }

    al::invalidateHitSensor(this, "WaterSprayLeft");
    al::invalidateHitSensor(this, "WaterSprayRight");
    makeActorAlive();
}

bool FireHydrant::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                             al::HitSensor* self) {
    if (rs::tryReceiveMsgInitCapTargetAndSetCapTargetInfo(message, mCapTargetInfo))
        return true;
    if (rs::isMsgCapIgnoreCollisionCheck(message))
        return true;
    if (rs::isMsgCapStartLockOn(message) && al::isNerve(this, &Wait)) {
        al::setNerve(this, &SquirtStart);
        return true;
    }
    if (rs::isMsgCapKeepLockOn(message)) {
        if (rs::isPlayerInputHoldCapKeepLockOn(this) || al::isLessEqualStep(this, 20)) {
            sead::Vector3f playerPos = rs::getPlayerPos(this);
            sead::Vector3f trans = al::getTrans(this);
            sead::Vector3f diff = playerPos - trans;
            return diff.length() < 1000.0f;
        } else {
            al::setNerve(this, &ForbidSquirt);
            return false;
        }
    }
    if (rs::isMsgCapCancelLockOn(message)) {
        al::setNerve(this, &SquirtStop);
        return true;
    }
    return false;
}

void FireHydrant::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isSensorSimple(self))
        rs::sendMsgRequestPlayerWet(other, self);
}

void FireHydrant::exeWait() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Wait");
        al::invalidateHitSensor(this, "WaterSprayLeft");
        al::invalidateHitSensor(this, "WaterSprayRight");
    }
}

void FireHydrant::exeSquirtStart() {
    if (al::isFirstStep(this)) {
        al::startAction(this, mSquirtStartAction.cstr());
        al::validateHitSensor(this, "WaterSprayLeft");
        al::validateHitSensor(this, "WaterSprayRight");
    }
    if (al::isActionEnd(this))
        al::setNerve(this, &SquirtLoop);
}

void FireHydrant::exeSquirtLoop() {
    if (al::isFirstStep(this))
        al::startAction(this, mSquirtLoopAction.cstr());
}

void FireHydrant::exeSquirtStop() {
    al::setNerve(this, &ForbidSquirt);
}

void FireHydrant::exeForbidSquirt() {
    if (al::isStep(this, 20))
        al::setNerve(this, &Wait);
}
