#pragma once

#include <math/seadVector.h>

#include "Library/Anim/DitherAnimator.h"

namespace al {
class LiveActor;
class Resource;

// TODO: Finish this
class ActorDitherAnimator : public DitherAnimator {
public:
    static ActorDitherAnimator* tryCreate(LiveActor*, const Resource*, const char*);

    ActorDitherAnimator(LiveActor* host);

    void update() override;
    void validateDitherAnim();
    void invalidateDitherAnim();
    void reset();
    f32 getDitherAlpha() const;
    void setClippingJudgeDistanceParam(const char* name);
    void resetClippingJudgeDistanceParam();
    void initSphereByProgram(f32, bool);
    void initSubJudgeTableByProgram(bool);
    void initSubJudgeBoundingBoxByProgram(const char*, const sead::Vector3f&,
                                          const sead::Vector3f&);

private:
    unsigned char padding[0x68 - sizeof(DitherAnimator)];
};

static_assert(sizeof(ActorDitherAnimator) == 0x68);
}  // namespace al
