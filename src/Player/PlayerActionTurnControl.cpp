#include "Player/PlayerActionTurnControl.h"

#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>

#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"

#include "Player/PlayerActionFunction.h"

PlayerActionTurnControl::PlayerActionTurnControl(al::LiveActor* player) : mPlayer(player) {
    _38.allocBuffer(5, static_cast<sead::Heap*>(nullptr), 8);
}

void PlayerActionTurnControl::setup(f32 turnAngleStart, f32 turnAngleFast, f32 turnAngleLimit,
                                    f32 turnAngleFastLimit, s32 turnAccelFrame,
                                    s32 turnAccelFrameFast, s32 turnBrakeFrame) {
    mTurnAngleStart = turnAngleStart;
    mTurnAngleFast = turnAngleFast;
    mTurnAngleLimit = turnAngleLimit;
    mTurnAngleFastLimit = turnAngleFastLimit;
    mTurnAccelFrame = turnAccelFrame;
    mTurnAccelFrameFast = turnAccelFrameFast;
    mTurnBrakeFrame = turnBrakeFrame;
}

void PlayerActionTurnControl::reset() {
    _38.clear();
    _30 = 0.0f;
    _8 = 0.0f;
    _24 = {0.0f, 0.0f, 0.0f};
    _18 = {0.0f, 0.0f, 0.0f};
    _c = {0.0f, 0.0f, 0.0f};
}

// NON_MATCHING: complete 0xb30 implementation versus target 0xb10; remaining differences are source-level control-flow and vector/quaternion scheduling after the history/turn-axis decisions; next hypothesis is matching the corpus branch-local lifetime split around the two rotation paths.
void PlayerActionTurnControl::update(const sead::Vector3f& input, const sead::Vector3f& up) {
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    al::calcFrontDir(&front, mPlayer);

    sead::Vector3f actorUp = {0.0f, 0.0f, 0.0f};
    al::calcUpDir(&actorUp, mPlayer);
    al::alongVectorNormalH(&front, front, actorUp, up);
    al::tryNormalizeOrZero(&front);
    _50 = front;

    sead::Vector3f previousDir = {0.0f, 0.0f, 0.0f};
    if (!al::isNearZero(_c, 0.001f) || !al::isNearZero(_24, 0.001f)) {
        al::alongVectorNormalH(&previousDir, _c, _24, up);
        al::tryNormalizeOrZero(&previousDir);
    }

    const sead::Vector3f gravity = al::getGravity(mPlayer);
    sead::Vector3f inputDir = {0.0f, 0.0f, 0.0f};
    const bool hasInput = al::tryNormalizeOrZero(&inputDir, input);
    const bool isSteep = actorUp.dot(-gravity) < 0.087156f && _89;
    const bool isFrontZero = al::isNearZero(front, 0.001f);
    const bool isPreviousZero = al::isNearZero(previousDir, 0.001f);
    const bool hasPrevious = hasInput && !isPreviousZero;
    const bool hasNoHistory = isPreviousZero || !hasInput;

    if (hasNoHistory || inputDir.dot(previousDir) < 0.34202f) {
        _38.clear();
        _30 = 0.0f;
    } else {
        f32 angle = 0.0f;
        if (hasPrevious) {
            angle = al::calcAngleDegree(previousDir, inputDir);
            if (angle <= 0.0f)
                angle = -angle;
        }
        _38.forcePushBack(angle);
        _30 = 0.0f;
        const s32 count = _38.size() <= 5 ? _38.size() : 5;
        for (s32 i = 0; i < count; i++)
            _30 += _38(_38.size() - 1 - i);
        _30 /= 5.0f;
    }

    bool useFrontOnOpposite = false;
    bool keepInputOnOpposite = false;
    bool canTurn = hasInput;
    if (!isSteep && !isFrontZero && hasInput) {
        const bool isOpposite = PlayerActionFunction::isOppositeDir(inputDir, front);
        if ((hasNoHistory || (hasPrevious && _30 <= 1.0f)) && isOpposite) {
            keepInputOnOpposite = _88;
            useFrontOnOpposite = !_88;
            canTurn = hasInput && keepInputOnOpposite;
        }
    }
    _69 = keepInputOnOpposite;
    _68 = useFrontOnOpposite;
    _6a = isSteep;

    sead::Vector3f turnFront = {0.0f, 0.0f, 0.0f};
    if (canTurn && !isSteep && !isFrontZero) {
        const sead::Vector3f& angleBase = hasPrevious ? previousDir : front;
        const f32 turnAngle = sead::Mathf::acos(sead::Mathf::clamp(angleBase.dot(inputDir), -1.0f,
                                                                  1.0f));
        const f32 frontAngle = sead::Mathf::acos(
            sead::Mathf::clamp(front.dot(inputDir), -1.0f, 1.0f));

        sead::Vector3f turnAxis = {0.0f, 0.0f, 0.0f};
        turnAxis.setCross(previousDir, inputDir);
        al::tryNormalizeOrZero(&turnAxis);

        bool forceAxisTurn = false;
        bool preserveAxisSign = false;
        if (hasPrevious && !al::isNearZero(_18, 0.001f)) {
            const f32 turnAngleDegree = sead::Mathf::rad2deg(turnAngle);
            if (turnAxis.dot(_18) <= 0.0f) {
                const f32 threshold = mTurnAngleStart <= _8 ? _8 : mTurnAngleStart;
                if (turnAngleDegree >= threshold) {
                    _8 = 0.0f;
                } else {
                    _8 = al::converge(_8, 0.0f, turnAngleDegree);
                    preserveAxisSign = turnAngleDegree < mTurnAngleStart;
                }
            } else {
                sead::Vector3f frontToInput = {0.0f, 0.0f, 0.0f};
                frontToInput.setCross(front, inputDir);
                if (al::tryNormalizeOrZero(&frontToInput) && frontToInput.dot(turnAxis) < 0.0f) {
                    forceAxisTurn = _30 > 10.0f || turnAngleDegree > 3.0f;
                    preserveAxisSign = _30 <= 10.0f && turnAngleDegree <= 3.0f;
                }
            }
        }

        const f32 frontAngleDegree = sead::Mathf::rad2deg(frontAngle);
        f32 inputRate = al::calcRate01(input.length(), 0.0f, 0.8f);
        inputRate = sead::Mathf::clamp(inputRate, 0.25f, 1.0f);
        if (frontAngleDegree < mTurnAngleStart) {
            _8 = PlayerActionFunction::brake(_8, mTurnBrakeFrame, mTurnAngleLimit);
        } else if (frontAngleDegree >= mTurnAngleFast) {
            _8 = PlayerActionFunction::accel(
                _8, mTurnAngleFastLimit, mTurnAngleFastLimit / static_cast<f32>(mTurnAccelFrameFast));
        } else {
            const f32 targetPower = inputRate * mTurnAngleLimit;
            if (_8 >= targetPower)
                _8 = PlayerActionFunction::brake(_8, mTurnBrakeFrame, mTurnAngleFastLimit);
            else
                _8 = PlayerActionFunction::accel(
                    _8, targetPower, targetPower / static_cast<f32>(mTurnAccelFrame));
        }

        sead::Vector3f storedAxis = {0.0f, 0.0f, 0.0f};
        if (forceAxisTurn || al::isReverseDirection(front, inputDir, 0.01f)) {
            storedAxis = up;
            if (turnAxis.dot(storedAxis) <= 0.0f)
                storedAxis = -storedAxis;
            sead::Quatf quat = sead::Quatf::unit;
            quat.setAxisAngle(storedAxis, _8);
            turnFront = front;
            turnFront.rotate(quat);
            al::tryNormalizeOrZero(&turnFront);
        } else {
            sead::Quatf quat = sead::Quatf::unit;
            al::makeQuatRotationLimit(&quat, front, inputDir, sead::Mathf::deg2rad(_8));
            turnFront = front;
            turnFront.rotate(quat);
            al::normalize(&turnFront);
            storedAxis.setCross(front, inputDir);
            al::tryNormalizeOrZero(&storedAxis);
            if (preserveAxisSign && storedAxis.dot(_18) < 0.0f)
                storedAxis = -storedAxis;
        }

        _18 = storedAxis;
        _c = inputDir;
        _24 = up;
    } else if (hasInput) {
        if (useFrontOnOpposite) {
            turnFront = front;
            _8 = 0.0f;
        } else {
            turnFront = inputDir;
            if (isSteep)
                _8 = mTurnAngleLimit;
        }
    } else if (isSteep) {
        al::calcFrontDir(&turnFront, mPlayer);
        if (actorUp.dot(-gravity) < 0.0f)
            turnFront = -turnFront;
        _24 = sead::Vector3f::zero;
        _18 = sead::Vector3f::zero;
        _c = sead::Vector3f::zero;
        _30 = 0.0f;
        _8 = mTurnAngleLimit;
    } else {
        if (isSteep || isFrontZero)
            al::calcFrontDir(&turnFront, mPlayer);
        else
            turnFront = front;
        _24 = sead::Vector3f::zero;
        _18 = sead::Vector3f::zero;
        _c = sead::Vector3f::zero;
        _30 = 0.0f;
        _8 = PlayerActionFunction::brake(_8, mTurnBrakeFrame, mTurnAngleFast);
    }

    _5c = turnFront;
}

void PlayerActionTurnControl::calcTilt(sead::Vector3f* out, const sead::Vector3f& front,
                                       f32 degree) {
    const sead::Vector3f axis = _5c;
    const f32 power = calcTurnPowerRate(front);
    sead::Quatf quat = sead::Quatf::unit;
    quat.setAxisAngle(axis, power * degree);
    out->setRotated(quat, front);
}

f32 PlayerActionTurnControl::calcTurnPowerRate(const sead::Vector3f& front) const {
    f32 rate = al::easeIn(al::calcRate01(_8, mTurnAngleStart, mTurnAngleLimit));
    if (front.dot(_18) > 0.0f)
        rate = -rate;
    return rate;
}
