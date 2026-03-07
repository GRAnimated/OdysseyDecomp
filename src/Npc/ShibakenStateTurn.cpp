#include "Npc/ShibakenStateTurn.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
NERVE_IMPL(ShibakenStateTurn, TurnL30);
NERVE_IMPL(ShibakenStateTurn, TurnR30);
NERVE_IMPL(ShibakenStateTurn, TurnL90);
NERVE_IMPL(ShibakenStateTurn, TurnR90);
NERVE_IMPL(ShibakenStateTurn, TurnL180);
NERVE_IMPL(ShibakenStateTurn, TurnR180);
NERVES_MAKE_NOSTRUCT(ShibakenStateTurn, TurnL30, TurnR30, TurnL90, TurnR90, TurnL180, TurnR180);
}  // namespace

ShibakenStateTurn::ShibakenStateTurn(const char* name, al::LiveActor* actor)
    : al::ActorStateBase(name, actor), _20(sead::Quatf::unit), _30(sead::Quatf::unit) {
    initNerve(&TurnL30, 0);
}

// NON_MATCHING: quat copy uses 32-bit ops instead of 64-bit; cross product scheduling;
// nerve selection uses CSEL instead of explicit branches
bool ShibakenStateTurn::tryStartTurn(const sead::Vector3f& targetPos) {
    sead::Vector3f diff = targetPos - al::getTrans(mActor);
    sead::Vector3f upDir(0, 0, 0);
    sead::Vector3f frontDir(0, 0, 0);

    al::calcUpDir(&upDir, mActor);
    al::calcFrontDir(&frontDir, mActor);

    al::verticalizeVec(&diff, upDir, diff);
    if (!al::tryNormalizeOrZero(&diff))
        return false;
    if (al::isNearDirection(frontDir, diff, 0.01f))
        return false;

    _20 = al::getQuat(mActor);
    al::makeQuatFrontUp(&_30, diff, upDir);

    f32 crossSign = (frontDir.y * diff.z - frontDir.z * diff.y) * upDir.x +
                    (frontDir.z * diff.x - frontDir.x * diff.z) * upDir.y +
                    (frontDir.x * diff.y - frontDir.y * diff.x) * upDir.z;

    f32 angle = al::calcAngleDegree(frontDir, diff);

    const al::Nerve* leftNerve;
    const al::Nerve* rightNerve;
    if (angle < 45.0f) {
        leftNerve = &TurnL30;
        rightNerve = &TurnR30;
    } else if (angle < 120.0f) {
        leftNerve = &TurnL90;
        rightNerve = &TurnR90;
    } else {
        leftNerve = &TurnL180;
        rightNerve = &TurnR180;
    }

    al::setNerve(this, crossSign > 0.0f ? leftNerve : rightNerve);
    return true;
}

// NON_MATCHING: target merges exeTurnL30 thunk with the shared exeTurn body into one
// 204-byte function; our compiler emits them as separate functions
void ShibakenStateTurn::exeTurnL30() {
    exeTurn("TurnL30", _20, _30);
}

void ShibakenStateTurn::exeTurn(const char* actionName, const sead::Quatf& from,
                                const sead::Quatf& to) {
    if (al::isFirstStep(this))
        al::startAction(mActor, actionName);

    sead::Quatf* quatPtr = al::getQuatPtr(mActor);
    f32 frameMax = al::getActionFrameMax(mActor, al::getActionName(mActor));
    f32 rate = al::calcNerveRate(this, (s32)frameMax);
    al::slerpQuat(quatPtr, from, to, rate);

    if (al::isActionEnd(mActor))
        kill();
}

void ShibakenStateTurn::exeTurnL90() {
    exeTurn("TurnL90", _20, _30);
}

void ShibakenStateTurn::exeTurnL180() {
    exeTurn("TurnL180", _20, _30);
}

void ShibakenStateTurn::exeTurnR30() {
    exeTurn("TurnR30", _20, _30);
}

void ShibakenStateTurn::exeTurnR90() {
    exeTurn("TurnR90", _20, _30);
}

void ShibakenStateTurn::exeTurnR180() {
    exeTurn("TurnR180", _20, _30);
}
