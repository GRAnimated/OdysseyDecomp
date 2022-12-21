#pragma once

#include "math/seadMatrix.h"
#include "math/seadVector.h"
#include <basis/seadTypes.h>

namespace al {
    class HitSensor;
    class LiveActor;

    class HitSensorKeeper {
    public:
        HitSensorKeeper(int);

<<<<<<< HEAD
        HitSensor *addSensor(LiveActor *, const char *, u32, f32, u16, const sead::Vector3f *, const sead::Matrix34f *, const sead::Vector3f &);
=======
        HitSensor* addSensor(LiveActor*, const char*, u32, f32, u16, const sead::Vector3f*, const sead::Matrix34f*, const sead::Vector3f&);
>>>>>>> 60112915b1b559e06290092c73b9c070ba03786b
        void update();
        int getSensorNum() const;
        HitSensor *getSensor(int) const;
        void attackSensor();
        void clear();
        void validate();
        void invalidate();
        void validateBySystem();
        void invalidateBySystem();
<<<<<<< HEAD
        HitSensor *getSensor(const char *) const;

        int mMaxSensorNum;       // _0
        int mCurSensorNum;       // _4
        HitSensor **mSensors;    // _8
=======
        HitSensor* getSensor(const char*) const;

        int mMaxSensorNum;       // _0
        int mCurSensorNum;       // _4
        HitSensor** mSensors;    // _8
>>>>>>> 60112915b1b559e06290092c73b9c070ba03786b
    };
};    // namespace al
