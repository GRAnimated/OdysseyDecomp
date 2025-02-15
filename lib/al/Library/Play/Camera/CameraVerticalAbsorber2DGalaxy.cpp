#include "Library/Play/Camera/CameraVerticalAbsorber2DGalaxy.h"

#include "Library/Camera/CameraPoserFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveExecutor.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
using namespace al;
NERVE_IMPL(CameraVerticalAbsorber2DGalaxy, None)
NERVE_IMPL(CameraVerticalAbsorber2DGalaxy, Ground)
NERVE_IMPL(CameraVerticalAbsorber2DGalaxy, Limit)
NERVE_IMPL(CameraVerticalAbsorber2DGalaxy, LimitAfter)
NERVE_IMPL(CameraVerticalAbsorber2DGalaxy, LimitOver)

NERVES_MAKE_STRUCT(CameraVerticalAbsorber2DGalaxy, None, Ground, Limit, LimitAfter, LimitOver);
}  // namespace

namespace al {
CameraVerticalAbsorber2DGalaxy::CameraVerticalAbsorber2DGalaxy() : NerveExecutor("2D") {
    initNerve(&NrvCameraVerticalAbsorber2DGalaxy.None, 0);
}

void CameraVerticalAbsorber2DGalaxy::start(const CameraPoser* poser) {
    alCameraPoserFunction::calcTargetTrans(&mTargetTrans, poser);
    alCameraPoserFunction::calcTargetGravity(&mTargetGravity, poser);
    alCameraPoserFunction::calcTargetUp(&mTargetUp, poser);
    mIsTargetCollideGround = alCameraPoserFunction::isTargetCollideGround(poser);
    if (mIsTargetCollideGround)
        setNerve(this, &NrvCameraVerticalAbsorber2DGalaxy.Ground);
    else
        setNerve(this, &NrvCameraVerticalAbsorber2DGalaxy.None);
}

void CameraVerticalAbsorber2DGalaxy::update(const CameraPoser* poser) {
    mPrevTargetTrans.set(mTargetTrans);
    mPrevTargetGravity.set(mTargetGravity);
    alCameraPoserFunction::calcTargetTrans(&mTargetTrans, poser);
    alCameraPoserFunction::calcTargetGravity(&mTargetGravity, poser);
    alCameraPoserFunction::calcTargetUp(&mTargetUp, poser);
    mIsTargetCollideGround = alCameraPoserFunction::isTargetCollideGround(poser);

    updateNerve();
}

void CameraVerticalAbsorber2DGalaxy::applyLimit(sead::Vector3f* limit) const {
    *limit -= (unkFloat * mLimit);
}

void CameraVerticalAbsorber2DGalaxy::exeNone() {
    if (mIsTargetCollideGround)
        setNerve(this, &NrvCameraVerticalAbsorber2DGalaxy.Ground);
    else {
        unkFloat2 = lerpValue(unkFloat2, 0.0f, 0.075f);
        unkFloat = lerpValue(unkFloat, 0.0f, 0.05f);
    }
}

// NON_MATCHING: .dot uses wrong order
void CameraVerticalAbsorber2DGalaxy::exeGround() {
    if (mIsTargetCollideGround) {
        unkFloat2 = lerpValue(unkFloat2, 0.0f, 0.075f);
        unkFloat = lerpValue(unkFloat, 0.0f, 0.05f);
    } else {
        sead::Vector3f gravity = -mPrevTargetGravity;

        unkFloat = (mTargetTrans - mPrevTargetTrans).dot(gravity);
        mLimit = gravity;

        setNerve(this, &NrvCameraVerticalAbsorber2DGalaxy.Limit);
    }
}

// NON_MATCHING: .dot uses wrong order
void CameraVerticalAbsorber2DGalaxy::exeLimit() {
    if (isFirstStep(this))
        unkVec.set(mTargetUp);

    unkFloat += (mTargetTrans - mPrevTargetTrans).dot(mLimit);

    if (mIsTargetCollideGround) {
        unkFloat2 = unkFloat;
        setNerve(this, &NrvCameraVerticalAbsorber2DGalaxy.LimitAfter);

    } else if (unkFloat >= 600.0f) {
        unkFloat2 = unkFloat;
        setNerve(this, &NrvCameraVerticalAbsorber2DGalaxy.LimitOver);

    } else if (calcAngleDegree(unkVec, mTargetUp) > 30.0f) {
        unkFloat2 = unkFloat;
        setNerve(this, &NrvCameraVerticalAbsorber2DGalaxy.LimitOver);
    }
}

void CameraVerticalAbsorber2DGalaxy::exeLimitOver() {
    unkFloat2 = lerpValue(unkFloat2, 0.0f, 0.1f);
    unkFloat = lerpValue(unkFloat, unkFloat2, 0.05f);
    if (mIsTargetCollideGround)
        setNerve(this, &NrvCameraVerticalAbsorber2DGalaxy.LimitAfter);
}

void CameraVerticalAbsorber2DGalaxy::exeLimitAfter() {
    unkFloat2 = lerpValue(unkFloat2, 0.0f, 0.075f);
    unkFloat = lerpValue(unkFloat, unkFloat2, 0.05f);

    if (mIsTargetCollideGround) {
        if (isNearZero(unkFloat, 0.01f))
            setNerve(this, &NrvCameraVerticalAbsorber2DGalaxy.Ground);
    } else
        setNerve(this, &NrvCameraVerticalAbsorber2DGalaxy.Limit);
}

}  // namespace al
