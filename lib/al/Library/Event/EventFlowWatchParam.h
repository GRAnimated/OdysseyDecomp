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
    const char* mSensorName = nullptr;
    sead::Vector3f mLocalOffset = {0.0f, 0.0f, 0.0f};
};
}  // namespace al
