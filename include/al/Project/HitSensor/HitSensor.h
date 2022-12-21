#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
    class SensorHitGroup;
    class LiveActor;

    class HitSensor {
    public:
        HitSensor(LiveActor *, const char *, u32, f32, u16, const sead::Vector3f *, const sead::Matrix34f *, const sead::Vector3f &);

        bool trySensorSort();
        void setFollowPosPtr(const sead::Vector3f *);
        void setFollowMtxPtr(const sead::Matrix34f *);
        void validate();
        void invalidate();
        void validateBySystem();
        void invalidateBySystem();
        void update();
        void addHitSensor(HitSensor*);

        const char *mSensorName;    // _0
        u32 _8;
        u32 _C;
        u32 *_10;
        f32 mSensorRadius;       // _18
        u16 mMaxSensorCount;     // _1C
        u16 mSensorCount;        // _1E
        HitSensor **mSensors;    // _20
        u32 *_28;
        SensorHitGroup *mHitGroup;    // _30
        bool mIsValid;                // _38
        bool mIsValidBySystem;        // _39
        u32 _3C;
        LiveActor *mParentActor;                 // _40
        const sead::Vector3f *mFollowPosVec;     // _48
        const sead::Matrix34f *mFollowPosMtx;    // _50
        sead::Vector3f _58;
    };
};    // namespace al