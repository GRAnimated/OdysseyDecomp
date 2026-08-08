#include "Player/PlayerJointControlGroundPose.h"

#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/MatrixUtil.h"

namespace alModelJointFunction {
void calcMtx34JointMtxByIndexRaw(sead::Matrix34f*, al::ModelKeeper*, s32);
}

namespace {
// NON_MATCHING: 612 bytes vs target 656; the SEAD basis-vector rotation preserves the target translation semantics, but current FP register pressure/store scheduling differs; next hypothesis is an owner/helper source form that left-rotates the three basis columns without generic Matrix33 x Matrix34 translation rotation.
void calcTiltMtx(sead::Matrix34f* mtx, const sead::Vector3f& axis,
                 const sead::Vector3f& targetDir, f32 degree, f32 rate, f32 limit) {
    f32 angle = al::calcAngleDegree(sead::Vector3f::ey, targetDir);
    if (sead::Vector3f::ey.cross(targetDir).dot(axis) < 0.0f)
        angle = -angle;

    const f32 targetAngle = sead::Mathf::clamp(degree * rate + angle, -limit, limit);
    sead::Quatf quat = sead::Quatf::unit;
    quat.setAxisAngle(axis, targetAngle - angle);

    sead::Matrix33f rotation;
    rotation.fromQuat(quat);
    for (s32 i = 0; i < 3; i++) {
        sead::Vector3f base;
        mtx->getBase(base, i);
        base.mul(rotation);
        mtx->setBase(i, base);
    }
}
}  // namespace

void PlayerJointControlGroundPose::calcJointCallback(s32 jointIndex, sead::Matrix34f* jointMtx) {
    calcTilt(jointIndex, jointMtx);
    calcCenterTilt(jointIndex, jointMtx);
    calcCenterBalance(jointIndex, jointMtx);
    calcIK(jointIndex, jointMtx);
}

void PlayerJointControlGroundPose::calcTilt(s32 jointIndex, sead::Matrix34f* jointMtx) {
    if (al::isNearZero(_1e4, 0.001f))
        return;

    if (jointIndex == mJointIndexAllRoot)
        calcRootMtxTilt(jointMtx);
    else if (jointIndex == mJointIndexSpine1)
        calcSpineMtxTilt(jointMtx);
    else if (jointIndex == mJointIndexHead)
        calcHeadMtxTilt(jointMtx);
    else if (jointIndex == mJointIndexLegTopL || jointIndex == mJointIndexLegTopR)
        calcLegTopMtxTilt(jointMtx, jointIndex == mJointIndexLegTopR);
}

void PlayerJointControlGroundPose::calcRootMtxTilt(sead::Matrix34f* jointMtx) {
    sead::Vector3f zDir;
    sead::Vector3f yDir;
    jointMtx->getBase(yDir, 1);
    jointMtx->getBase(zDir, 2);
    calcTiltMtx(jointMtx, zDir, yDir, 20.0f, _1e4, 45.0f);
}

void PlayerJointControlGroundPose::calcSpineMtxTilt(sead::Matrix34f* jointMtx) {
    sead::Matrix34f rootMtx = sead::Matrix34f::ident;
    alModelJointFunction::calcMtx34JointMtxByIndexRaw(&rootMtx, mPlayer->getModelKeeper(),
                                                       mJointIndexAllRoot);
    sead::Vector3f zDir;
    sead::Vector3f yDir;
    rootMtx.getBase(yDir, 1);
    rootMtx.getBase(zDir, 2);
    calcTiltMtx(jointMtx, zDir, yDir, 15.0f, _1e4, 50.0f);
}

void PlayerJointControlGroundPose::calcHeadMtxTilt(sead::Matrix34f* jointMtx) {
    al::rotateMtxXDirDegree(jointMtx, *jointMtx, _1e4 * 10.0f);
    al::rotateMtxYDirDegree(jointMtx, *jointMtx, _1e4 * 25.0f);
    al::rotateMtxZDirDegree(jointMtx, *jointMtx, sead::Mathf::abs(_1e4) * -20.0f);
}

void PlayerJointControlGroundPose::calcLegTopMtxTilt(sead::Matrix34f* jointMtx, bool isRight) {
    al::rotateMtxYDirDegree(jointMtx, *jointMtx, _1e4 * -10.0f);
}

f32 PlayerJointControlGroundPose::initCenterBalanceRate() const {
    return 1.0f;
}

f32 PlayerJointControlGroundPose::calcCenterBalanceBlendRate() const {
    return _220 * _1dc;
}

// NON_MATCHING: best clean form is 764 bytes vs target 720 with matching 0x90 frame and behavior; current forward/transpose Matrix33 member stores are scalar where the target pairs adjacent stores. Next hypothesis is the original source form that materializes paired forward/inverse matrix stores without changing quaternion behavior.
void PlayerJointControlGroundPose::calcRootMtx(sead::Matrix34f* jointMtx) {
    sead::Vector3f up = -al::getGravity(mPlayer);

    sead::Vector3f yDir;
    jointMtx->getBase(yDir, 1);
    al::normalize(&yDir);

    sead::Quatf rootQuat = sead::Quatf::unit;
    al::makeQuatRotationLimit(&rootQuat, yDir, up, sead::Mathf::deg2rad(_1dc * 25.0f));
    sead::Matrix33f rootRotation;
    rootRotation.fromQuat(rootQuat);
    _78 = rootRotation;
    _9c.setTranspose(rootRotation);

    sead::Quatf spineQuat = sead::Quatf::unit;
    al::makeQuatRotationLimit(&spineQuat, yDir, up, sead::Mathf::deg2rad(_1dc * 30.0f));
    sead::Matrix33f spineRotation;
    spineRotation.fromQuat(spineQuat);
    _c0 = spineRotation;
    _e4.setTranspose(spineRotation);

    jointMtx->getBase(_54, 3);

    sead::Vector3f xDir;
    jointMtx->getBase(xDir, 0);
    const f32 angle = al::calcAngleDegree(xDir, up) - 90.0f;
    f32 rate = sead::Mathf::clamp(sead::Mathf::abs(angle) / 45.0f, 0.0f, 1.0f);
    if (angle < 0.0f)
        rate = -rate;
    _60 = xDir * (_1dc * (rate * 12.5f));
}

void PlayerJointControlGroundPose::calcHipMtx(sead::Matrix34f* jointMtx) {
    sead::Vector3f trans;
    jointMtx->getBase(trans, 3);
    trans -= _54;
    trans.mul(_78);
    sead::Matrix34CalcCommon<f32>::multiply(*jointMtx, _78, *jointMtx);
    trans += _54;
    trans += _60;
    jointMtx->setBase(3, trans);
}

void PlayerJointControlGroundPose::calcSpineMtx(sead::Matrix34f* jointMtx) {
    sead::Vector3f trans;
    jointMtx->getBase(trans, 3);
    trans -= _54;
    trans.mul(_c0);
    sead::Matrix34CalcCommon<f32>::multiply(*jointMtx, _c0, *jointMtx);
    trans += _54;
    trans += _60;
    jointMtx->setBase(3, trans);
}

void PlayerJointControlGroundPose::update(f32 tiltRate, f32 centerTiltRate,
                                          f32 centerBalanceRate, f32 ikRate,
                                          bool isGroundPose) {
    const f32 centerBalanceStep = (centerBalanceRate - _1e4) * 0.15f;
    if (sead::Mathf::abs(centerBalanceStep) > 0.1f)
        _1e4 = al::converge(_1e4, centerBalanceRate, 0.1f);
    else
        _1e4 += centerBalanceStep;

    const f32 ikStep = (ikRate - _1e8) * 0.15f;
    if (sead::Mathf::abs(ikStep) > 0.1f)
        _1e8 = al::converge(_1e8, ikRate, 0.1f);
    else
        _1e8 += ikStep;

    const f32 convergeStep = !(_1d8 <= tiltRate) ? 0.2f : 0.1f;
    _1d8 = al::converge(_1d8, tiltRate, convergeStep);
    _1dc = al::easeOut(_1d8) * sead::Mathf::clamp(1.0f - _1e4, 0.0f, 1.0f);
    _1e0 = al::converge(_1e0, centerTiltRate, convergeStep);
    _1ec = isGroundPose;
}

void PlayerJointControlGroundPose::resetTiltRate() {
    _1e4 = 0.0f;
    _1e8 = 0.0f;
    _228 = 0.0f;
    _22c = 0.0f;
    _230 = 0.0f;
}

void PlayerJointControlGroundPose::calcGroundPoseUp(sead::Vector3f* up) const {
    al::calcUpDir(up, mPlayer);
    if (!al::isNearZero(_1dc, 0.001f)) {
        up->mul(_e4);
        al::normalize(up);
    }
}

void PlayerJointControlGroundPose::updateCenterTilt(f32 targetRate, const sead::Vector3f& direction) {
    _228 = al::converge(_228, targetRate, 0.2f);

    sead::Vector3f side(0.0f, 0.0f, 0.0f);
    al::calcSideDir(&side, mPlayer);

    const f32 spring = al::calcSpringDumperForce(_22c, _230, 0.5f, 0.25f);
    const f32 directionRate = side.dot(direction);
    const f32 velocity = sead::Mathf::clamp(
        _230 * 0.999f + directionRate * (_228 * 0.15f) + spring * 0.1f, -2.0f, 2.0f);
    _230 = velocity;
    _22c = sead::Mathf::clamp(_22c + velocity * 0.1f, -2.0f, 2.0f);
}

const char* PlayerJointControlGroundPose::getCtrlTypeName() const {
    return "地上姿勢制御";
}
