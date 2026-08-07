#pragma once

#include "Library/Anim/DitherAnimator.h"

namespace al {
class LiveActor;
class Resource;

// TODO: Finish this
class ActorDitherAnimator : public DitherAnimator {
public:
    static ActorDitherAnimator* tryCreate(LiveActor*, const Resource*, const char*);

    ActorDitherAnimator(LiveActor* host);

    void validateDitherAnim();
    void invalidateDitherAnim();
    void reset();
    f32 getDitherAlpha() const;
    void setClippingJudgeDistanceParam(const char* name);
    void resetClippingJudgeDistanceParam();

private:
    unsigned char padding[0x68 - sizeof(DitherAnimator)];
};

static_assert(sizeof(ActorDitherAnimator) == 0x68);
}  // namespace al
