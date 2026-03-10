#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}

class CityManRhythmInfo {
public:
    CityManRhythmInfo(al::LiveActor* actor, const u8* data, bool useCustomOffset, f32 beatOffset);

    void update(bool forceReset);
    s32 getAnimId(s32 index);
    f32 getAnimBeat(s32 index);

    al::LiveActor* mActor;
    f32 mBeatOffset;
    f32 mCurrentBeat = -1.0f;
    f32 mPrevBeat = -1.0f;
    void* mEntries = nullptr;
    f32 mAnimBeat = -1.0f;
    s32 mAnimId = -1;
    s32 _28 = -1;
    s32 mAnimCount = -1;
    s32 mAnimIndex = 0;
    bool _34 = false;
    bool _35 = false;
};

static_assert(sizeof(CityManRhythmInfo) == 0x38);
