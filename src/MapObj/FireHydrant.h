#pragma once

#include <prim/seadSafeString.h>

#include "Library/LiveActor/LiveActor.h"

namespace al {
struct ActorInitInfo;
}
class CapTargetInfo;

class FireHydrant : public al::LiveActor {
public:
    FireHydrant(const char* name);

    void init(const al::ActorInitInfo& info) override;
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                    al::HitSensor* self) override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;

    void exeWait();
    void exeSquirtStart();
    void exeSquirtLoop();
    void exeSquirtStop();
    void exeForbidSquirt();

private:
    CapTargetInfo* mCapTargetInfo = nullptr;
    sead::SafeStringBase<char> mSquirtStartAction;
    sead::SafeStringBase<char> mSquirtLoopAction;
};

static_assert(sizeof(FireHydrant) == 0x130);
