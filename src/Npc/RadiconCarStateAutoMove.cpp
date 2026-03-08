#include "Npc/RadiconCarStateAutoMove.h"

#include <math/seadVector.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Se/SeFunction.h"

#include "Npc/RadiconCar.h"

namespace {
NERVE_IMPL(RadiconCarStateAutoMove, Move);
NERVE_IMPL(RadiconCarStateAutoMove, Wait);
NERVES_MAKE_NOSTRUCT(RadiconCarStateAutoMove, Move, Wait);
}  // namespace

RadiconCarStateAutoMove::RadiconCarStateAutoMove(RadiconCar* car)
    : HostStateBase("ラジコンカー", car) {
    initNerve(&Move, 0);
}

void RadiconCarStateAutoMove::appear() {
    al::NerveStateBase::appear();
    al::setNerve(this, &Wait);
}

// NON_MATCHING: regswap (s3 vs s0)
void RadiconCarStateAutoMove::calcAccel(sead::Vector3f* accel) const {
    sead::Vector3f front;
    al::calcFrontDir(&front, getHost());
    s32 step = al::getNerveStep(this);
    f32 factor = sead::Mathf::sin((f32)step / 30.0f) * 0.3f;
    f32 ax = front.x * factor;
    f32 ay = factor * front.y;
    f32 az = factor * front.z;
    accel->x = ax;
    accel->y = ay;
    accel->z = az;
}

void RadiconCarStateAutoMove::exeWait() {
    if (al::isGreaterStep(this, 150)) {
        al::startAction(getHost(), "Move");
        al::setNerve(this, &Move);
    } else {
        getHost()->updateVelocity();
    }
}

// NON_MATCHING: SafeString construction scheduling, STP vs 2xSTR
void RadiconCarStateAutoMove::exeMove() {
    if (al::isVelocityFastH(getHost(), 3.0f)) {
        sead::SafeString actionName("Move");
        al::holdSeWithParam(getHost(), actionName, al::calcSpeedH(getHost()), "");
    }

    sead::Vector3f front;
    al::calcFrontDir(&front, getHost());
    s32 step = al::getNerveStep(this);
    f32 factor = sead::Mathf::sin((f32)step / 30.0f) * 0.3f;
    sead::Vector3f accel = {front.x * factor, front.y * factor, front.z * factor};
    al::addVelocity(getHost(), accel);
    getHost()->updateVelocity();
}
