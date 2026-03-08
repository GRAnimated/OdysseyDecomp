#include "Npc/Tiara.h"

#include <cstring>
#include <math/seadQuat.h>

#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"

#include "Npc/NpcStateReaction.h"
#include "Util/NpcAnimUtil.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(Tiara, Wait);
NERVE_IMPL(Tiara, Reaction);
NERVE_IMPL(Tiara, Scare);
NERVE_IMPL(Tiara, Talk);
NERVE_END_IMPL(Tiara, Stop);
NERVE_IMPL(Tiara, TurnToHost);
NERVE_IMPL(Tiara, StopAction);
NERVE_IMPL(Tiara, FindTurn);
NERVE_IMPL(Tiara, WaitInitPeachWorldHomeCastleCapDemo);
NERVE_IMPL(Tiara, ShakeHandPeachWorldHomeCastleCapDemo);
NERVE_IMPL(Tiara, StopToHost);
NERVE_IMPL(Tiara, ShakeHand);
NERVE_IMPL(Tiara, WaitAfterPeachWorldHomeCastleCapDemo);

NERVES_MAKE_NOSTRUCT(Tiara, Talk, Stop, TurnToHost, StopAction, FindTurn,
                     WaitInitPeachWorldHomeCastleCapDemo, ShakeHandPeachWorldHomeCastleCapDemo,
                     StopToHost, ShakeHand, WaitAfterPeachWorldHomeCastleCapDemo);
NERVES_MAKE_STRUCT(Tiara, Wait, Reaction, Scare);
}  // namespace

Tiara::Tiara(const char* name) : al::LiveActor(name) {}

void Tiara::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, "CapManHeroine", nullptr);
    al::initNerve(this, &NrvTiara.Wait, 1);
    mNpcStateReaction = NpcStateReaction::create(this, nullptr);
    al::initNerveState(this, mNpcStateReaction, &NrvTiara.Reaction, u8"リアクション");
    al::tryGetStringArg(&mWaitActionName, info, "TiaraWaitActionName");
    al::initJointControllerKeeper(this, 1);
    rs::initCapWorldNpcTail(this);
    makeActorAlive();
}

void Tiara::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    rs::attackSensorNpcCommon(self, other);
}

bool Tiara::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) {
    if (rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;
    bool result;
    if (mIsInvalidTrample)
        result = mNpcStateReaction->receiveMsgWithoutTrample(msg, other, self);
    else
        result = mNpcStateReaction->receiveMsg(msg, other, self);
    if (result) {
        if (!al::isNerve(this, &NrvTiara.Reaction))
            al::setNerve(this, &NrvTiara.Reaction);
        return true;
    }
    return mNpcStateReaction->receiveMsgNoReaction(msg, other, self);
}

void Tiara::tryStartScare() {
    if (!al::isNerve(this, &NrvTiara.Scare) && !al::isNerve(this, &NrvTiara.Reaction))
        al::setNerve(this, &NrvTiara.Scare);
}

void Tiara::tryEndScare() {
    if (al::isNerve(this, &NrvTiara.Scare))
        al::setNerve(this, &NrvTiara.Wait);
}

void Tiara::startWait() {
    if (!al::isNerve(this, &NrvTiara.Wait))
        al::setNerve(this, &NrvTiara.Wait);
}

void Tiara::startTalk() {
    al::setNerve(this, &Talk);
}

void Tiara::startStop() {
    al::setNerve(this, &Stop);
}

void Tiara::startTurnToHostAndStop(s32 frames) {
    mTurnStep = frames;
    al::calcFrontDir(&mTurnDir, this);
    al::setNerve(this, &TurnToHost);
}

void Tiara::startStopAction(const char* actionName) {
    al::startAction(this, actionName);
    al::setNerve(this, &StopAction);
}

void Tiara::startFindTurn() {
    mIsFindTurn = true;
    al::setNerve(this, &FindTurn);
}

bool Tiara::isEndTurn() const {
    return !al::isNerve(this, &TurnToHost);
}

void Tiara::startPeachWorldHomeCastleCapDemo() {
    al::setNerve(this, &WaitInitPeachWorldHomeCastleCapDemo);
}

void Tiara::startShakeHandPeachWorldHomeCastleCapDemo() {
    al::setNerve(this, &ShakeHandPeachWorldHomeCastleCapDemo);
}

void Tiara::endPeachWorldHomeCastleCapDemo() {
    if (!al::isNerve(this, &NrvTiara.Wait))
        al::setNerve(this, &NrvTiara.Wait);
}

void Tiara::exeWait() {
    if (al::isFirstStep(this)) {
        al::startAction(this, mWaitActionName);
        al::calcFrontDir(&mFrontDir, this);
    }
    if (mIsFindTurn) {
        const sead::Vector3f& playerPos = rs::getPlayerPos(this);
        const sead::Vector3f& trans = al::getTrans(this);
        sead::Vector3f dir = playerPos - trans;
        al::verticalizeVec(&dir, al::getGravity(this), dir);
        if (!al::tryNormalizeOrZero(&dir))
            dir.set(mFrontDir);
        al::turnVecToVecRate(&dir, mFrontDir, dir, 0.2f);
        if (al::calcAngleDegree(dir, mFrontDir) > 2.0f)
            al::turnVecToVecDegree(&dir, mFrontDir, dir, 2.0f);
        mFrontDir.set(dir);
        sead::Vector3f frontDir = {0.0f, 0.0f, 0.0f};
        al::calcFrontDir(&frontDir, this);
        al::turnVecToVecRate(&frontDir, frontDir, mFrontDir, 0.1f);
        al::normalize(&frontDir);
        sead::Quatf quat = sead::Quatf::unit;
        const sead::Vector3f& gravity = al::getGravity(this);
        sead::Vector3f up = {-gravity.x, -gravity.y, -gravity.z};
        al::makeQuatFrontUp(&quat, frontDir, up);
        al::updatePoseQuat(this, quat);
    }
}

void Tiara::exeTalk() {
    if (al::isFirstStep(this))
        al::startAction(this, "Talk");
}

void Tiara::exeStop() {
    if (al::isFirstStep(this))
        al::stopAction(this);
}

void Tiara::endStop() {
    al::restartAction(this);
}

void Tiara::exeReaction() {
    al::updateNerveStateAndNextNerve(this, &NrvTiara.Wait);
}

void Tiara::exeTurnToHost() {
    if (al::isFirstStep(this))
        al::startAction(this, "Turn");
    const sead::Vector3f& hostTrans = al::getTrans(mHostActor);
    const sead::Vector3f& trans = al::getTrans(this);
    sead::Vector3f dir = hostTrans - trans;
    al::verticalizeVec(&dir, al::getGravity(this), dir);
    if (!al::tryNormalizeOrZero(&dir)) {
        al::setNerve(this, &StopToHost);
        return;
    }
    const sead::Vector3f& turnDir = mTurnDir;
    f32 rate = al::calcNerveRate(this, mTurnStep);
    al::turnVecToVecRate(&dir, turnDir, dir, rate);
    al::normalize(&dir);
    sead::Quatf quat = sead::Quatf::unit;
    const sead::Vector3f& gravity = al::getGravity(this);
    sead::Vector3f up = {-gravity.x, -gravity.y, -gravity.z};
    al::makeQuatFrontUp(&quat, dir, up);
    al::updatePoseQuat(this, quat);
    al::setNerveAtGreaterEqualStep(this, &StopToHost, mTurnStep);
}

void Tiara::exeStopToHost() {
    if (al::isFirstStep(this))
        al::startAction(this, mWaitActionName);
}

void Tiara::exeStopAction() {
    const char* actionName = al::getActionName(this);
    if (al::isActionOneTime(this, actionName) && al::isActionEnd(this))
        al::setNerve(this, &StopToHost);
}

void Tiara::exeFindTurn() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "FindTurn");
        al::calcFrontDir(&mTurnDir, this);
    }
    const sead::Vector3f& playerPos = rs::getPlayerPos(this);
    const sead::Vector3f& trans = al::getTrans(this);
    sead::Vector3f dir = playerPos - trans;
    al::verticalizeVec(&dir, al::getGravity(this), dir);
    if (!al::tryNormalizeOrZero(&dir))
        al::calcFrontDir(&dir, this);
    const sead::Vector3f& turnDir = mTurnDir;
    f32 frame = al::getActionFrame(this);
    const char* actionName = al::getActionName(this);
    f32 frameMax = al::getActionFrameMax(this, actionName);
    f32 rate = al::normalize(frame, 0.0f, frameMax);
    al::turnVecToVecRate(&dir, turnDir, dir, rate);
    al::normalize(&dir);
    sead::Quatf quat = sead::Quatf::unit;
    const sead::Vector3f& gravity = al::getGravity(this);
    sead::Vector3f up = {-gravity.x, -gravity.y, -gravity.z};
    al::makeQuatFrontUp(&quat, dir, up);
    al::updatePoseQuat(this, quat);
    if (al::isActionEnd(this))
        al::setNerve(this, &ShakeHand);
}

void Tiara::exeShakeHand() {
    if (al::isFirstStep(this))
        al::startAction(this, "ShakeHand");
    al::setNerveAtActionEnd(this, &NrvTiara.Wait);
}

void Tiara::exeScare() {
    if (al::isFirstStep(this))
        al::startAction(this, "Scared");
}

void Tiara::exeWaitInitPeachWorldHomeCastleCapDemo() {
    if (al::isFirstStep(this))
        al::startAction(this, mWaitActionName);
}

void Tiara::exeShakeHandPeachWorldHomeCastleCapDemo() {
    if (al::isFirstStep(this))
        al::startAction(this, "ShakeHand");
    al::setNerveAtActionEnd(this, &WaitAfterPeachWorldHomeCastleCapDemo);
}

void Tiara::exeWaitAfterPeachWorldHomeCastleCapDemo() {
    if (al::isFirstStep(this))
        al::startAction(this, mWaitActionName);
}
