#pragma once

#include <basis/seadTypes.h>

class NpcStateRumbleParam {
public:
    NpcStateRumbleParam(s32 duration, f32 frequency, f32 phaseOffset, f32 amplitude, f32 scale);

    s32 mDuration;
    f32 mFrequency;
    f32 mPhaseOffset;
    f32 mAmplitude;
    f32 mScale;
};

static_assert(sizeof(NpcStateRumbleParam) == 0x14);
