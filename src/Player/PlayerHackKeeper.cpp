#include "Player/PlayerHackKeeper.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Se/SeFunction.h"
#include "Library/Effect/EffectSystemInfo.h"

#include "Player/HackCap.h"
#include "Player/CapTargetInfo.h"
#include "Player/PlayerCapFunction.h"
#include "Player/PlayerHackStartTexKeeper.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolder.h"
#include "System/GameDataHolderWriter.h"
#include "Util/DemoUtil.h"
#include "Util/SensorMsgFunction.h"

bool PlayerHackKeeper::startHack(al::HitSensor* hackSensor, al::HitSensor* parentSensor,
                                 al::LiveActor* hackActor) {
    if (!hackActor)
        hackActor = al::getSensorHost(hackSensor);
    if (!rs::sendMsgInitHack(parentSensor, hackSensor))
        return false;

    mHackActor = hackActor;
    mHackHitSensor = hackSensor;
    mParentBodySensor = parentSensor;
    mIsCancellingHack = false;
    mIsHackDemoStarted = false;
    mIsPuppetable2 = false;
    mIsStartedHacking = false;
    mHackObjectInfo = CapFunction::getHackObjInfo(mHackCap);
    return true;
}

void PlayerHackKeeper::setupHack(al::HitSensor* hackSensor, al::HitSensor* parentSensor,
                                 al::LiveActor* hackActor) {
    mParentBodySensor = parentSensor;
    mHackActor = hackActor;
    mHackHitSensor = hackSensor;
    mIsCancellingHack = false;
    mIsHackDemoStarted = false;
    mIsPuppetable2 = false;
    mIsStartedHacking = false;
    mHackObjectInfo = CapFunction::getHackObjInfo(mHackCap);
}

bool PlayerHackKeeper::endHack(const HackEndParam* param) {
    if (mIsHackDemoStarted) {
        rs::requestEndDemoHackStart(mHackActor);
        mIsHackDemoStarted = false;
        mIsPuppetable2 = false;
    }
    if (!rs::sendMsgEndHack(mParentBodySensor, mHackHitSensor, param))
        return false;
    mParentBodySensor = nullptr;
    mIsStartedHacking = false;
    mIsHackDemoStarted = false;
    mIsPuppetable2 = false;
    mIsPuppetable = false;
    mHackActor = nullptr;
    mHackHitSensor = nullptr;
    mHackObjectInfo = nullptr;
    return true;
}

void PlayerHackKeeper::endHackStartDemo(al::LiveActor*) {
    rs::requestEndDemoHackStart(mHackActor);
    mIsHackDemoStarted = false;
    mIsPuppetable2 = false;
}

void PlayerHackKeeper::startHackStartDemo(al::LiveActor*) {
    mIsHackDemoStarted = true;
    rs::requestStartDemoHackStart(mHackActor);
    mHackStartTexKeeper->setCaptureTextureCleared(true);
    mHackCap->addHackStartDemo();
    rs::addDemoActor(mHackModel, false);
}

void PlayerHackKeeper::startHackStartDemoPuppetable(al::LiveActor*) {
    mIsHackDemoStarted = true;
    rs::requestStartDemoHackStart(mHackActor);
    mHackStartTexKeeper->setCaptureTextureCleared(true);
    mHackCap->addHackStartDemo();
    rs::addDemoActor(mHackModel, false);
    mIsPuppetable = true;
    al::tryStopSe(mParent, "HackStart", -1, nullptr);
    mIsPuppetable2 = true;
}

void PlayerHackKeeper::addHackStartDemo(al::LiveActor* actor) {
    rs::addDemoActor(actor, false);
}

void PlayerHackKeeper::deleteHackDemoModelEffect() {
    al::tryDeleteEffect(mHackModel, "Trace");
}

void PlayerHackKeeper::killHackDemoModel() {
    al::tryDeleteEffect(mHackModel, "Trace");
    mHackModel->kill();
}

bool PlayerHackKeeper::isActiveHackStartDemo() const {
    return mIsHackDemoStarted && rs::isActiveDemoHackStart(mHackCap);
}

void PlayerHackKeeper::recordHack() {
    const char* hackName = mHackCap->getCapTargetInfo()->getHackName();
    if (hackName) {
        GameDataHolderWriter writer(mHackCap);
        GameDataFunction::addHackDictionary(writer, hackName);
    }
}

bool PlayerHackKeeper::cancelHackArea() {
    if (rs::sendMsgCancelHackArea(mHackHitSensor, mParentBodySensor))
        return true;
    mIsCancellingHack = true;
    if (!rs::sendMsgCancelHack(mHackHitSensor, mParentBodySensor)) {
        al::startHitReaction(mParent, "ひょうい解除失敗");
        mIsCancellingHack = false;
        return false;
    }
    mIsCancellingHack = false;
    return true;
}

u32 PlayerHackKeeper::cancelHack() {
    u32 result = 1;
    mIsCancellingHack = true;
    if (!rs::sendMsgCancelHack(mHackHitSensor, mParentBodySensor)) {
        al::startHitReaction(mParent, "ひょうい解除失敗");
        result = 0;
    }
    mIsCancellingHack = false;
    return result;
}

u32 PlayerHackKeeper::cancelForceRecovery() {
    u32 result = 1;
    mIsCancellingHack = true;
    if (!rs::sendMsgCancelHack(mHackHitSensor, mParentBodySensor)) {
        al::startHitReaction(mParent, "ひょうい解除失敗");
        result = 0;
    }
    mIsCancellingHack = false;
    return result;
}

bool PlayerHackKeeper::tryEscapeHack() {
    if (rs::sendMsgHackInvalidEscapeNoReaction(mHackHitSensor, mParentBodySensor))
        return false;
    if (rs::sendMsgHackInvalidEscape(mHackHitSensor, mParentBodySensor)) {
        al::startHitReaction(mParent, "ひょうい解除失敗");
        return false;
    }
    mIsCancellingHack = true;
    if (!rs::sendMsgCancelHack(mHackHitSensor, mParentBodySensor)) {
        al::startHitReaction(mParent, "ひょうい解除失敗");
        mIsCancellingHack = false;
        return false;
    }
    mIsCancellingHack = false;
    return true;
}

bool PlayerHackKeeper::sendTransferHack() {
    if (rs::sendMsgTransferHack(mHackHitSensor, mParentBodySensor))
        return true;
    mIsCancellingHack = true;
    if (!rs::sendMsgCancelHack(mHackHitSensor, mParentBodySensor)) {
        al::startHitReaction(mParent, "ひょうい解除失敗");
        mIsCancellingHack = false;
        return false;
    }
    mIsCancellingHack = false;
    return true;
}

u32 PlayerHackKeeper::sendMarioDemo() {
    if (rs::sendMsgHackMarioDemo(mHackHitSensor, mParentBodySensor))
        return 1;
    al::LiveActor* actor = mHackActor;
    cancelHack();
    actor->kill();
    return 0;
}

void PlayerHackKeeper::forceKillHack() {
    al::LiveActor* actor = mHackActor;
    cancelHack();
    actor->kill();
}

u32 PlayerHackKeeper::sendMarioDead() {
    if (rs::sendMsgHackMarioDead(mHackHitSensor, mParentBodySensor))
        return 1;
    al::LiveActor* actor = mHackActor;
    cancelHack();
    actor->kill();
    return 0;
}

bool PlayerHackKeeper::sendMarioInWater() {
    if (rs::sendMsgHackMarioInWater(mHackHitSensor, mParentBodySensor))
        return true;
    cancelHack();
    return false;
}

bool PlayerHackKeeper::sendMarioDeathArea() {
    if (rs::sendMsgHackDeathAreaSelfCheck(mHackHitSensor, mParentBodySensor))
        return false;
    if (rs::sendMsgHackMarioDead(mHackHitSensor, mParentBodySensor))
        return true;
    al::LiveActor* actor = mHackActor;
    cancelHack();
    actor->kill();
    return 1;
}

bool PlayerHackKeeper::sendMsgEnableMapCheckPointWarp() {
    return rs::sendMsgEnableMapCheckPointWarp(mHackHitSensor, mParentBodySensor);
}

bool PlayerHackKeeper::sendMsgSelfCeilingCheckMiss() {
    return rs::sendMsgHackSelfCeilingCheckMiss(mHackHitSensor, mParentBodySensor);
}

bool PlayerHackKeeper::receiveRequestTransferHack(al::HitSensor* sensor) {
    return mHackHitSensor && mHackCap->receiveRequestTransferHack(sensor, mParentBodySensor);
}

bool PlayerHackKeeper::requestDamage() {
    mIsTookDamage = true;
    return true;
}

bool PlayerHackKeeper::receiveRequestDamage() {
    if (!mIsTookDamage)
        return false;
    mIsTookDamage = false;
    return true;
}

bool PlayerHackKeeper::sendSyncDamageVisibility() {
    return rs::sendMsgHackSyncDamageVisibility(mHackHitSensor, mParentBodySensor);
}

bool PlayerHackKeeper::pushWorldEndBorder(const sead::Vector3f& offset) {
    al::setTrans(mHackActor, al::getTrans(mHackActor) + offset);
    return true;
}

const char* PlayerHackKeeper::getCurrentHackName() const {
    return mHackHitSensor ? mHackObjectInfo->hackName : nullptr;
}

IUsePlayerCollision* PlayerHackKeeper::getPlayerCollision() const {
    return mHackCap->getCapTargetInfo()->getPlayerCollision();
}

f32 PlayerHackKeeper::getHackGuideHeight() const {
    return mHackObjectInfo->guideHeight;
}

bool PlayerHackKeeper::isHackGuideEnable() const {
    return mHackObjectInfo->isGuideEnable;
}

f32 PlayerHackKeeper::getHackStayGravityMargine() const {
    return mHackObjectInfo->stayGravityMargine;
}

al::CollisionPartsFilterBase* PlayerHackKeeper::getCollisionPartsFilter() const {
    return mHackObjectInfo->isUseCollisionPartsFilterActor ? mCollisionFilter : nullptr;
}

bool PlayerHackKeeper::isHackGroupTalkScare() const {
    return mHackObjectInfo->isScare;
}

bool PlayerHackKeeper::isHackNoCollisionMsg() const {
    return mHackObjectInfo->isNoCollisionMsg;
}

bool PlayerHackKeeper::isHackNoSeparateCameraInput() const {
    return mHackObjectInfo->isNoSeparateCameraInput;
}

bool PlayerHackKeeper::isHackUsePlayerCollision() const {
    return mHackObjectInfo->isUsePlayerCollision;
}

bool PlayerHackKeeper::isHackCancelCeilingCheck() const {
    return mHackCap->getCapTargetInfo()->isHackCancelCeilingCheck();
}

bool PlayerHackKeeper::isHackInvalidLifeRecovery() const {
    return mHackCap->getCapTargetInfo()->isHackInvalidLifeRecovery();
}

void PlayerHackKeeper::requestForceHackStageStart(al::HitSensor* sensor,
                                                   const CapTargetInfo* target,
                                                   al::LiveActor* actor) {
    mStageStartActorSensor = sensor;
    mStageStartCapTargetInfo = const_cast<CapTargetInfo*>(target);
    mStageStartActor = actor ? actor : al::getSensorHost(sensor);
}

bool PlayerHackKeeper::executeForceHackStageStart(al::HitSensor* sensor, IUsePlayerHack* hack) {
    if (!mStageStartActorSensor)
        return false;
    mHackCap->forceHack(mStageStartActorSensor, mStageStartCapTargetInfo);
    mHackActor = mStageStartActor;
    mHackHitSensor = mStageStartActorSensor;
    mParentBodySensor = sensor;
    mIsCancellingHack = false;
    mIsHackDemoStarted = false;
    mIsPuppetable2 = false;
    mIsStartedHacking = false;
    mHackObjectInfo = CapFunction::getHackObjInfo(mHackCap);
    rs::sendMsgHackDirectStageInit(mStageStartActorSensor, sensor, hack);
    mStageStartActorSensor = nullptr;
    mStageStartActor = nullptr;
    mStageStartCapTargetInfo = nullptr;
    return true;
}

void PlayerHackKeeper::startDemo() {
    if (rs::sendMsgHackDemoStart(mHackHitSensor, mParentBodySensor))
        mIsStartedHacking = true;
}

void PlayerHackKeeper::endDemo() {
    if (rs::sendMsgHackDemoEnd(mHackHitSensor, mParentBodySensor))
        mIsStartedHacking = false;
}
