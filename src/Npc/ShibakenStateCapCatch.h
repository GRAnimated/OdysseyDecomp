#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class HitSensor;
class SensorMsg;
}  // namespace al

class Shibaken;
class ShibakenStateJump;

class ShibakenStateCapCatch : public al::NerveStateBase {
public:
    ShibakenStateCapCatch(const char* name, Shibaken* shibaken);

    void appear() override;
    void kill() override;

    bool tryStart();
    bool tryStartByReceiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                              al::HitSensor* self);
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self);

    void exeChase();
    void exeCatch();
    void exeBack();
    void exeJump();

private:
    Shibaken* mShibaken = nullptr;
    ShibakenStateJump* mStateJump = nullptr;
    bool _28 = false;
    s32 _2c = 0;
};

static_assert(sizeof(ShibakenStateCapCatch) == 0x30);
