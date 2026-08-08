#pragma once

#include <basis/seadTypes.h>

namespace al {
class HitSensor;
class SensorMsg;
}  // namespace al
class PlayerBindableSensorList;
class IUsePlayerPuppet;

class PlayerBindKeeper {
public:
    PlayerBindKeeper(al::HitSensor* bodyHitSensor, IUsePlayerPuppet* puppet);
    bool sendStartMsg();
    void clearBindableSensor();
    void cancelBind();
    bool receiveEndMsg(const al::SensorMsg* msg);
    void clearBindImpl();
    void cancelBindByDemo();
    bool sendMsgBindDamage();
    bool receiveRequestDamage();
    bool sendMsgCollidedGround();
    bool sendMsgEnableMapCheckPointWarp();
    bool sendMsgBindKeepDemoStart();
    bool sendMsgBindKeepDemoExecute();
    bool sendMsgBindKeepDemoEnd();
    bool sendMsgBindRecoveryLife() const;
    bool collectBindableSensor(al::HitSensor* self, al::HitSensor* other);
    void appendBindRequest(al::HitSensor* sensor);
    void resetInvalidTimer();

    al::HitSensor* getBindSensor() const { return mBindSensor; }

    bool isKeepDemo() const { return mIsKeepDemo; }
    bool isBindKeepDemo() const { return mIsKeepDemo; }

private:
    al::HitSensor* mBodyHitSensor;
    al::HitSensor* mBindSensor = nullptr;
    PlayerBindableSensorList* mBindableSensorList = nullptr;
    IUsePlayerPuppet* mPuppet;
    s32 _20 = 0;
    union {
        bool mIsKeepDemo;
        bool _24 = false;
    };
};

static_assert(sizeof(PlayerBindKeeper) == 0x28);
