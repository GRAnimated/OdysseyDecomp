#include "Npc/ShibakenStateWait.h"

#include <math/seadMathCalcCommon.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/Shibaken.h"
#include "Npc/ShibakenFunction.h"

namespace {
NERVE_IMPL(ShibakenStateWait, Wait);
NERVE_IMPL(ShibakenStateWait, Bow);
NERVE_IMPL(ShibakenStateWait, Shake);
NERVE_IMPL(ShibakenStateWait, Jump);
NERVES_MAKE_NOSTRUCT(ShibakenStateWait, Wait, Bow, Shake, Jump);
}  // namespace

ShibakenStateWait::ShibakenStateWait(const char* name, Shibaken* shibaken, bool isAlwaysWait)
    : al::NerveStateBase(name), mShibaken(shibaken), mIsAlwaysWait(isAlwaysWait) {
    initNerve(&Wait, 0);
}

void ShibakenStateWait::appear() {
    al::NerveStateBase::appear();
    al::setNerve(this, &Wait);
}

bool ShibakenStateWait::isPlayingWait() const {
    return al::isNerve(this, &Wait);
}

// NON_MATCHING: instruction scheduling and tail-call optimization for setNerve in array branch
void ShibakenStateWait::exeWait() {
    if (al::isFirstStep(this)) {
        al::tryStartActionIfNotPlaying(mShibaken, "Wait");
        mWaitFrames = al::getRandom(120, 240);
    }

    ShibakenFunction::addFallVelocityToGround(mShibaken, -1.0f);

    if (mIsAlwaysWait)
        return;

    if (al::isGreaterEqualStep(this, mWaitFrames)) {
        const al::Nerve* nerves[3];
        nerves[0] = &Bow;
        nerves[1] = &Shake;
        nerves[2] = &Jump;
        s32 idx = al::getRandom(3);
        al::setNerve(this, nerves[sead::Mathi::clamp(idx, 0, 2)]);
    }
}

void ShibakenStateWait::exeBow() {
    if (al::isFirstStep(this))
        al::startAction(mShibaken, "WaitBow");

    ShibakenFunction::addFallVelocityToGround(mShibaken, -1.0f);

    if (al::isActionEnd(mShibaken))
        al::setNerve(this, &Wait);
}

void ShibakenStateWait::exeShake() {
    if (al::isFirstStep(this))
        al::startAction(mShibaken, "WaitShake");

    ShibakenFunction::addFallVelocityToGround(mShibaken, -1.0f);

    if (al::isActionEnd(mShibaken))
        al::setNerve(this, &Wait);
}

void ShibakenStateWait::exeJump() {
    if (al::isFirstStep(this))
        al::startAction(mShibaken, "WaitJump");

    ShibakenFunction::addFallVelocityToGround(mShibaken, -1.0f);

    if (al::isActionEnd(mShibaken))
        al::setNerve(this, &Wait);
}
