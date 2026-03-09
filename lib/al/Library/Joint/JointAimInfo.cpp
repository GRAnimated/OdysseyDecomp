#include "Library/Joint/JointAimInfo.h"

#include <cmath>
#include <math/seadQuat.h>

#include "Library/Math/MathUtil.h"

namespace al {

JointAimInfo::JointAimInfo() = default;

// NON_MATCHING: register allocation differences (x21/x22 swap)
void JointAimInfo::makeTurnQuat(sead::Quatf* quat, const sead::Vector3f& targetDir) const {
    sead::Vector3f dir;
    tryNormalizeOrZero(&dir, targetDir);

    if (!isNearZero(mBaseOffsetLocal)) {
        f32 dot1 = mBaseOffsetLocal.dot(mBaseAimLocalDir);
        f32 aimLenSq = mBaseAimLocalDir.dot(mBaseAimLocalDir);
        f32 offsetLenSq = mBaseOffsetLocal.dot(mBaseOffsetLocal);
        f32 targetLenSq = targetDir.dot(targetDir);
        f32 discriminant = dot1 * dot1 - (offsetLenSq - targetLenSq) * aimLenSq;

        f32 t;
        if (discriminant >= 0.0f) {
            f32 sqrtDisc = sqrtf(discriminant);
            f32 t1 = (sqrtDisc - dot1) / aimLenSq;
            f32 t2 = (-dot1 - sqrtDisc) / aimLenSq;
            f32 tMax = t1 > t2 ? t1 : t2;
            t = fmaxf(tMax, 0.0f);
        } else {
            t = 0.0f;
        }

        sead::Vector3f adjusted = mBaseAimLocalDir * t + mBaseOffsetLocal;
        tryNormalizeOrZero(&adjusted);

        sead::Quatf rotRate;
        makeQuatRotationRate(&rotRate, adjusted, mBaseAimLocalDir, 1.0f);
        rotateVectorQuat(&dir, rotRate);
    }

    switch (_60) {
    case 2: {
        f32 angleUp = calcAngleOnPlaneDegree(mBaseAimLocalDir, dir, mBaseSideLocalDir);
        f32 angleSide = calcAngleOnPlaneDegree(mBaseAimLocalDir, dir, mBaseUpLocalDir);

        f32 clampedUp = angleUp;
        if (clampedUp > _58)
            clampedUp = _58;
        if (clampedUp < -_5c)
            clampedUp = -_5c;

        f32 clampedSide = angleSide;
        if (clampedSide > _50)
            clampedSide = _50;
        if (clampedSide < -_54)
            clampedSide = -_54;

        *quat = sead::Quatf{0.0f, 0.0f, 0.0f, 1.0f};
        rotateQuatRadian(quat, *quat, mBaseSideLocalDir, clampedUp * sead::Mathf::deg2rad(1.0f));
        rotateQuatRadian(quat, *quat, mBaseUpLocalDir, clampedSide * sead::Mathf::deg2rad(1.0f));
        return;
    }
    case 1:
        makeTurnQuatOval(quat, dir);
        return;
    case 0:
        *quat = sead::Quatf{0.0f, 0.0f, 0.0f, 1.0f};
        turnQuat(quat, *quat, mBaseAimLocalDir, dir, _50 * sead::Mathf::deg2rad(1.0f));
        return;
    }
}

void JointAimInfo::makeTurnQuatCircle(sead::Quatf* quat, const sead::Vector3f& targetDir) const {
    *quat = sead::Quatf{0.0f, 0.0f, 0.0f, 1.0f};
    turnQuat(quat, *quat, mBaseAimLocalDir, targetDir, _50 * sead::Mathf::deg2rad(1.0f));
}

// NON_MATCHING: minor epilog ordering difference
void JointAimInfo::makeTurnQuatOval(sead::Quatf* quat, const sead::Vector3f& targetDir) const {
    f32 dotSide = targetDir.dot(mBaseSideLocalDir);
    f32 dotUp = targetDir.dot(mBaseUpLocalDir);
    f32 dotSideSq = dotSide * dotSide;
    f32 dotUpSq = dotUp * dotUp;
    f32 sum = dotSideSq + dotUpSq;

    if (isNearZero(sum)) {
        *quat = sead::Quatf{0.0f, 0.0f, 0.0f, 1.0f};
        return;
    }

    const f32* limitSide = dotSide > 0.0f ? &_50 : &_54;
    const f32* limitUp = dotUp > 0.0f ? &_58 : &_5c;
    f32 limitSideSq = *limitSide * *limitSide;
    f32 limitUpSq = *limitUp * *limitUp;

    f32 angle;
    if (isNearZero(limitSideSq) && isNearZero(limitUpSq)) {
        angle = 0.0f;
    } else {
        angle = sqrtf(limitSideSq * limitUpSq /
                      (dotUpSq / sum * limitSideSq + dotSideSq / sum * limitUpSq)) *
                sead::Mathf::deg2rad(1.0f);
    }

    *quat = sead::Quatf{0.0f, 0.0f, 0.0f, 1.0f};
    turnQuat(quat, *quat, mBaseAimLocalDir, targetDir, angle);
}

void JointAimInfo::makeTurnQuatRect(sead::Quatf* quat, const sead::Vector3f& targetDir) const {
    f32 angleUp = calcAngleOnPlaneDegree(mBaseAimLocalDir, targetDir, mBaseSideLocalDir);
    f32 angleSide = calcAngleOnPlaneDegree(mBaseAimLocalDir, targetDir, mBaseUpLocalDir);

    f32 clampedUp = angleUp;
    if (clampedUp > _58)
        clampedUp = _58;
    if (clampedUp < -_5c)
        clampedUp = -_5c;

    f32 clampedSide = angleSide;
    if (clampedSide > _50)
        clampedSide = _50;
    if (clampedSide < -_54)
        clampedSide = -_54;

    *quat = sead::Quatf{0.0f, 0.0f, 0.0f, 1.0f};
    rotateQuatRadian(quat, *quat, mBaseSideLocalDir, clampedUp * sead::Mathf::deg2rad(1.0f));
    rotateQuatRadian(quat, *quat, mBaseUpLocalDir, clampedSide * sead::Mathf::deg2rad(1.0f));
}

void JointAimInfo::setBaseAimLocalDir(const sead::Vector3f& dir) {
    mBaseAimLocalDir.set(dir);
}

void JointAimInfo::setBaseUpLocalDir(const sead::Vector3f& dir) {
    mBaseUpLocalDir.set(dir);
}

void JointAimInfo::setBaseSideLocalDir(const sead::Vector3f& dir) {
    mBaseSideLocalDir.set(dir);
}

void JointAimInfo::setBaseOffsetLocal(const sead::Vector3f& offset) {
    mBaseOffsetLocal.set(offset);
}

void JointAimInfo::setBaseMtxPtr(const sead::Matrix34f* mtxPtr) {
    mBaseMtxPtr = mtxPtr;
}

void JointAimInfo::setTargetPos(const sead::Vector3f& pos) {
    mTargetPos.set(pos);
}

void JointAimInfo::setPowerRate(f32 rate) {
    mPowerRate = sead::Mathf::clamp(rate, 0.0f, 1.0f);
}

void JointAimInfo::setLimitDegreeCircle(f32 degree) {
    _50 = degree;
    _54 = degree;
    _58 = degree;
    _5c = degree;
    _60 = 0;
}

void JointAimInfo::setLimitDegreeOval(f32 a, f32 b, f32 c, f32 d) {
    _50 = a;
    _54 = b;
    _58 = c;
    _5c = d;
    _60 = 1;
}

void JointAimInfo::setLimitDegreeRect(f32 a, f32 b, f32 c, f32 d) {
    _50 = a;
    _54 = b;
    _58 = c;
    _5c = d;
    _60 = 2;
}

void JointAimInfo::setEnableBackAim(bool enable) {
    mIsEnableBackAim = enable;
}

void JointAimInfo::addPowerRate(f32 rate) {
    mPowerRate = sead::Mathf::clamp(mPowerRate + rate, 0.0f, 1.0f);
}

void JointAimInfo::subPowerRate(f32 rate) {
    mPowerRate = sead::Mathf::clamp(mPowerRate - rate, 0.0f, 1.0f);
}

void JointAimInfo::setInterpoleRate(f32 rate) {
    mInterpoleRate = rate;
}

}  // namespace al
