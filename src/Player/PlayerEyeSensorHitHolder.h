#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <math/seadVector.h>

namespace al {
class HitSensor;
class LiveActor;
}

class PlayerEyeSensorHitHolder {
public:
    PlayerEyeSensorHitHolder(s32 capacity);

    void createTargetMarkerBuffer(s32 capacity);
    void createEatTargetBuffer(s32 capacity);
    void clear();
    bool tryRecordHomingAttack(al::HitSensor* self, al::HitSensor* other);
    bool tryRecordTargetMarker(al::HitSensor* self, al::HitSensor* other);
    bool tryRecordEatTarget(al::HitSensor* self, al::HitSensor* other);
    bool tryRecordLookAtTarget(al::HitSensor* self, al::HitSensor* other);
    bool tryRecordPriorityTarget(const al::LiveActor* target, al::HitSensor* self,
                                 al::HitSensor* other);
    bool isEnableRecord(al::HitSensor* self, al::HitSensor* other) const;
    bool isEnableRecordLookAt(al::HitSensor* self, al::HitSensor* other) const;
    al::HitSensor* findNearestSensorDirH(sead::Vector3f* direction, const sead::Vector3f& origin,
                                         const sead::Vector3f& front,
                                         const sead::Vector3f& up, f32 maxDistance,
                                         f32 maxAngle) const;
    al::HitSensor* findNearestSensor(sead::Vector3f* direction, const sead::Vector3f& origin,
                                     const sead::Vector3f& front, const sead::Vector3f& up,
                                     f32 maxDistance, f32 maxAngle, f32 minDistance) const;
    al::HitSensor* findNearestSensorLimit(sead::Vector3f* direction,
                                          const sead::Vector3f& origin,
                                          const sead::Vector3f& front,
                                          const sead::Vector3f& up, f32 maxDistance,
                                          f32 maxAngle, f32 minDistance,
                                          f32 verticalLimit) const;
    al::HitSensor* findTargetMarkerSensor(sead::Vector3f* direction,
                                          const sead::Vector3f& origin,
                                          const sead::Vector3f& front,
                                          const sead::Vector3f& up, f32 maxDistance,
                                          f32 maxAngle, f32 minDistance) const;
    al::HitSensor* findEatTargetSensor(sead::Vector3f* direction,
                                       const sead::Vector3f& origin,
                                       const sead::Vector3f& front,
                                       const sead::Vector3f& up, f32 maxDistance,
                                       f32 maxAngle, f32 minDistance) const;
    al::HitSensor* getLookAtTargetSensor() const { return mLookAtTargetSensor; }

private:
    sead::PtrArray<al::HitSensor> mHomingAttackSensors;
    sead::PtrArray<al::HitSensor> mTargetMarkerSensors;
    sead::PtrArray<al::HitSensor> mEatTargetSensors;
    al::HitSensor* mLookAtTargetSensor;
    al::HitSensor* mPriorityTargetSensor;
    al::HitSensor* mPreviousPriorityTargetSensor;
};

static_assert(sizeof(PlayerEyeSensorHitHolder) == 0x48);
