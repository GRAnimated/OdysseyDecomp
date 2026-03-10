#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

#include "Npc/YukimaruInput.h"

struct ActorInitInfo;

namespace al {
class HitSensor;
class SensorMsg;
}  // namespace al

class YukimaruStateMove;

class YukimaruRacer : public al::LiveActor, public YukimaruInput {
public:
    YukimaruRacer(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void initAfterPlacement() override;
    void movement() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) override;

    void start();
    void exeWait();
    void exeRun();
    void updateMoveVec();

    bool isTriggerJump() const override;
    bool isHoldJump() const override;
    void calcInputVec(sead::Vector3f* out) const override;

private:
    sead::Quatf mRotation;
    YukimaruStateMove* mStateMove;
    sead::Vector3f mMoveVec;
    bool _134;
};

static_assert(sizeof(YukimaruRacer) == 0x138);
