#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}

class EquipmentInfo {
public:
    EquipmentInfo(const al::LiveActor* actor);

    void onNoCapThrow();
    void onForceDash(s32 direction, f32 speed);

    bool isNoCapThrow() const;
    bool isForceDash() const;

private:
    const al::LiveActor* mActor;
    u32 mFlags = 0;
    s32 mForceDashDirection = 0;
    f32 mForceDashSpeed = 0.0f;
};

static_assert(sizeof(EquipmentInfo) == 0x18);
