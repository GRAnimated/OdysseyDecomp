#include "Npc/NpcStateWait.h"

#include <math/seadMathCalcCommon.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Movement/RumbleCalculator.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Player/PlayerUtil.h"
#include "Library/Stage/StageSwitchUtil.h"
#include "Library/Thread/FunctorV0M.h"

#include "Npc/NpcStateRumbleParam.h"
#include "Npc/NpcStateTurnParam.h"
#include "Npc/NpcStateWaitParam.h"

namespace {
NERVE_IMPL(NpcStateWait, Wait);
NERVE_IMPL(NpcStateWait, WaitAfter);
NERVE_IMPL(NpcStateWait, Turn);
NERVE_IMPL(NpcStateWait, TurnEnd);

NERVES_MAKE_STRUCT(NpcStateWait, Wait, WaitAfter, Turn, TurnEnd);
}  // namespace

static const NpcStateWaitParam sDefaultWaitParam("Wait", nullptr, nullptr, nullptr, nullptr,
                                                 nullptr, false, nullptr, false);

// NON_MATCHING: regalloc in FunctorV0M setup and IUseStageSwitch cast
NpcStateWait::NpcStateWait(al::LiveActor* actor, const al::ActorInitInfo& initInfo,
                           const NpcStateWaitParam* waitParam, const NpcStateTurnParam* turnParam,
                           const NpcStateRumbleParam* rumbleParam)
    : ActorStateBase("NPC", actor) {
    mIsWaitAfter = false;
    mIsTurnInvalid = false;
    mWaitParam = waitParam;
    mTurnParam = turnParam;
    mRumbleParam = rumbleParam;
    mRumbleCalculator = nullptr;
    mRumbleTimer = -1;
    initNerve(&NrvNpcStateWait.Wait, 1);

    if (!waitParam)
        mWaitParam = &sDefaultWaitParam;

    if (!mWaitParam->mTrampledAction) {
        if (mRumbleParam) {
            auto* calc = new al::RumbleCalculatorCosMultLinear(
                mRumbleParam->mFrequency, mRumbleParam->mPhaseOffset, mRumbleParam->mAmplitude,
                mRumbleParam->mDuration);
            mRumbleCalculator = calc;
        }
    }

    al::FunctorV0M<NpcStateWait*, void (NpcStateWait::*)()> functorOn(this,
                                                                      &NpcStateWait::setWaitAfter);
    al::FunctorV0M<NpcStateWait*, void (NpcStateWait::*)()> functorOff(this,
                                                                       &NpcStateWait::setWait);
    al::listenStageSwitchOnOffStart(static_cast<al::IUseStageSwitch*>(mActor), functorOn,
                                    functorOff);
}

void NpcStateWait::setWaitAfter() {
    mIsWaitAfter = true;
    if (al::isNerve(this, &NrvNpcStateWait.Wait) ||
        al::isNerve(this, &NrvNpcStateWait.WaitAfter))
        al::setNerve(this, &NrvNpcStateWait.WaitAfter);
}

void NpcStateWait::setWait() {
    mIsWaitAfter = false;
    if (al::isNerve(this, &NrvNpcStateWait.Wait) ||
        al::isNerve(this, &NrvNpcStateWait.WaitAfter))
        al::setNerve(this, &NrvNpcStateWait.Wait);
}

void NpcStateWait::appear() {
    mIsDead = false;
    al::setNerve(this, !mIsWaitAfter ? &NrvNpcStateWait.Wait
                                     : (const al::Nerve*)&NrvNpcStateWait.WaitAfter);
}

void NpcStateWait::startWait() {
    al::setNerve(this, !mIsWaitAfter ? &NrvNpcStateWait.Wait
                                     : (const al::Nerve*)&NrvNpcStateWait.WaitAfter);
}

// NON_MATCHING: instruction scheduling for Vector3f add (ldp vs individual ldr)
void NpcStateWait::control() {
    if (mRumbleTimer < 0)
        return;

    if (mRumbleTimer == 0)
        mRumbleCalculator->start(0);

    if (mRumbleParam->mDuration > mRumbleTimer) {
        mRumbleCalculator->calc();
        f32 s = mRumbleParam->mScale;
        sead::Vector3f scale;
        scale.x = s + mRumbleCalculator->mResult.x;
        scale.y = s + mRumbleCalculator->mResult.y;
        scale.z = s + mRumbleCalculator->mResult.z;
        al::setScale(mActor, scale);
        mRumbleTimer++;
    } else {
        al::setScaleAll(mActor, mRumbleParam->mScale);
        mRumbleCalculator->reset();
        mRumbleTimer = -1;
    }
}

void NpcStateWait::invalidateTurn() {
    mIsTurnInvalid = true;
    if (al::isNerve(this, &NrvNpcStateWait.Turn))
        al::setNerve(this, !mIsWaitAfter ? &NrvNpcStateWait.Wait
                                         : (const al::Nerve*)&NrvNpcStateWait.WaitAfter);
}

void NpcStateWait::exeWait() {
    if (al::isFirstStep(this)) {
        if (mWaitParam->mWaitAction)
            al::tryStartActionIfNotPlaying(mActor, mWaitParam->mWaitAction);
    }
    tryStartTurn(mTurnParam);
}

// NON_MATCHING: fabs vs fneg+fcmp+fcsel for Mathf::abs; regalloc (d8 vs x23)
bool NpcStateWait::tryStartTurn(const NpcStateTurnParam* param) {
    if (!param || mIsTurnInvalid)
        return false;

    if (!param->mIsEnable || (al::isNerve(this, &NrvNpcStateWait.Wait) &&
                              param->mIsOnlyFromWaitAfter))
        return false;

    al::LiveActor* player = al::getPlayerActor(mActor, 0);
    if (!al::isNear(player, mActor, param->mNearDistance))
        return false;

    f32 angle = al::calcAngleToTargetH(mActor, al::getTrans(player));
    if (param->mAngleThreshold >= sead::Mathf::abs(angle))
        return false;

    al::setNerve(this, &NrvNpcStateWait.Turn);
    return true;
}

void NpcStateWait::exeWaitAfter() {
    if (al::isFirstStep(this))
        al::tryStartActionIfNotPlaying(mActor, mWaitParam->mWaitAfterAction);
    tryStartTurn(mTurnParam);
}

// NON_MATCHING: fabs vs fneg+fcmp+fcsel for Mathf::abs; cmp #0 vs #1 for > 0 check
void NpcStateWait::exeTurn() {
    if (al::isFirstStep(this)) {
        if (mWaitParam->mTurnAction)
            al::tryStartActionIfNotPlaying(mActor, mWaitParam->mTurnAction);
    }

    if (mRumbleCalculator && mRumbleCalculator->mFrame < mRumbleCalculator->mDuration)
        return;

    al::LiveActor* player = al::getPlayerActor(mActor, 0);
    if (!al::isNear(player, mActor, mTurnParam->mNearDistance))
        return;

    if (!al::turnToTarget(mActor, player, mTurnParam->mTurnSpeed)) {
        f32 angle = al::calcAngleToTargetH(mActor, al::getTrans(player));
        if (sead::Mathf::abs(angle) >= mTurnParam->mAngleEnd)
            return;
    }

    startTurnEnd();
}

// NON_MATCHING: cmp #0;b.le vs cmp #1;b.lt for > 0 check
void NpcStateWait::startTurnEnd() {
    if (mTurnParam->mTurnEndStep > 0)
        al::setNerve(this, &NrvNpcStateWait.TurnEnd);
    else
        al::setNerve(this, !mIsWaitAfter ? &NrvNpcStateWait.Wait
                                         : (const al::Nerve*)&NrvNpcStateWait.WaitAfter);
}

void NpcStateWait::exeTurnEnd() {
    if (tryStartTurn(mTurnParam))
        return;

    if (al::isGreaterEqualStep(this, mTurnParam->mTurnEndStep))
        al::setNerve(this, !mIsWaitAfter ? &NrvNpcStateWait.Wait
                                         : (const al::Nerve*)&NrvNpcStateWait.WaitAfter);
}

void NpcStateWait::exeTrampled() {
    if (al::isFirstStep(this))
        al::startAction(mActor, mWaitParam->mTrampledAction);

    if (al::isActionEnd(mActor))
        al::setNerve(this, !mIsWaitAfter ? &NrvNpcStateWait.Wait
                                         : (const al::Nerve*)&NrvNpcStateWait.WaitAfter);
}

// NON_MATCHING: pre-increment addressing, 8-byte vector copy vs member-by-member
NpcStateWaitParam::NpcStateWaitParam(const char* waitAction, const char* waitAfterAction,
                                     const char* turnAction, const char* a18, const char* a20,
                                     const char* trampledAction, bool a30,
                                     const sead::Vector3f* vec, bool a40) {
    mWaitAction = waitAction;
    mWaitAfterAction = waitAfterAction;
    mTurnAction = turnAction;
    _18 = a18;
    _20 = a20;
    mTrampledAction = trampledAction;
    _30 = a30;
    _34.x = 0;
    _34.y = 0;
    _34.z = 0;
    _40 = a40;
    if (vec)
        _34 = *vec;
}

NpcStateTurnParam::NpcStateTurnParam(f32 angleThreshold, f32 angleEnd, f32 turnSpeed,
                                     f32 nearDistance, bool isEnable, bool isOnlyFromWaitAfter,
                                     s32 turnEndStep) {
    mIsEnable = isEnable;
    mAngleThreshold = angleThreshold;
    mAngleEnd = angleEnd;
    mTurnSpeed = turnSpeed;
    mNearDistance = nearDistance;
    mIsOnlyFromWaitAfter = isOnlyFromWaitAfter;
    mTurnEndStep = turnEndStep;
}

NpcStateRumbleParam::NpcStateRumbleParam(s32 duration, f32 frequency, f32 phaseOffset,
                                         f32 amplitude, f32 scale) {
    mDuration = duration;
    mFrequency = frequency;
    mPhaseOffset = phaseOffset;
    mAmplitude = amplitude;
    mScale = scale;
}
