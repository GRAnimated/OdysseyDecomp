#include "Player/PlayerJointControlKeeper.h"

#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Model/ModelShapeUtil.h"

#include "Util/ObjUtil.h"

namespace {
const sead::Vector2f cLookAtLimitX(-60.0f, 60.0f);
const sead::Vector2f cLookAtLimitY(-10.0f, 50.0f);
}  // namespace

PlayerJointControlKeeper::PlayerJointControlKeeper(al::LiveActor* player,
                                                   const PlayerConst* playerConst,
                                                   bool isNeedShrinkNose, bool isEnableBigEar,
                                                   bool isFollowJoeStrap)
    : mPlayer(player), mCapThrow(nullptr), mGrabDynamics(nullptr), mGrabPose(nullptr),
      mGroundPose(nullptr), mLookAt(nullptr), mPartsDynamics(nullptr), mPartsFitting(nullptr),
      mPoleClimbPose(nullptr), mSwimPose(nullptr), mCapThrowParam(nullptr), mGrabParam(nullptr),
      mHandLegAngleParam(nullptr), mCenterDynamicsParam(nullptr), mSwimParam(nullptr),
      mGroundPoseParam(nullptr), mGroundPoseRate(0.0f), mBodyPoseRate(0.0f), mTurnTiltRate(0.0f),
      mCapDynamicsRate(0.0f), mNoseDynamicsRate(0.0f), mIsLookAtEnabled(false),
      mIsLookAtTargetPosition(false), _96(false), mIsGrabCeilPose(false),
      mLookAtTargetSensor(nullptr), _a0{}, mLookAtTargetPosition(0.0f, 0.0f, 0.0f), _b8(0),
      mJointIndexHead(0), mCapThrowMtx(sead::Matrix34f::ident) {
    al::initJointControllerKeeper(mPlayer, 7);

    mSwimParam = new PlayerJointParamSwim{};
    mSwimPose = new PlayerJointControlSwimPose(mPlayer);
    al::registerJointController(mPlayer, mSwimPose);

    mCenterDynamicsParam = new PlayerJointParamCenterDynamics{
        false, false, false, false, 0.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
        sead::Vector3f()};

    mGroundPoseParam = new PlayerJointParamGroundPose{};
    mGroundPose = new PlayerJointControlGroundPose(mPlayer);
    al::registerJointController(mPlayer, mGroundPose);

    mCapThrowParam = new PlayerJointParamCapThrow{};
    mCapThrow = new PlayerJointControlCapThrow(mPlayer, &mCapThrowMtx);
    al::registerJointController(mPlayer, mCapThrow);

    auto* lookAt = al::initJointLookAtController(mPlayer, 1);
    const al::LiveActor* lookAtPlayer = mPlayer;
    mLookAt = lookAt;
    sead::Vector3f lookAtUp = -sead::Vector3f::ey;
    al::appendJointLookAtController(lookAt, lookAtPlayer, "Head", 0.15f, cLookAtLimitX,
                                    cLookAtLimitY, sead::Vector3f::ex, lookAtUp);

    mHandLegAngleParam =
        new PlayerJointParamHandLegAngle{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 1.0f};

    mPoleClimbPose = new PlayerJointControlPoleClimbPose(mPlayer);
    al::registerJointController(mPlayer, mPoleClimbPose);

    auto* grabParam = new PlayerJointParamGrab;
    grabParam->dynamicsRate = 0.0f;
    grabParam->dynamicsFollowRate = 1.0f;
    grabParam->poseRate = 0.0f;
    grabParam->direction.set(0.0f, 1.0f, 0.0f);
    grabParam->interpolateRate = 0.0f;
    grabParam->isEnable = false;
    mGrabParam = grabParam;

    mGrabDynamics = new PlayerJointControlGrabDynamics(mPlayer, playerConst);
    mGrabPose = new PlayerJointControlGrabPose(mPlayer);
    al::registerJointController(mPlayer, mGrabPose);

    mPartsDynamics = new PlayerJointControlPartsDynamics(
        mPlayer, playerConst, isNeedShrinkNose, isEnableBigEar, isFollowJoeStrap, 0);
    mPartsFitting = new PlayerJointControlPartsFitting(mPlayer);
    mPartsFitting->initByHostResource();

    mJointIndexHead = al::getJointIndex(mPlayer->getModelKeeper(), "Head");
}

// NON_MATCHING: current 368-byte composed update has the target's 11/11 semantic call sequence;
// its +4 bytes come from inlined updateSwimPose using two scalar source loads instead of the target
// LDP. Next source-level hypothesis: recover the original 16-byte swim-parameter copy shape.
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

// NON_MATCHING: current 356 bytes versus target 364 with the same 6/6 semantic call sequence;
// target keeps X20 as &mLookAt across the sensor path. Next source-level hypothesis: recover the
// original alias/control lifetime that preserves that address hoist without duplicating cleanup.
void PlayerJointControlKeeper::updateLookAt() {
    if ((mIsLookAtEnabled || mIsLookAtTargetPosition) && !_96) {
        if (mIsLookAtTargetPosition) {
            mLookAt->requestJointLookAt(mLookAtTargetPosition);
            mLookAt->set_52(true);
        } else {
            if (!mLookAtTargetSensor) {
                mLookAt->updateRequestState();
                _96 = false;
                return;
            }

            auto* lookAt = &mLookAt;
            if (_b8 <= 39) {
                (*lookAt)->set_52(true);
                sead::Vector3f targetPos(0.0f, 0.0f, 0.0f);
                rs::calcLookAtSensorTargetPos(&targetPos, mLookAtTargetSensor);
                (*lookAt)->requestJointLookAt(targetPos);
                (*lookAt)->updateRequestState();
                _b8 = al::converge(_b8, 40, 1);
                _96 = false;
                return;
            }

            (*lookAt)->set_52(false);
            sead::Vector3f eyePos;
            calcLookAtEyePos(&eyePos);
            sead::Vector3f targetPos(0.0f, 0.0f, 0.0f);
            if (rs::tryCalcLookAtTargetInDistance(&targetPos, mLookAtTargetSensor, eyePos,
                                                  400.0f)) {
                (*lookAt)->requestJointLookAt(targetPos);
                (*lookAt)->updateRequestState();
                _b8 = al::converge(_b8, 40, 1);
                _96 = false;
                return;
            }
        }
    } else {
        mLookAt->set_52(false);
    }

    mLookAt->updateRequestState();
    mLookAtTargetSensor = nullptr;
    _b8 = -1;
    _96 = false;
}

// NON_MATCHING: target uses one LDP and two unaligned STURs (24 bytes), while the current const-ref
// aggregate copy emits two LDRs (28 bytes); by-value, scalar, local-copy, and alignment variants
// were rejected. Next source-level hypothesis: recover the original same-TU copy shape that proves
// non-overlap and enables the paired source load.
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

