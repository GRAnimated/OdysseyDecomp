#pragma once

#include <math/seadVector.h>

#include "Library/Anim/DitherAnimator.h"

namespace al {
class ByamlIter;
class ClippingActorInfo;
class IJudgeDistanceCalculator;
class JudgeDistanceCalculatorHolder;
class LiveActor;
class Resource;

class ActorDitherAnimator : public DitherAnimator {
public:
    static ActorDitherAnimator* tryCreate(LiveActor*, const Resource*, const char*);

    ActorDitherAnimator(LiveActor* host);

    void initSphereByProgram(f32, bool);
    void setOneTimeCtrl();
    void initSubJudgeTableByProgram(s32);
    void initSubJudgeBoundingBoxByProgram(const char*, const sead::Vector3f&,
                                          const sead::Vector3f&);

    void update() override;
    void reset();
    f32 getDitherAlpha() const;
    f32 calcJudgeDistance() const;
    IJudgeDistanceCalculator* getJudgeDistanceCalculator() const;
    void setClippingJudgeDistanceParam(const char*);
    f32 getNearClipStartDistance() const;
    f32 getNearClipEndDistance() const;
    void resetClippingJudgeDistanceParam();
    void setClippingJudgeSphereRadius(f32);
    void setClippingJudgeBoundingBox(const sead::Vector3f&);
    void setClippingJudgeLocalOffset(const sead::Vector3f&);
    void stopAutoCtrl();
    void restartAutoCtrl();
    void validateDitherAnim();
    void invalidateDitherAnim();

    bool isValidNearClip() const override;

private:
    void loadParam(const ByamlIter&);

    void exeAuto();
    void exeShowStart();
    void exeShow();
    void exeHideStart();
    void exeHide();
    void exeStop();
    void exeValidateChange();
    void exeInvalidateChange();

    JudgeDistanceCalculatorHolder* getJudgeDistanceCalculatorHolder() const;

    LiveActor* mHostActor;
    f32 mDitherAlpha;
    f32 mPrevDitherAlpha;
    f32 mMinNearDitherAlpha;
    f32 mMaxAlpha;
    bool mIsValidFarClip;
    bool mIsValidNearClip;
    bool _2a;
    bool mIsOneTimeCtrl;
    f32 mNearClipStartDistance;
    f32 mNearClipEndDistance;
    f32 mNearClipStartDistancePlayerCamera;
    f32 mNearClipEndDistancePlayerCamera;
    unsigned char _3c[4];
    JudgeDistanceCalculatorHolder* mJudgeDistanceCalculatorHolder;
    JudgeDistanceCalculatorHolder** mSubJudgeCalculatorTable;
    s32 mSubJudgeCalculatorCount;
    s32 mSubJudgeCalculatorCapacity;
    s32 mCurrentSubJudgeCalculator;
    unsigned char _5c[4];
    ClippingActorInfo* mClippingActorInfo;
};

static_assert(sizeof(ActorDitherAnimator) == 0x68);
}  // namespace al
