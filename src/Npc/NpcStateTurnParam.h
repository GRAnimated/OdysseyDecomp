#pragma once

#include <basis/seadTypes.h>

class NpcStateTurnParam {
public:
    NpcStateTurnParam(f32 angleThreshold, f32 angleEnd, f32 turnSpeed, f32 nearDistance,
                      bool isEnable, bool isOnlyFromWaitAfter, s32 turnEndStep);

    f32 mAngleThreshold;
    f32 mAngleEnd;
    f32 mTurnSpeed;
    f32 mNearDistance;
    bool mIsEnable;
    bool mIsOnlyFromWaitAfter;
    s32 mTurnEndStep;
};

static_assert(sizeof(NpcStateTurnParam) == 0x18);
