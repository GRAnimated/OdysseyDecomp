#include "Npc/NpcStateTurnSeparate.h"

#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

struct TurnDirInfo {
    const char* mActionName;
    f32 mCenterAngle;
    f32 mTargetAngle;
};

namespace {
NERVE_IMPL(NpcStateTurnSeparate, Turn);
NERVES_MAKE_NOSTRUCT(NpcStateTurnSeparate, Turn);

const TurnDirInfo sTurnL90 = {"TurnL90", 90.0f, 90.0f};
const TurnDirInfo sTurnR90 = {"TurnR90", -90.0f, -90.0f};
const TurnDirInfo sTurn180Plus = {"Turn180", 180.0f, 180.0f};
const TurnDirInfo sTurn180Minus = {"Turn180", -180.0f, -180.0f};
}  // namespace

NpcStateTurnSeparate::NpcStateTurnSeparate(const char* name, al::LiveActor* actor)
    : NpcStateTurnBase(name, actor) {
    initNerve(&Turn, 0);
}

void NpcStateTurnSeparate::startTurnToTarget(const sead::Vector3f& target) {
    const sead::Vector3f& trans = al::getTrans(mActor);
    sead::Vector3f dir = target - trans;
    sead::Vector3f up = {0, 0, 0};
    al::calcUpDir(&up, mActor);
    al::verticalizeVec(&dir, up, dir);
    al::normalize(&dir);
    tryStartTurnToDir(dir, -1.0f);
}

bool NpcStateTurnSeparate::tryStartTurnToTarget(const sead::Vector3f& target, f32 threshold) {
    const sead::Vector3f& trans = al::getTrans(mActor);
    sead::Vector3f dir = target - trans;
    sead::Vector3f up = {0, 0, 0};
    al::calcUpDir(&up, mActor);
    al::verticalizeVec(&dir, up, dir);
    al::normalize(&dir);
    return tryStartTurnToDir(dir, threshold);
}

void NpcStateTurnSeparate::startTurnToDir(const sead::Vector3f& dir) {
    tryStartTurnToDir(dir, -1.0f);
}

// NON_MATCHING: csel vs branch for direction selection (regalloc); 90% match
bool NpcStateTurnSeparate::tryStartTurnToDir(const sead::Vector3f& dir, f32 threshold) {
    sead::Vector3f frontDir = {0, 0, 0};
    sead::Vector3f upDir = {0, 0, 0};
    sead::Vector3f targetDir = dir;

    al::calcPoseDir(nullptr, &upDir, &frontDir, mActor);
    al::verticalizeVec(&targetDir, upDir, targetDir);
    if (!al::tryNormalizeOrZero(&targetDir))
        return false;

    f32 angle = al::calcAngleOnPlaneDegree(frontDir, targetDir, upDir);
    if (threshold > 0.0f) {
        if (sead::Mathf::abs(angle) < threshold)
            return false;
    }

    const TurnDirInfo* chosen = angle > 0.0f ? &sTurn180Plus : &sTurn180Minus;

    if (al::sign(angle) == al::sign(90.0f)) {
        f32 distCenter = angle - chosen->mCenterAngle;
        f32 dist90 = angle + -90.0f;
        if (distCenter * distCenter >= dist90 * dist90)
            chosen = &sTurnL90;
    }

    if (al::sign(angle) == al::sign(-90.0f)) {
        f32 distCenter = angle - chosen->mCenterAngle;
        f32 dist90 = angle + 90.0f;
        if (distCenter * distCenter >= dist90 * dist90)
            chosen = &sTurnR90;
    }

    mTurnDirInfo = chosen;
    mFrontDir.set(frontDir);
    mUpDir.set(upDir);
    mAngle = al::calcAngleOnPlaneDegree(mFrontDir, targetDir, mUpDir);
    if (!al::isSameSign(mAngle, mTurnDirInfo->mTargetAngle))
        mAngle += al::sign(mAngle) * -360.0f;

    al::setNerve(this, &Turn);
    appear();
    return true;
}

void NpcStateTurnSeparate::exeTurn() {
    if (al::isFirstStep(this))
        startActionLocal(mTurnDirInfo->mActionName);

    sead::Vector3f rotDir = {0, 0, 0};
    f32 frameMax = al::getActionFrameMax(mActor, al::getActionName(mActor));
    f32 angle = mAngle;
    const sead::Vector3f& frontDir = mFrontDir;
    const sead::Vector3f& upDir = mUpDir;
    f32 rate = al::calcNerveRate(this, (s32)frameMax);
    al::rotateVectorDegree(&rotDir, frontDir, upDir, angle * rate);
    al::normalize(&rotDir);

    sead::Quatf quat = sead::Quatf::unit;
    al::makeQuatFrontUp(&quat, rotDir, upDir);
    al::updatePoseQuat(mActor, quat);

    if (al::isActionEnd(mActor))
        kill();
}
