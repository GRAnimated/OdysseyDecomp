#pragma once

namespace al {
class LiveActor;
class StageSyncCounter;

class ActorVariableFrameRateSkeletalAnimator {
public:
    ActorVariableFrameRateSkeletalAnimator(LiveActor*, const StageSyncCounter*);
    void preMovement(LiveActor*);
};

}  // namespace al
