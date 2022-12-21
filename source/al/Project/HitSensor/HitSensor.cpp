#include "HitSensor/HitSensor.h"
#include "HitSensor/SensorHitGroup.h"

namespace al {
    // HitSensor::trySensorSort

    void HitSensor::setFollowPosPtr(const sead::Vector3f *pVec) {
        mFollowPosVec = pVec;
        mFollowPosMtx = nullptr;
    }

    void HitSensor::setFollowMtxPtr(const sead::Matrix34f *pMtx) {
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