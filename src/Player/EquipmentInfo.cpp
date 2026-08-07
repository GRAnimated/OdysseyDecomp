#include "Player/EquipmentInfo.h"

EquipmentInfo::EquipmentInfo(const al::LiveActor* actor)
    : mActor(actor), mFlags(0), mForceDashDirection(0), mForceDashSpeed(0.0f) {}

void EquipmentInfo::onNoCapThrow() {
    mFlags |= 1;
}

void EquipmentInfo::onForceDash(s32 direction, f32 speed) {
    mFlags |= 2;
    mForceDashDirection = direction;
    mForceDashSpeed = speed;
}

bool EquipmentInfo::isNoCapThrow() const {
    return mFlags & 1;
}

bool EquipmentInfo::isForceDash() const {
    return (mFlags >> 1) & 1;
}
