#include "Player/PlayerBindKeeper.h"

#include "Library/LiveActor/ActorSensorUtil.h"

#include "Player/IUsePlayerPuppet.h"
#include "Player/PlayerBindableSensorList.h"
#include "Util/SensorMsgFunction.h"

namespace rs {
bool isGreaterBindPriority(const al::HitSensor* candidate, const al::HitSensor* current);
}

PlayerBindKeeper::PlayerBindKeeper(al::HitSensor* bodyHitSensor, IUsePlayerPuppet* puppet)
    : mBodyHitSensor(bodyHitSensor), mBindSensor(nullptr), mBindableSensorList(nullptr),
      mPuppet(puppet), _20(0), _24(false) {
    mBindableSensorList = new PlayerBindableSensorList();
}

// NON_MATCHING: exact 0x104 size; final success-path store is scheduled after the return-value move instead of before it. Next hypothesis is a source lifetime that consumes mBindSensor before setting _24.
bool PlayerBindKeeper::sendStartMsg() {
    s32 invalidTimer = _20 - 1;
    if (invalidTimer >= 0) {
        _20 = invalidTimer;
        mBindableSensorList->clear();
        return false;
    }

    mBindableSensorList->sort();
    u32 num = mBindableSensorList->getNum();
    if (num == 0) {
        mBindableSensorList->clear();
        return false;
    }

    u32 i = 0;
    al::HitSensor* bindSensor = nullptr;
    for (; i < num; i++) {
        bindSensor = mBindableSensorList->get(i);
        if (rs::isGreaterBindPriority(bindSensor, mBindSensor) &&
            al::sendMsgBindStart(bindSensor, mBodyHitSensor))
            break;
    }
    if (i >= num) {
        mBindableSensorList->clear();
        return false;
    }

    if (mBindSensor) {
        al::sendMsgBindCancel(mBindSensor, mBodyHitSensor);
        mBindSensor = nullptr;
        _24 = false;
        mPuppet->cancel();
        _20 = 30;
    }

    al::sendMsgBindInit(bindSensor, mBodyHitSensor, mBindableSensorList->getType(i));
    mBindableSensorList->clear();
    mBindSensor = bindSensor;
    _24 = false;
    return true;
}

void PlayerBindKeeper::clearBindableSensor() {
    mBindableSensorList->clear();
}

void PlayerBindKeeper::cancelBind() {
    if (!mBindSensor)
        return;
    al::sendMsgBindCancel(mBindSensor, mBodyHitSensor);
    clearBindImpl();
}

bool PlayerBindKeeper::receiveEndMsg(const al::SensorMsg* msg) {
    if (!al::isMsgBindEnd(msg))
        return false;
    mBindSensor = nullptr;
    return true;
}

void PlayerBindKeeper::clearBindImpl() {
    mBindSensor = nullptr;
    _24 = false;
    mPuppet->cancel();
    _20 = 30;
}

void PlayerBindKeeper::cancelBindByDemo() {
    if (!mBindSensor)
        return;
    if (!al::sendMsgBindCancelByDemo(mBindSensor, mBodyHitSensor)) {
        if (!mBindSensor)
            return;
        al::sendMsgBindCancel(mBindSensor, mBodyHitSensor);
    }
    clearBindImpl();
}

bool PlayerBindKeeper::sendMsgBindDamage() {
    return mBindSensor && al::sendMsgBindDamage(mBindSensor, mBodyHitSensor);
}

bool PlayerBindKeeper::receiveRequestDamage() {
    if (!mBindSensor || !mPuppet->isRequestDamage())
        return false;
    mPuppet->clearRequestDamage();
    return true;
}

bool PlayerBindKeeper::sendMsgCollidedGround() {
    return rs::sendMsgBindCollidedGround(mBindSensor, mBodyHitSensor);
}

bool PlayerBindKeeper::sendMsgEnableMapCheckPointWarp() {
    return rs::sendMsgEnableMapCheckPointWarp(mBindSensor, mBodyHitSensor);
}

bool PlayerBindKeeper::sendMsgBindKeepDemoStart() {
    if (!rs::sendMsgBindKeepDemoStart(mBindSensor, mBodyHitSensor))
        return false;
    _24 = true;
    return true;
}

bool PlayerBindKeeper::sendMsgBindKeepDemoExecute() {
    rs::sendMsgBindKeepDemoExecute(mBindSensor, mBodyHitSensor);
    return true;
}

bool PlayerBindKeeper::sendMsgBindKeepDemoEnd() {
    if (!rs::sendMsgBindKeepDemoEnd(mBindSensor, mBodyHitSensor))
        return false;
    _24 = false;
    return true;
}

bool PlayerBindKeeper::sendMsgBindRecoveryLife() const {
    return rs::sendMsgBindRecoveryLife(mBindSensor, mBodyHitSensor);
}

bool PlayerBindKeeper::collectBindableSensor(al::HitSensor* self, al::HitSensor* other) {
    if (!al::isSensorBindableAll(other))
        return false;
    f32 distance = al::calcDistance(other, self);
    f32 selfRadius = al::getSensorRadius(self);
    f32 otherRadius = al::getSensorRadius(other);
    f32 overlap = selfRadius + otherRadius - distance;
    mBindableSensorList->append(other, 0, overlap, 4);
    return true;
}

void PlayerBindKeeper::appendBindRequest(al::HitSensor* sensor) {
    mBindableSensorList->append(sensor, 0, 0.0f, 4);
}

void PlayerBindKeeper::resetInvalidTimer() {
    _20 = 0;
}
