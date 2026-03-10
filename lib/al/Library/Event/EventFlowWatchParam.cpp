#include "Library/Event/EventFlowWatchParam.h"

#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Yaml/ByamlIter.h"
#include "Library/Yaml/ByamlUtil.h"

namespace al {

EventFlowWatchParam::EventFlowWatchParam() = default;

void EventFlowWatchParam::load(const ByamlIter& iter) {
    ByamlIter watchIter;
    if (tryGetByamlIterByKey(&watchIter, iter, "PlayerWatchParam")) {
        mSensorName = tryGetByamlKeyStringOrNULL(watchIter, "SensorName");
        tryGetByamlV3f(&mLocalOffset, watchIter, "LocalOffset");
    }
}

bool EventFlowWatchParam::isWatchSensor(const HitSensor* sensor) const {
    if (mSensorName)
        return isSensorName(sensor, mSensorName);
    return false;
}

// NON_MATCHING: Vector3f copy generates 32-bit loads/stores instead of 64-bit; minor regswaps
void EventFlowWatchParam::calcWatchTrans(sead::Vector3f* out, const LiveActor* actor) const {
    if (mSensorName) {
        HitSensor* sensor = getHitSensor(actor, mSensorName);
        const sead::Vector3f& sensorPos = getSensorPos(sensor);
        out->z = sensorPos.z;
        out->x = sensorPos.x;
        out->y = sensorPos.y;
        if (!isNearZero(mLocalOffset, 0.001f)) {
            sead::Vector3f transformed = mLocalOffset;
            multVecPoseNoTrans(&transformed, actor, transformed);
            out->x += transformed.x;
            out->y += transformed.y;
            out->z += transformed.z;
        }
    } else {
        *out = getTrans(actor);
    }
}

}  // namespace al
