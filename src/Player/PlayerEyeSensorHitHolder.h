#pragma once

#include <basis/seadTypes.h>

namespace al {
class HitSensor;
}  // namespace al

class PlayerEyeSensorHitHolder {
public:
    PlayerEyeSensorHitHolder(s32 maxCount);

    void clear();
    bool isEnableRecordLookAt(al::HitSensor* eyeSensor, al::HitSensor* targetSensor) const;
    void tryRecordLookAtTarget(al::HitSensor* eyeSensor, al::HitSensor* targetSensor);
};
