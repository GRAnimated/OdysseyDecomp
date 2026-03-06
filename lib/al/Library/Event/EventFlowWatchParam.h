#pragma once

#include <math/seadVector.h>

namespace al {
class ByamlIter;
class HitSensor;
class LiveActor;

class EventFlowWatchParam {
public:
    EventFlowWatchParam();

    void load(const ByamlIter& iter);
    bool isWatchSensor(const HitSensor* sensor) const;
    void calcWatchTrans(sead::Vector3f* out, const LiveActor* actor) const;

private:
    u8 mData[0x18];
};
}  // namespace al
