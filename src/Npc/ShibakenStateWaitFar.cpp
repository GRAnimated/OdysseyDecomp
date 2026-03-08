#include "Npc/ShibakenStateWaitFar.h"

#include <math/seadVector.h>

#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/Shibaken.h"
#include "Npc/ShibakenFunction.h"
#include "Npc/ShibakenStateSit.h"
#include "Npc/ShibakenStateTurn.h"
#include "Util/PlayerUtil.h"

namespace {
NERVE_IMPL(ShibakenStateWaitFar, Turn);
NERVE_IMPL(ShibakenStateWaitFar, Sit);
NERVES_MAKE_NOSTRUCT(ShibakenStateWaitFar, Turn, Sit);
}  // namespace

ShibakenStateWaitFar::ShibakenStateWaitFar(const char* name, Shibaken* shibaken)
    : al::NerveStateBase(name), mShibaken(shibaken) {
    initNerve(&Turn, 2);

    auto* stateTurn = new ShibakenStateTurn("ターン", shibaken);
    mStateTurn = stateTurn;

    auto* stateSit = new ShibakenStateSit("座り", shibaken, nullptr, false);
    mStateSit = stateSit;

    al::initNerveState(this, mStateTurn, &Turn, "ターン");
    al::initNerveState(this, mStateSit, &Sit, "座り");
}

// NON_MATCHING: compiler optimizes Vector3f copy to 64-bit LDR+STR for x,y; ours does 3 separate 32-bit copies
bool ShibakenStateWaitFar::tryStart() {
    if (ShibakenFunction::checkStopChaseByFaceWall(mShibaken)) {
        mWallNormal = al::getCollidedWallNormal(mShibaken);
        return true;
    }

    if (rs::isNearPlayerH(mShibaken, 600.0f))
        return false;

    mWallNormal = {0.0f, 0.0f, 0.0f};
    return true;
}

void ShibakenStateWaitFar::appear() {
    ShibakenStateTurn* stateTurn = mStateTurn;
    mIsDead = false;
    const sead::Vector3f& playerPos = rs::getPlayerPos(mShibaken);
    if (stateTurn->tryStartTurn(playerPos))
        al::setNerve(this, &Turn);
    else
        al::setNerve(this, &Sit);
}

void ShibakenStateWaitFar::exeTurn() {
    al::updateNerveStateAndNextNerve(this, &Sit);
}

void ShibakenStateWaitFar::exeSit() {
    if (al::isGreaterEqualStep(this, 30)) {
        if (rs::isNearPlayerH(mShibaken, 525.0f)) {
            mStateSit->tryStartSitEnd();
        } else {
            sead::Vector3f upDir = {0.0f, 0.0f, 0.0f};
            sead::Vector3f frontDir = {0.0f, 0.0f, 0.0f};

            const sead::Vector3f& playerPos = rs::getPlayerPos(mShibaken);
            const sead::Vector3f& trans = al::getTrans(mShibaken);
            sead::Vector3f diff = playerPos - trans;

            al::calcUpDir(&upDir, mShibaken);
            al::calcFrontDir(&frontDir, mShibaken);
            al::verticalizeVec(&diff, upDir, diff);

            if (al::tryNormalizeOrZero(&diff)) {
                if (al::calcAngleDegree(frontDir, diff) > 80.0f)
                    mStateSit->tryStartSitEnd();
            }
        }
    }

    if (al::updateNerveState(this)) {
        if (rs::isNearPlayerH(mShibaken, 525.0f) ||
            (al::isNormalize(mWallNormal, 0.001f) &&
             !ShibakenFunction::checkStopChaseByFaceWall(mShibaken, mWallNormal))) {
            kill();
        } else {
            ShibakenStateTurn* stateTurn = mStateTurn;
            const sead::Vector3f& playerPos = rs::getPlayerPos(mShibaken);
            if (stateTurn->tryStartTurn(playerPos))
                al::setNerve(this, &Turn);
            else
                al::setNerve(this, &Sit);
        }
    }
}
