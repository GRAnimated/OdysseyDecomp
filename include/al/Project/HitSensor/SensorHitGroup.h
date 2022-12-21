#pragma once

namespace al {
    class HitSensor;

    class SensorHitGroup {
    public:
        SensorHitGroup(int, const char*);

        void add(HitSensor *);
        void remove(HitSensor *);
        HitSensor *getSensor(int) const;
        void clear() const;

        int mMaxSensorCount;        // _0
        int mCurrentSensorCount;    // _4
        HitSensor **mHitSensors;    // _8
    };
};    // namespace al