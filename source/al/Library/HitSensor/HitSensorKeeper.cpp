#include "al/Library/HitSensor/HitSensorKeeper.h"
#include "al/Library/Base/String.h"
#include "al/Project/HitSensor/HitSensor.h"

namespace al {
    HitSensorKeeper::HitSensorKeeper(int maxCount) {
        mMaxSensorNum = maxCount;
        mCurSensorNum = 0;
        mSensors = new HitSensor *[maxCount];

        for (int i = 0; i < mMaxSensorNum; i++) {
            mSensors[i] = nullptr;
        }
    }

    HitSensor *HitSensorKeeper::addSensor(LiveActor *pActor, const char *pSensorName, u32 maxCount, f32 radius, u16 a5, const sead::Vector3f *a6, const sead::Matrix34f *a7, const sead::Vector3f &a8) {
        HitSensor *sensor = new HitSensor(pActor, pSensorName, maxCount, radius, a5, a6, a7, a8);
        mSensors[mCurSensorNum] = sensor;
        mCurSensorNum++;
        sensor->update();
        return sensor;
    }

    void HitSensorKeeper::update() {
        for (int i = 0; i < mCurSensorNum; i++) {
            mSensors[i]->update();
        }
    }

    int HitSensorKeeper::getSensorNum() const { return mCurSensorNum; }

    HitSensor *HitSensorKeeper::getSensor(int idx) const { return mSensors[idx]; }

    // HitSensorKeeper::attackSensor -- needs LiveActor impl

    void HitSensorKeeper::clear() {
        for (int i = 0; i < mCurSensorNum; i++) {
            mSensors[i]->mSensorCount = 0;
        }
    }

    void HitSensorKeeper::validate() {
        for (int i = 0; i < mCurSensorNum; i++) {
            mSensors[i]->validate();
        }
    }

    void HitSensorKeeper::invalidate() {
        for (int i = 0; i < mCurSensorNum; i++) {
            mSensors[i]->invalidate();
        }
    }

    void HitSensorKeeper::validateBySystem() {
        for (int i = 0; i < mCurSensorNum; i++) {
            mSensors[i]->validateBySystem();
        }
    }

    void HitSensorKeeper::invalidateBySystem() {
        for (int i = 0; i < mCurSensorNum; i++) {
            mSensors[i]->invalidateBySystem();
        }
    }

    HitSensor *HitSensorKeeper::getSensor(const char *pSensorName) const {
        if (mCurSensorNum == 1) {
            return mSensors[0];
        }

        for (int i = 0; i < mCurSensorNum; i++) {
            if (al::isEqualString(mSensors[i]->mSensorName, pSensorName)) {
                return mSensors[i];
            }
        }

        return nullptr;
    }
};    // namespace al