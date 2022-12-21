#include "HitSensor/HitSensor.h"
#include "HitSensor/SensorHitGroup.h"

namespace al {
    // HitSensor::trySensorSort

<<<<<<< HEAD
    void HitSensor::setFollowPosPtr(const sead::Vector3f *pVec) {
=======
    void HitSensor::setFollowPosPtr(const sead::Vector3f* pVec) {
>>>>>>> 60112915b1b559e06290092c73b9c070ba03786b
        mFollowPosVec = pVec;
        mFollowPosMtx = nullptr;
    }

<<<<<<< HEAD
    void HitSensor::setFollowMtxPtr(const sead::Matrix34f *pMtx) {
=======
    void HitSensor::setFollowMtxPtr(const sead::Matrix34f* pMtx) {
>>>>>>> 60112915b1b559e06290092c73b9c070ba03786b
        mFollowPosVec = nullptr;
        mFollowPosMtx = pMtx;
    }

    void HitSensor::validate() {
        if (!mIsValidBySystem) {
            mIsValidBySystem = true;

            if (mMaxSensorCount != 0) {
                if (mIsValid) {
                    mHitGroup->add(this);
                }
            }
        }

        mSensorCount = 0;
    }

    void HitSensor::invalidate() {
        if (mIsValidBySystem) {
            mIsValidBySystem = false;

            if (mMaxSensorCount != 0) {
                if (mIsValid) {
                    mHitGroup->remove(this);
                }
            }
        }

        mSensorCount = 0;
    }

    void HitSensor::validateBySystem() {
        if (!mIsValid) {
            if (mMaxSensorCount != 0) {
                if (mIsValidBySystem) {
                    mHitGroup->add(this);
                }
            }

            mIsValid = true;
            mSensorCount = 0;
        }
    }

    void HitSensor::invalidateBySystem() {
        if (mIsValid) {
            if (mMaxSensorCount != 0) {
                if (mIsValidBySystem) {
                    mHitGroup->remove(this);
                }
            }

            mIsValid = false;
            mSensorCount = 0;
        }
    }

    // HitSensor::update

    void HitSensor::addHitSensor(HitSensor* pSensor) {
        if (mSensorCount < mMaxSensorCount) {
            mSensors[mSensorCount] = pSensor;
            mSensorCount++;
        }
    }
};    // namespace al