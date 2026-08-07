#include "Player/PlayerEyeSensorHitHolder.h"

#include <cstring>

#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"

#include "Util/SensorMsgFunction.h"

PlayerEyeSensorHitHolder::PlayerEyeSensorHitHolder(s32 capacity) {
    std::memset(this, 0, sizeof(*this));
    mHomingAttackSensors.allocBuffer(capacity, nullptr, 8);
}

void PlayerEyeSensorHitHolder::createTargetMarkerBuffer(s32 capacity) {
    mTargetMarkerSensors.allocBuffer(capacity, nullptr, 8);
}

void PlayerEyeSensorHitHolder::createEatTargetBuffer(s32 capacity) {
    mEatTargetSensors.allocBuffer(capacity, nullptr, 8);
}

void PlayerEyeSensorHitHolder::clear() {
    al::HitSensor* lookAtTargetSensor = mLookAtTargetSensor;
    al::HitSensor* priorityTargetSensor = mPriorityTargetSensor;
    mHomingAttackSensors.clear();
    mTargetMarkerSensors.clear();
    mEatTargetSensors.clear();
    mPriorityTargetSensor = nullptr;
    mPreviousPriorityTargetSensor = priorityTargetSensor;

    if (lookAtTargetSensor &&
        (al::isDead(al::getSensorHost(lookAtTargetSensor)) ||
         !al::isSensorValid(mLookAtTargetSensor) ||
         al::sendMsgPlayerDisregard(mLookAtTargetSensor, mLookAtTargetSensor))) {
        mLookAtTargetSensor = nullptr;
    }
}

bool PlayerEyeSensorHitHolder::tryRecordHomingAttack(al::HitSensor* self,
                                                      al::HitSensor* other) {
    if (rs::sendMsgPlayerDisregardHomingAttack(self, other))
        return false;
    if (!mHomingAttackSensors.isFull())
        mHomingAttackSensors.pushBack(self);
    return true;
}

bool PlayerEyeSensorHitHolder::tryRecordTargetMarker(al::HitSensor* self,
                                                      al::HitSensor* other) {
    if (rs::sendMsgPlayerDisregardTargetMarker(self, other))
        return false;
    if (!mTargetMarkerSensors.isFull())
        mTargetMarkerSensors.pushBack(self);
    return true;
}

bool PlayerEyeSensorHitHolder::tryRecordEatTarget(al::HitSensor* self, al::HitSensor* other) {
    if (!rs::sendMsgYoshiTongueEatHomingTarget(self, other))
        return false;
    if (!mEatTargetSensors.isFull())
        mEatTargetSensors.pushBack(self);
    return true;
}

bool PlayerEyeSensorHitHolder::tryRecordLookAtTarget(al::HitSensor* self,
                                                      al::HitSensor* other) {
    if (mLookAtTargetSensor) {
        const sead::Vector3f& origin = al::getSensorPos(other);
        const sead::Vector3f& current = al::getSensorPos(mLookAtTargetSensor);
        const sead::Vector3f currentDifference = current - origin;
        const sead::Vector3f& candidate = al::getSensorPos(self);
        const sead::Vector3f candidateDifference = candidate - origin;
        if (currentDifference.squaredLength() <= candidateDifference.squaredLength())
            return false;
    }
    mLookAtTargetSensor = self;
    return true;
}

bool PlayerEyeSensorHitHolder::tryRecordPriorityTarget(const al::LiveActor* target,
                                                        al::HitSensor* self,
                                                        al::HitSensor* other) {
    if (mPriorityTargetSensor)
        return false;
    if (!rs::sendMsgPlayerEyePriorityTarget(self, other))
        return false;

    sead::Vector3f sensorDir(0.0f, 0.0f, 0.0f);
    if (al::calcDirBetweenSensorsH(&sensorDir, other, self)) {
        sead::Vector3f front(0.0f, 0.0f, 0.0f);
        al::calcFrontDir(&front, target);
        al::verticalizeVec(&front, al::getGravity(target), front);
        if (al::tryNormalizeOrZero(&front) && front.dot(sensorDir) < -0.17365f)
            return false;
    }

    mPriorityTargetSensor = self;
    return true;
}

bool PlayerEyeSensorHitHolder::isEnableRecord(al::HitSensor* self,
                                               al::HitSensor* other) const {
    if (al::isSensorEnemyBody(self) || al::isSensorMapObj(self) || al::isSensorNpc(self) ||
        al::isSensorBindableAll(self)) {
        return !al::sendMsgPlayerDisregard(self, other);
    }
    return false;
}

bool PlayerEyeSensorHitHolder::isEnableRecordLookAt(al::HitSensor* self,
                                                     al::HitSensor* other) const {
    if (al::isSensorEnemyBody(self) || al::isSensorMapObj(self) || al::isSensorNpc(self) ||
        al::isSensorBindableAll(self) || al::isSensorLookAt(self)) {
        return !al::sendMsgPlayerDisregard(self, other);
    }
    return false;
}


al::HitSensor* PlayerEyeSensorHitHolder::findNearestSensorDirH(
    sead::Vector3f* direction, const sead::Vector3f& origin, const sead::Vector3f& front,
    const sead::Vector3f& up, f32 maxDistance, f32 maxAngle) const {
    if (mHomingAttackSensors.isEmpty())
        return nullptr;

    al::HitSensor* nearestSensor = nullptr;
    f32 nearestDistance = 10000.0f;
    al::HitSensor** sensorPtr = mHomingAttackSensors.dataBegin();
    al::HitSensor** sensorEnd = mHomingAttackSensors.dataEnd();
    for (; sensorPtr != sensorEnd; sensorPtr++) {
        sead::Vector3f cross;
        sead::Vector3f sensorDirection = al::getSensorPos(*sensorPtr) - origin;
        al::verticalizeVec(&sensorDirection, up, sensorDirection);

        sead::Vector3f parallel(0.0f, 0.0f, 0.0f);
        sead::Vector3f vertical(0.0f, 0.0f, 0.0f);
        al::separateVectorParallelVertical(&parallel, &vertical, front, sensorDirection);
        const f32 distance = parallel.length() + vertical.length() * 5.0f;
        if (distance > nearestDistance)
            continue;

        sead::Vector3f normalizedDirection(0.0f, 0.0f, 0.0f);
        if (!al::tryNormalizeOrZero(&normalizedDirection, sensorDirection))
            continue;

        cross = front.cross(normalizedDirection);
        al::tryNormalizeOrZero(&cross);
        const f32 angleDot = cross.dot(up);
        f32 angleOffset = -maxAngle;
        if (angleDot >= 0.0f)
            angleOffset = maxAngle;
        const f32 angleLimit = maxDistance + angleOffset;
        const f32 angle = al::calcAngleDegree(front, normalizedDirection);
        if (!(angleLimit >= angle))
            continue;

        direction->set(normalizedDirection);
        nearestSensor = *sensorPtr;
        nearestDistance = distance;
    }
    return nearestSensor;
}

al::HitSensor* PlayerEyeSensorHitHolder::findNearestSensor(
    sead::Vector3f* direction, const sead::Vector3f& origin, const sead::Vector3f& front,
    const sead::Vector3f& up, f32 maxDistance, f32 maxAngle, f32 minDistance) const {
    return findNearestSensorLimit(direction, origin, front, up, maxDistance, maxAngle,
                                  minDistance, 10000.0f);
}


// NON_MATCHING: first corpus-backed implementation; next compare nested angle-check temporary lifetimes.
al::HitSensor* PlayerEyeSensorHitHolder::findNearestSensorLimit(
    sead::Vector3f* direction, const sead::Vector3f& origin, const sead::Vector3f& front,
    const sead::Vector3f& up, f32 maxDistance, f32 maxAngle, f32 minDistance,
    f32 verticalLimit) const {
    if (mHomingAttackSensors.isEmpty())
        return nullptr;

    al::HitSensor* nearestSensor = nullptr;
    f32 nearestDistance = 10000.0f;
    al::HitSensor** sensorPtr = mHomingAttackSensors.dataBegin();
    al::HitSensor** sensorEnd = mHomingAttackSensors.dataEnd();
    for (; sensorPtr != sensorEnd; sensorPtr++) {
        sead::Vector3f cross;
        sead::Vector3f sensorDirection = al::getSensorPos(*sensorPtr) - origin;
        if (sensorDirection.length() > verticalLimit)
            continue;

        sead::Vector3f frontParallel(0.0f, 0.0f, 0.0f);
        sead::Vector3f frontVertical(0.0f, 0.0f, 0.0f);
        al::separateVectorParallelVertical(&frontParallel, &frontVertical, front,
                                           sensorDirection);
        const f32 distance = frontParallel.length() + frontVertical.length() * 5.0f;
        if (distance > nearestDistance)
            continue;

        sead::Vector3f upParallel(0.0f, 0.0f, 0.0f);
        sead::Vector3f upVertical(0.0f, 0.0f, 0.0f);
        al::separateVectorParallelVertical(&upParallel, &upVertical, up, sensorDirection);

        sead::Vector3f normalizedDirection(0.0f, 0.0f, 0.0f);
        if (!al::tryNormalizeOrZero(&normalizedDirection, sensorDirection))
            continue;

        cross = front.cross(normalizedDirection);
        al::tryNormalizeOrZero(&cross);
        f32 angleOffset = -0.0f;
        if (cross.dot(up) >= 0.0f)
            angleOffset = 0.0f;
        const f32 angleLimit = maxAngle + angleOffset;
        const f32 angle = al::calcAngleDegree(front, normalizedDirection);
        if (!(angleLimit >= angle))
            continue;

        if (upVertical.length() > maxDistance) {
            sead::Vector3f secondaryDirection(0.0f, 0.0f, 0.0f);
            if (!al::tryNormalizeOrZero(&secondaryDirection, sensorDirection))
                continue;

            const sead::Vector3f sideDirection = front.cross(up);
            cross = front.cross(secondaryDirection);
            al::tryNormalizeOrZero(&cross);
            f32 secondaryAngleOffset = -0.0f;
            if (sideDirection.dot(cross) >= 0.0f)
                secondaryAngleOffset = 0.0f;
            const f32 secondaryAngleLimit = minDistance + secondaryAngleOffset;
            const f32 secondaryAngle = al::calcAngleDegree(front, secondaryDirection);
            if (!(secondaryAngleLimit >= secondaryAngle))
                continue;
        }

        cross.set(0.0f, 0.0f, 0.0f);
        if (!al::tryNormalizeOrZero(&cross, sensorDirection))
            continue;
        direction->set(cross);
        nearestSensor = *sensorPtr;
        nearestDistance = distance;
    }
    return nearestSensor;
}


al::HitSensor* PlayerEyeSensorHitHolder::findTargetMarkerSensor(
    sead::Vector3f* direction, const sead::Vector3f& origin, const sead::Vector3f& front,
    const sead::Vector3f& up, f32 maxDistance, f32 maxAngle, f32 minDistance) const {
    if (mTargetMarkerSensors.isEmpty())
        return nullptr;

    al::HitSensor* nearestSensor = nullptr;
    f32 nearestDistance = 10000.0f;
    al::HitSensor** sensorPtr = mTargetMarkerSensors.dataBegin();
    al::HitSensor** sensorEnd = mTargetMarkerSensors.dataEnd();
    const f32 negativeMaxAngle = -maxAngle;
    for (; sensorPtr != sensorEnd; sensorPtr++) {
        sead::Vector3f cross;
        sead::Vector3f sensorDirection = al::getSensorPos(*sensorPtr) - origin;
        const f32 sensorHeight = sead::Mathf::max(
            0.0f, sead::Mathf::abs(sensorDirection.dot(up)) - al::getSensorRadius(*sensorPtr));
        if (sensorHeight > minDistance)
            continue;

        al::verticalizeVec(&sensorDirection, up, sensorDirection);
        sead::Vector3f parallel(0.0f, 0.0f, 0.0f);
        sead::Vector3f vertical(0.0f, 0.0f, 0.0f);
        al::separateVectorParallelVertical(&parallel, &vertical, front, sensorDirection);
        const f32 distance = parallel.length() + vertical.length() * 5.0f;
        if (distance > nearestDistance)
            continue;

        sead::Vector3f normalizedDirection(0.0f, 0.0f, 0.0f);
        if (!al::tryNormalizeOrZero(&normalizedDirection, sensorDirection))
            continue;

        cross = front.cross(normalizedDirection);
        al::tryNormalizeOrZero(&cross);
        f32 angleOffset = negativeMaxAngle;
        if (cross.dot(up) >= 0.0f)
            angleOffset = maxAngle;
        const f32 angleLimit = maxDistance + angleOffset;
        const f32 angle = al::calcAngleDegree(front, normalizedDirection);
        if (!(angleLimit >= angle))
            continue;

        direction->set(normalizedDirection);
        nearestSensor = *sensorPtr;
        nearestDistance = distance;
    }
    return nearestSensor;
}


// NON_MATCHING: first corpus-backed two-stage implementation; next compare shared temporary reuse and parameter spill ordering.
al::HitSensor* PlayerEyeSensorHitHolder::findEatTargetSensor(
    sead::Vector3f* direction, const sead::Vector3f& origin, const sead::Vector3f& front,
    const sead::Vector3f& up, f32 maxDistance, f32 maxAngle, f32 minDistance) const {
    al::HitSensor* nearestSensor = nullptr;
    f32 nearestDistance = 10000.0f;

    if (!mEatTargetSensors.isEmpty()) {
        al::HitSensor** sensorPtr = mEatTargetSensors.dataBegin();
        al::HitSensor** sensorEnd = mEatTargetSensors.dataEnd();
        for (; sensorPtr != sensorEnd; sensorPtr++) {
            sead::Vector3f cross;
            sead::Vector3f sensorDirection = al::getSensorPos(*sensorPtr) - origin;
            sead::Vector3f frontParallel(0.0f, 0.0f, 0.0f);
            sead::Vector3f frontVertical(0.0f, 0.0f, 0.0f);
            al::separateVectorParallelVertical(&frontParallel, &frontVertical, front,
                                               sensorDirection);
            const f32 distance = frontParallel.length() + frontVertical.length() * 5.0f;
            if (distance > nearestDistance)
                continue;

            sead::Vector3f upParallel(0.0f, 0.0f, 0.0f);
            sead::Vector3f upVertical(0.0f, 0.0f, 0.0f);
            al::separateVectorParallelVertical(&upParallel, &upVertical, up, sensorDirection);

            sead::Vector3f normalizedDirection(0.0f, 0.0f, 0.0f);
            if (!al::tryNormalizeOrZero(&normalizedDirection, sensorDirection))
                continue;

            cross = front.cross(normalizedDirection);
            al::tryNormalizeOrZero(&cross);
            f32 angleOffset = -0.0f;
            if (cross.dot(up) >= 0.0f)
                angleOffset = 0.0f;
            const f32 angleLimit = maxAngle + angleOffset;
            const f32 angle = al::calcAngleDegree(front, normalizedDirection);
            if (!(angleLimit >= angle))
                continue;

            if (upVertical.length() > maxDistance) {
                sead::Vector3f secondaryDirection(0.0f, 0.0f, 0.0f);
                if (!al::tryNormalizeOrZero(&secondaryDirection, sensorDirection))
                    continue;

                const sead::Vector3f sideDirection = front.cross(up);
                cross = front.cross(secondaryDirection);
                al::tryNormalizeOrZero(&cross);
                f32 secondaryAngleOffset = -0.0f;
                if (sideDirection.dot(cross) >= 0.0f)
                    secondaryAngleOffset = 0.0f;
                const f32 secondaryAngleLimit = minDistance + secondaryAngleOffset;
                const f32 secondaryAngle = al::calcAngleDegree(front, secondaryDirection);
                if (!(secondaryAngleLimit >= secondaryAngle))
                    continue;
            }

            cross.set(0.0f, 0.0f, 0.0f);
            if (!al::tryNormalizeOrZero(&cross, sensorDirection))
                continue;
            direction->set(cross);
            nearestSensor = *sensorPtr;
            nearestDistance = distance;
        }
        if (nearestSensor)
            return nearestSensor;
    }

    if (mHomingAttackSensors.isEmpty())
        return nullptr;

    al::HitSensor** sensorPtr = mHomingAttackSensors.dataBegin();
    al::HitSensor** sensorEnd = mHomingAttackSensors.dataEnd();
    for (; sensorPtr != sensorEnd; sensorPtr++) {
        sead::Vector3f cross;
        sead::Vector3f sensorDirection = al::getSensorPos(*sensorPtr) - origin;
        al::verticalizeVec(&sensorDirection, up, sensorDirection);
        sead::Vector3f parallel(0.0f, 0.0f, 0.0f);
        sead::Vector3f vertical(0.0f, 0.0f, 0.0f);
        al::separateVectorParallelVertical(&parallel, &vertical, front, sensorDirection);
        const f32 distance = parallel.length() + vertical.length() * 5.0f;
        if (distance > nearestDistance)
            continue;

        sead::Vector3f normalizedDirection(0.0f, 0.0f, 0.0f);
        if (!al::tryNormalizeOrZero(&normalizedDirection, sensorDirection))
            continue;

        cross = front.cross(normalizedDirection);
        al::tryNormalizeOrZero(&cross);
        f32 angleOffset = -0.0f;
        if (cross.dot(up) >= 0.0f)
            angleOffset = 0.0f;
        const f32 angleLimit = maxAngle + angleOffset;
        const f32 angle = al::calcAngleDegree(front, normalizedDirection);
        if (!(angleLimit >= angle))
            continue;

        direction->set(normalizedDirection);
        nearestSensor = *sensorPtr;
        nearestDistance = distance;
    }
    return nearestSensor;
}
