#include "Player/PlayerJointControlKeeper.h"

#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Model/ModelShapeUtil.h"

#include "Util/ObjUtil.h"

// NON_MATCHING: current 368-byte composed update differs from the 364-byte target only because inlined updateSwimPose uses two scalar source loads instead of the target LDP. Next source-level hypothesis: recover the original 16-byte swim-parameter copy shape; this should remove the same one-instruction delta here.
void PlayerJointControlKeeper::update() {
    updateLookAt();
    updateSwimPose();
    updateCenter();
    updateGroundPose();
    updateCapThrow();
    updatePoleClimb();
    updateGrab();
    updatePartsDynamics();
}

// NON_MATCHING: behavior is corpus-complete, but current structured cleanup emits 384 bytes versus target 364; target shares two distinct request-state cleanup tails and keeps only X20 live on the sensor path. Next source-level hypothesis: recover the original structured control flow that yields those two compiler-merged tails without artificial gotos/attributes.
void PlayerJointControlKeeper::updateLookAt() {
    if ((!mIsLookAtEnabled && !mIsLookAtTargetPosition) || _96) {
        mLookAt->set_52(false);
        mLookAt->updateRequestState();
        mLookAtTargetSensor = nullptr;
        _b8 = -1;
        _96 = false;
        return;
    }

    if (mIsLookAtTargetPosition) {
        mLookAt->requestJointLookAt(mLookAtTargetPosition);
        mLookAt->set_52(true);
        mLookAt->updateRequestState();
        mLookAtTargetSensor = nullptr;
        _b8 = -1;
        _96 = false;
        return;
    }

    if (!mLookAtTargetSensor) {
        mLookAt->updateRequestState();
        _96 = false;
        return;
    }

    if (_b8 <= 39) {
        mLookAt->set_52(true);
        sead::Vector3f targetPos;
        targetPos.set(0.0f, 0.0f, 0.0f);
        rs::calcLookAtSensorTargetPos(&targetPos, mLookAtTargetSensor);
        mLookAt->requestJointLookAt(targetPos);
        mLookAt->updateRequestState();
        _b8 = al::converge(_b8, 40, 1);
        _96 = false;
        return;
    }

    mLookAt->set_52(false);
    sead::Vector3f eyePos;
    calcLookAtEyePos(&eyePos);
    sead::Vector3f targetPos;
    targetPos.set(0.0f, 0.0f, 0.0f);
    if (!rs::tryCalcLookAtTargetInDistance(&targetPos, mLookAtTargetSensor, eyePos, 400.0f)) {
        mLookAt->updateRequestState();
        mLookAtTargetSensor = nullptr;
        _b8 = -1;
        _96 = false;
        return;
    }

    mLookAt->requestJointLookAt(targetPos);
    mLookAt->updateRequestState();
    _b8 = al::converge(_b8, 40, 1);
    _96 = false;
}

// NON_MATCHING: target uses one LDP and two unaligned STURs (0x18), while the current const-ref aggregate copy emits two LDRs (0x1c); by-value, scalar, local-copy, and alignment variants were rejected. Next source-level hypothesis: recover the original same-TU setParam/copy shape that proves non-overlap and enables the paired source load.
void PlayerJointControlKeeper::updateSwimPose() {
    mSwimPose->setParam(*mSwimParam);
}

void PlayerJointControlKeeper::updateCenter() {
    mGroundPose->updateCenterTilt(mCenterDynamicsParam->tiltRate,
                                  mCenterDynamicsParam->tiltDirection);
    mGroundPose->updateCenterBalance(
        mCenterDynamicsParam->isEnable, mCenterDynamicsParam->isReset,
        mCenterDynamicsParam->isUseTilt, mCenterDynamicsParam->balanceDirection,
        mCenterDynamicsParam->balanceUp, mCenterDynamicsParam->isUseBalance);
    PlayerJointParamCenterDynamics* param = mCenterDynamicsParam;
    param->isEnable = false;
    param->isReset = false;
    param->isUseTilt = false;
    param->isUseBalance = false;
    param->tiltRate = 0.0f;
    param->tiltDirection.set(0.0f, 0.0f, 0.0f);
    param->balanceDirection.set(0.0f, 0.0f, 0.0f);
    param->balanceUp.set(0.0f, 0.0f, 0.0f);
}

void PlayerJointControlKeeper::updateGroundPose() {
    if (mIsGrabCeilPose) {
        mGroundPose->resetTiltRate();
        mIsGrabCeilPose = false;
    }

    f32 ikRate = mLookAt->get_52() ? 0.0f : mTurnTiltRate;
    mGroundPose->update(mGroundPoseRate, mBodyPoseRate, mTurnTiltRate, ikRate,
                        mGroundPoseParam->isEnable);
}

void PlayerJointControlKeeper::updateCapThrow() {
    if (mCapThrowParam->isStart) {
        mCapThrow->start(mCapThrowParam->rate, mCapThrowParam->direction,
                         mCapThrowParam->isReverse);
        mCapThrowParam->isStart = false;
    }
    if (mCapThrowParam->isEnd) {
        mCapThrow->forceEnd();
        mCapThrowParam->isEnd = false;
    }
    mCapThrow->update();
}

void PlayerJointControlKeeper::updatePoleClimb() {
    mPoleClimbPose->update(mHandLegAngleParam->handAngle, mHandLegAngleParam->legAngle,
                           mHandLegAngleParam->blendRate);
}

void PlayerJointControlKeeper::updateGrab() {
    mGrabPose->update(mGrabParam->poseRate, mGrabParam->direction, mGrabParam->interpolateRate);
    mGrabDynamics->update(mGrabParam->dynamicsRate, mGrabParam->dynamicsFollowRate);
}

void PlayerJointControlKeeper::updatePartsDynamics() {
    mPartsDynamics->update(mCapDynamicsRate, mNoseDynamicsRate);
}

void PlayerJointControlKeeper::calcLookAtEyePos(sead::Vector3f* out) const {
    al::getJointMtxPtrByIndex(mPlayer->getModelKeeper(), mJointIndexHead)->getTranslation(*out);
}

void PlayerJointControlKeeper::resetPartsDynamics() {
    mPartsDynamics->resetDynamics();
}

void PlayerJointControlKeeper::updateLookAtTarget(al::HitSensor* sensor) {
    if (mLookAtTargetSensor == sensor)
        return;

    sead::Vector3f headPos;
    al::getJointMtxPtrByIndex(mPlayer->getModelKeeper(), mJointIndexHead)->getTranslation(headPos);
    sead::Vector3f targetPos;
    targetPos.set(0.0f, 0.0f, 0.0f);

    if (mLookAtTargetSensor &&
        rs::tryCalcLookAtTargetInDistance(&targetPos, mLookAtTargetSensor, headPos, 400.0f)) {
        const sead::Vector3f currentDelta = targetPos - headPos;
        const sead::Vector3f nextDelta = al::getSensorPos(sensor) - headPos;
        if (currentDelta.squaredLength() <= nextDelta.squaredLength())
            return;
    }

    if (rs::tryCalcLookAtTargetInDistance(&targetPos, sensor, headPos, 400.0f)) {
        mLookAtTargetSensor = sensor;
        _b8 = -1;
    }
}

void PlayerJointControlKeeper::calcGroundPoseUp(sead::Vector3f* out) const {
    mGroundPose->calcGroundPoseUp(out);
}

f32 PlayerJointControlKeeper::getHeadTiltRate() const {
    return mGroundPose->get_1e8();
}

