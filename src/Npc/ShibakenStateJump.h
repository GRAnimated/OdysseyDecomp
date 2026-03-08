#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class HitSensor;
class SensorMsg;
}  // namespace al

class Shibaken;

class ShibakenStateJump : public al::NerveStateBase {
public:
    ShibakenStateJump(const char* name, Shibaken* shibaken);

    void appear() override;
    bool update() override;

    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self);
    bool isKillByDeathArea() const;

    void exeJumpStart();
    void exeJump();
    void exeJumpEnd();
    void exeKillByDeathArea();

private:
    Shibaken* mShibaken = nullptr;
};

static_assert(sizeof(ShibakenStateJump) == 0x20);
