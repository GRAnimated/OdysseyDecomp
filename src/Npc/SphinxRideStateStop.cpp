#include "Npc/SphinxRideStateStop.h"

#include <math/seadMathCalcCommon.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/SphinxRide.h"
#include "Npc/SphinxRideParam.h"
#include "Util/CameraUtil.h"
#include "Util/PlayerPuppetFunction.h"

namespace {
NERVE_IMPL(SphinxRideStateStop, StopStart);
NERVE_IMPL(SphinxRideStateStop, StopSwing);
NERVE_IMPL(SphinxRideStateStop, StopButton);
NERVES_MAKE_NOSTRUCT(SphinxRideStateStop, StopStart, StopSwing, StopButton);

// NON_MATCHING: getPuppetMoveStick HFA return convention, fmin vs fminnm, fcsel hi vs gt
f32 calcStopInputSpeed(SphinxRide* sphinx, IUsePlayerPuppet* puppet) {
    sead::Vector3f frontDir;
    sead::Vector3f viewInput;
    al::calcFrontDir(&frontDir, sphinx);

    const sead::Vector2f& stick = rs::getPuppetMoveStick(puppet);
    al::calcVecViewInput(&viewInput, stick, sead::Vector3f::ey,
                         al::getViewMtxPtr(sphinx, 0));

    f32 angle;
    if (!al::tryCalcAngleDegree(&angle, frontDir, viewInput) || angle <= 30.0f)
        return 1.5f;

    f32 ratio = sead::Mathf::min(angle, 90.0f) / 90.0f;
    f32 speed = ratio * 1.5f + 1.5f;
    f32 magnitude = viewInput.length();
    f32 result = speed * viewInput.length();
    return magnitude > 0.8f ? result : result * 0.5f;
}
}  // namespace

SphinxRideStateStop::SphinxRideStateStop(SphinxRide* host, const SphinxRideParam& param)
    : HostStateBase<SphinxRide>("ストップ状態(スフィンクス)", host), mParam(&param) {
    mIsSwingStop = false;
    mIsGetOff = false;
    mButtonFrame = 0;
    mSwingFrame = 0;
    initNerve(&StopStart, 0);
}

void SphinxRideStateStop::appear() {
    mIsDead = false;
    mIsGetOff = false;
    mButtonFrame = 0;
    mSwingFrame = 0;
    al::setNerve(this, &StopStart);
}

void SphinxRideStateStop::kill() {
    mIsDead = true;
    mIsSwingStop = false;
}

// NON_MATCHING: csel operand order (eq vs ne) for nerve ternary
void SphinxRideStateStop::exeStopStart() {
    if (al::isFirstStep(this)) {
        al::startAction(getHost(), "StopStart");
        rs::startPuppetAction(getHost()->mPlayerPuppet, "SphinxRideStopStart");
        rs::setPuppetAnimRate(getHost()->mPlayerPuppet, 1.0f);
    }

    if (getHost()->tryGetOffAndCancelBind()) {
        mIsGetOff = true;
        kill();
        return;
    }

    bool notWall = !al::isCollidedWallVelocity(getHost());
    f32 speed = calcStopInputSpeed(getHost(), getHost()->mPlayerPuppet);
    getHost()->controlAndRotateYAndMove(speed, 1.0f, notWall);
    getHost()->updateGravityAndDump(mParam->gravBase, mParam->dumpRatio);
    getHost()->trySlipOnMoveLimit();

    SphinxRide* host = getHost();
    f32 maxFrame = al::getActionFrameMax(host, "StopStart");
    f32 easeRate = al::calcNerveEaseInRate(host, (s32)maxFrame);
    f32 lerpResult = al::lerpValue(0.0f, 1.0f, easeRate);
    rs::requestCameraTurnToFront(getHost(), lerpResult * 0.4f, lerpResult * 0.1f, 0);

    if (al::isActionEnd(getHost()))
        al::setNerve(this,
                     mIsSwingStop ? (al::Nerve*)&StopSwing : (al::Nerve*)&StopButton);
}

// NON_MATCHING: kill block placement, cmp canonicalization (0xb2 gt vs 0xb3 ge), eor+tbnz vs tbz
void SphinxRideStateStop::exeStopButton() {
    if (al::isFirstStep(this)) {
        al::startAction(getHost(), "Stop");
        if (getHost()->isValidateCameraAngleV(getHost()->mCameraAngle))
            rs::requestSetCameraAngleV(getHost(), getHost()->mCameraAngle + 0.0f, 0);
        rs::startPuppetAction(getHost()->mPlayerPuppet, "SphinxRideStop");
        rs::setPuppetAnimRate(getHost()->mPlayerPuppet, 1.0f);
    }

    if (getHost()->tryGetOffAndCancelBind()) {
        mIsGetOff = true;
        kill();
        return;
    }

    rs::requestCameraTurnToFront(getHost(), 0.4f, 0.1f, 0);
    f32 speed = calcStopInputSpeed(getHost(), getHost()->mPlayerPuppet);
    bool notWall = !al::isCollidedWallVelocity(getHost());
    getHost()->controlAndRotateYAndMove(speed, 1.0f, notWall);

    f32 gravAngle = mParam->gravBase + (f32)al::getNerveStep(this) * 0.0f / 180.0f;
    getHost()->updateGravityAndDump(gravAngle, mParam->dumpRatio);
    getHost()->trySlipOnMoveLimit();

    if (getHost()->isPlayerInputSwingSphinxStop()) {
        al::setNerve(this, &StopSwing);
        return;
    }

    s32 prevFrame = mButtonFrame++;
    bool holdButton = rs::isPuppetHoldAnyButton(getHost()->mPlayerPuppet);
    if (prevFrame > 178) {
        kill();
        return;
    }
    if (!holdButton)
        kill();
}

bool SphinxRideStateStop::updateStopCommon() {
    if (getHost()->tryGetOffAndCancelBind()) {
        mIsGetOff = true;
        kill();
        return true;
    }
    rs::requestCameraTurnToFront(getHost(), 0.4f, 0.1f, 0);
    return false;
}

void SphinxRideStateStop::exeStopButtonSwing() {
    if (updateStopCommon())
        return;

    updateStopStrong();

    ++mButtonFrame;
    ++mSwingFrame;

    if (getHost()->isPlayerInputSwingSphinxStop())
        mSwingFrame = 0;

    if (!rs::isPuppetHoldAnyButton(getHost()->mPlayerPuppet)) {
        kill();
        return;
    }

    if (mSwingFrame >= 20)
        al::setNerve(this, &StopButton);
}

void SphinxRideStateStop::updateStopStrong() {
    SphinxRide* host = getHost();
    rs::requestCameraTurnToFront(host, 0.4f, 0.1f, 0);
    f32 speed = calcStopInputSpeed(host, host->mPlayerPuppet);
    bool notWall = !al::isCollidedWallVelocity(host);
    host->controlAndRotateYAndMove(speed, 1.0f, notWall);
    host->updateGravityAndDump(0.85f, mParam->dumpRatio);
}

void SphinxRideStateStop::exeStopSwing() {
    if (al::isFirstStep(this)) {
        al::startAction(getHost(), "Stop");
        if (getHost()->isValidateCameraAngleV(getHost()->mCameraAngle))
            rs::requestSetCameraAngleV(getHost(), getHost()->mCameraAngle + 0.0f, 0);
        rs::startPuppetAction(getHost()->mPlayerPuppet, "SphinxRideStop");
        rs::setPuppetAnimRate(getHost()->mPlayerPuppet, 1.0f);
    }

    if (updateStopCommon())
        return;

    updateStopStrong();

    ++mSwingFrame;
    if (getHost()->isPlayerInputSwingSphinxStop()) {
        mSwingFrame = 0;
        return;
    }

    if (mSwingFrame >= 20)
        kill();
}
