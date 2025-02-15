#pragma once

#include <math/seadVector.h>

#include "Library/Nerve/NerveExecutor.h"

namespace al {
class CameraPoser;

class CameraVerticalAbsorber2DGalaxy : public NerveExecutor {
public:
    CameraVerticalAbsorber2DGalaxy();
    void start(const CameraPoser* poser);
    void update(const CameraPoser* poser);
    void applyLimit(sead::Vector3f* limit) const;
    void exeNone();
    void exeGround();
    void exeLimit();
    void exeLimitOver();
    void exeLimitAfter();

private:
    sead::Vector3f mTargetTrans = sead::Vector3f(0.0f, 0.0f, 0.0f);
    sead::Vector3f mTargetGravity = sead::Vector3f(0.0f, 0.0f, 0.0f);
    sead::Vector3f mTargetUp = sead::Vector3f(0.0f, 0.0f, 0.0f);
    bool mIsTargetCollideGround = false;
    sead::Vector3f mPrevTargetTrans = sead::Vector3f(0.0f, 0.0f, 0.0f);
    sead::Vector3f mPrevTargetGravity = sead::Vector3f(0.0f, 0.0f, 0.0f);
    f32 unkFloat = 0.0f;
    sead::Vector3f mLimit = sead::Vector3f(0.0f, 0.0f, 0.0f);
    sead::Vector3f unkVec = sead::Vector3f(0.0f, 0.0f, 0.0f);
    f32 unkFloat2 = 0.0f;
};
}  // namespace al
