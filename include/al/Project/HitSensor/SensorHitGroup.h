#pragma once

namespace al {
    class HitSensor;

    class SensorHitGroup {
    public:
        SensorHitGroup(int, const char*);

<<<<<<< HEAD
        void add(HitSensor *);
        void remove(HitSensor *);
        HitSensor *getSensor(int) const;
=======
        void add(HitSensor*);
        void remove(HitSensor*);
        HitSensor* getSensor(int) const;
>>>>>>> 60112915b1b559e06290092c73b9c070ba03786b
        void clear() const;

        int mMaxSensorCount;        // _0
        int mCurrentSensorCount;    // _4
        HitSensor **mHitSensors;    // _8
    };
};    // namespace al