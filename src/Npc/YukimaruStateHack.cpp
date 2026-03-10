#include "Npc/YukimaruStateHack.h"

#include <math/seadVector.h>

#include "Library/Effect/EffectSystemInfo.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Se/SeFunction.h"
#include "Library/Shadow/ActorShadowUtil.h"

#include "Npc/Yukimaru.h"
#include "Npc/YukimaruStateMove.h"
#include "Player/PlayerHackStartShaderCtrl.h"
#include "Util/DemoUtil.h"
#include "Util/Hack.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(YukimaruStateHack, Move);
NERVE_IMPL(YukimaruStateHack, WaitEnterMario);
NERVE_IMPL(YukimaruStateHack, Cancel);
NERVE_IMPL(YukimaruStateHack, Damage);
NERVE_IMPL(YukimaruStateHack, Kill);
NERVE_IMPL(YukimaruStateHack, TalkDemo);
NERVE_IMPL(YukimaruStateHack, HackStart);

NERVES_MAKE_NOSTRUCT(YukimaruStateHack, WaitEnterMario);
NERVES_MAKE_NOSTRUCT(YukimaruStateHack, HackStart);
NERVES_MAKE_NOSTRUCT(YukimaruStateHack, Move, Cancel, Damage, Kill, TalkDemo);
}  // namespace

YukimaruStateHack::YukimaruStateHack(Yukimaru* yukimaru) : ActorStateBase("憑依", yukimaru) {
    initNerve(&Move, 1);
    mStateMove =
        new YukimaruStateMove(yukimaru, static_cast<YukimaruInput*>(this), &yukimaru->mQuat);
    mStateMove->mIsHack = true;
    al::initNerveState(this, mStateMove, &Move, "移動状態");
    mShaderCtrl = new PlayerHackStartShaderCtrl(yukimaru, nullptr);
}

void YukimaruStateHack::start(al::HitSensor* other, al::HitSensor* self, bool isRunning) {
    mHackActor = rs::startHack(self, other, nullptr);
    rs::startHackStartDemo(mHackActor, mActor);
    al::setNerve(this, &WaitEnterMario);
    mIsRunning = isRunning;
}

bool YukimaruStateHack::tryStartDirect(const al::SensorMsg* msg) {
    if (!rs::isMsgHackDirectStageInit(&mHackActor, msg))
        return false;
    onHackShadowAndSilhouette();
    al::setNerve(this, &Move);
    return true;
}

void YukimaruStateHack::onHackShadowAndSilhouette() {
    al::showSilhouetteModelIfHide(mActor);
    al::invalidateOcclusionQuery(mActor);
    al::offDepthShadowModel(mActor);
    al::validateDepthShadowMap(mActor);
}

void YukimaruStateHack::appear() {
    NerveStateBase::appear();
    al::offGroupClipping(mActor);
    al::invalidateClipping(mActor);
    al::setSeKeeperPlayNamePrefix(mActor, "PHack");
    mStateMove->mIsHack = true;
}

void YukimaruStateHack::kill() {
    NerveStateBase::kill();
    al::onGroupClipping(mActor);
    al::showModelIfHide(mActor);
    al::hideSilhouetteModelIfShow(mActor);
    al::validateOcclusionQuery(mActor);
    al::onDepthShadowModel(mActor);
    al::invalidateDepthShadowMap(mActor);
    mStateMove->resetEffect();
    mStateMove->mIsHack = false;
    al::resetSeKeeperPlayNamePrefix(mActor);
    if (mHackActor) {
        if (rs::isActiveHackStartDemo(mHackActor))
            rs::endHackStartDemo(mHackActor, mActor);
        const sead::Vector3f& trans = al::getTrans(mActor);
        sead::Vector3f targetPos = trans + sead::Vector3f::ey * 240.0f;
        rs::endHackFromTargetPos(&mHackActor, targetPos, al::getQuat(mActor), sead::Vector3f::ey);
    }
}

void YukimaruStateHack::offHackShadowAndSilhouette() {
    al::hideSilhouetteModelIfShow(mActor);
    al::validateOcclusionQuery(mActor);
    al::onDepthShadowModel(mActor);
    al::invalidateDepthShadowMap(mActor);
}

void YukimaruStateHack::control() {
    mShaderCtrl->update();
}

void YukimaruStateHack::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (mHackActor)
        rs::sendMsgHackerNoReaction(mHackActor, other, self);
    if (al::isNerve(this, &Move))
        mStateMove->attackSensor(self, other);
}

bool YukimaruStateHack::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                   al::HitSensor* self) {
    if (rs::isMsgHackSyncDamageVisibility(msg)) {
        rs::syncDamageVisibility(mActor, mHackActor);
        return true;
    }
    if (rs::isMsgEnableMapCheckPointWarpCollidedGround(msg, mActor))
        return true;
    if (rs::isMsgCancelHack(msg)) {
        al::setNerve(this, &Cancel);
        kill();
        return true;
    }
    if (rs::isMsgHackMarioDead(msg)) {
        al::setNerve(this, &Damage);
        kill();
        return true;
    }
    if (rs::isMsgHackerDamageAndCancel(msg) && !al::isSensorEnemyAttack(self))
        return rs::requestDamage(mHackActor);
    if (rs::isMsgHackMarioCheckpointFlagWarp(msg) && !al::isSensorEnemyAttack(self)) {
        al::startAction(mActor, "HackEnd");
        al::setNerve(this, &Cancel);
        kill();
        return true;
    }
    if (rs::isMsgHackMarioDemo(msg)) {
        al::tryKillEmitterAndParticleAll(mActor);
        al::setNerve(this, &Kill);
        kill();
        return true;
    }
    if (rs::isMsgHackMarioInWater(msg) && !al::isSensorEnemyAttack(self)) {
        al::setNerve(this, &Damage);
        kill();
        return true;
    }
    if (rs::isMsgHackDemoStart(msg)) {
        rs::addDemoActor(mActor, false);
        al::setNerve(this, &TalkDemo);
        return true;
    }
    if (rs::isMsgHackDemoEnd(msg)) {
        mStateMove->startRun(false);
        al::setNerve(this, &Move);
        return true;
    }
    if (al::isNerve(this, &Move))
        return mStateMove->receiveMsg(msg, other, self);
    return false;
}

void YukimaruStateHack::exeWaitEnterMario() {
    if (rs::isHackStartDemoEnterMario(mHackActor))
        al::setNerve(this, &HackStart);
}

void YukimaruStateHack::exeHackStart() {
    if (al::isFirstStep(this)) {
        al::startAction(mActor, mIsRunning ? "HackStartRun" : "HackStart");
        onHackShadowAndSilhouette();
        mShaderCtrl->start();
    }
    if (al::isActionEnd(mActor)) {
        rs::endHackStartDemo(mHackActor, mActor);
        mStateMove->startRun(mIsRunning);
        al::setNerve(this, &Move);
    }
}

void YukimaruStateHack::exeMove() {
    al::updateNerveState(this);
}

void YukimaruStateHack::exeCancel() {}

void YukimaruStateHack::exeDamage() {}

void YukimaruStateHack::exeKill() {}

void YukimaruStateHack::exeTalkDemo() {
    if (al::isFirstStep(this))
        al::setVelocityZero(mActor);
}

bool YukimaruStateHack::isEndCancel() const {
    return al::isNerve(this, &Cancel);
}

bool YukimaruStateHack::isEndDamage() const {
    return al::isNerve(this, &Damage);
}

bool YukimaruStateHack::isEndKill() const {
    return al::isNerve(this, &Kill);
}

void YukimaruStateHack::updateMoveNoInput() {
    if (mStateMove)
        mStateMove->updateMoveNoInput();
}

void YukimaruStateHack::updateScale() {
    if (mStateMove)
        mStateMove->updateScale();
}

void YukimaruStateHack::resetMoveEffect() {
    if (mStateMove)
        mStateMove->resetEffect();
}

bool YukimaruStateHack::isTriggerJump() const {
    return rs::isTriggerHackJump(mHackActor) || rs::isTriggerHackSwing(mHackActor);
}

bool YukimaruStateHack::isHoldJump() const {
    return rs::isTriggerHackAnyButton(mHackActor);
}

void YukimaruStateHack::calcInputVec(sead::Vector3f* out) const {
    rs::calcHackerMoveVec(out, mHackActor, sead::Vector3f::ey);
    *out *= 0.5f;
}
