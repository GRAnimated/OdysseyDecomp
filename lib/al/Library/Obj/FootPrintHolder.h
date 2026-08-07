#pragma once

#include <basis/seadTypes.h>

namespace al {
class FootPrintServer;
class HitSensor;
class LiveActor;

class FootPrintHolder {
public:
    FootPrintHolder(LiveActor*, const char*, HitSensor*, FootPrintServer*);
    void update();
    void clearFollowTarget() {
        mFollowMtx = nullptr;
        mFollowJoint = nullptr;
    }

private:
    u8 mRuntime[72];
    void* mFollowMtx;
    void* mFollowJoint;
};
static_assert(sizeof(FootPrintHolder) == 0x58);
}
