#include "HitSensor/SensorHitGroup.h"
#include "HitSensor/HitSensor.h"

namespace al {
    SensorHitGroup::SensorHitGroup(int maxCount, const char* pName) {
        mMaxSensorCount = maxCount;
        mCurrentSensorCount = 0;
        mHitSensors = new HitSensor *[maxCount];

        for (auto i = 0; i < mMaxSensorCount; i++) {
            mHitSensors[i] = nullptr;
        }
    }

    void SensorHitGroup::add(HitSensor* pSensor) {
        mHitSensors[mCurrentSensorCount] = pSensor;
        mCurrentSensorCount++;
    }

    void SensorHitGroup::remove(HitSensor* pSensor) {
        for (auto i = 0; i < mCurrentSensorCount; i++) {
            if (mHitSensors[i] == pSensor) {
                // take the last sensor in the list and put it where this one was removed
                mHitSensors[i] = mHitSensors[mCurrentSensorCount - 1];
                mCurrentSensorCount--;
                break;
            }
        }
    }

    HitSensor *SensorHitGroup::getSensor(int idx) const { return mHitSensors[idx]; }

    void SensorHitGroup::clear() const {
        for (auto i = 0; i < mCurrentSensorCount; i++) {
            mHitSensors[i]->mSensorCount = 0;
        }
    }
};    // namespace al