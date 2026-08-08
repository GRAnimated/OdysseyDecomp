#pragma once

#include <math/seadVector.h>

#include "Library/Anim/DitherAnimator.h"

namespace al {
class ByamlIter;
class IJudgeDistanceCalculator;
class JudgeDistanceCalculatorHolder;
class LiveActor;
class Resource;

class ActorDitherAnimator : public DitherAnimator {
public:
    static ActorDitherAnimator* tryCreate(LiveActor*, const Resource*, const char*);
    ~ActorDitherAnimator() override = default;

    void update() override;
    bool isValidNearClip() const override;

    ActorDitherAnimator(LiveActor* host);

    void loadParam(const ByamlIter& iter);
    void initSphereByProgram(f32 radius, bool isValidFarClip);
    void setOneTimeCtrl();
    void initSubJudgeTableByProgram(s32 count);
    void initSubJudgeBoundingBoxByProgram(const char* name, const sead::Vector3f& size,
                                          const sead::Vector3f& offset);
    void reset();
    f32 getDitherAlpha() const;
    f32 calcJudgeDistance() const;
    IJudgeDistanceCalculator* getJudgeDistanceCalculator() const;
    void setClippingJudgeDistanceParam(const char* name);
    f32 getNearClipStartDistance() const;
    f32 getNearClipEndDistance() const;
    void resetClippingJudgeDistanceParam();
    void setClippingJudgeSphereRadius(f32 radius);
    void setClippingJudgeBoundingBox(const sead::Vector3f& size);
    void setClippingJudgeLocalOffset(const sead::Vector3f& offset);
    void stopAutoCtrl();
    void restartAutoCtrl();
    void validateDitherAnim();
    void invalidateDitherAnim();

private:
    void exeAuto();
    void exeShowStart();
    void exeShow();
    void exeHideStart();
    void exeHide();
    void exeStop();
    void exeValidateChange();
    void exeInvalidateChange();
    JudgeDistanceCalculatorHolder* getJudgeDistanceCalculatorHolder() const;

    unsigned char padding[0x68 - sizeof(DitherAnimator)];
};

static_assert(sizeof(ActorDitherAnimator) == 0x68);
}  // namespace al
