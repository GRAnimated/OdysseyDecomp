#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}

class EquipmentInfo {
public:
    explicit EquipmentInfo(const al::LiveActor* actor);

    void onNoCapThrow();
    void onForceDash(s32 direction, f32 speed);

    bool isNoCapThrow() const;
    bool isForceDash() const;

private:
    const al::LiveActor* mActor;
    u32 mFlags;
    s32 mForceDashDirection;
    f32 mForceDashSpeed;
};

static_assert(sizeof(EquipmentInfo) == 0x18);
