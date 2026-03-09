#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
class StageSyncCounter;

class ActorVariableFrameRateSkeletalAnimator {
public:
    ActorVariableFrameRateSkeletalAnimator(LiveActor*, const StageSyncCounter*);
    void preMovement(LiveActor*);

private:
    void* _0 = nullptr;
    s32 _8 = 6;
    const StageSyncCounter* mSyncCounter = nullptr;
    u32 _18 = 0;
};

}  // namespace al
