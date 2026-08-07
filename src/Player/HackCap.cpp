#include "Player/HackCap.h"

#include "Library/Collision/Collider.h"
#include "Library/Controller/InputFunction.h"
#include "Library/Controller/PadRumbleFunction.h"
#include "Library/Effect/EffectSystemInfo.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/MatrixUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Obj/ActorDitherAnimator.h"
#include "Library/Play/Layout/SimpleLayoutAppearWaitEnd.h"
#include "Project/Controller/PadRumbleKeeper.h"

#include "Player/CapTargetInfo.h"
#include "Player/HackCapJointControlKeeper.h"
#include "Player/HackCapJudgePreInputSeparateThrow.h"
#include "Player/HackCapStateHide.h"
#include "Player/HackCapThrowParam.h"
#include "Player/HackCapTrigger.h"
#include "Player/PlayerCapFunction.h"
#include "Player/PlayerExternalVelocity.h"
#include "Player/PlayerEyeSensorHitHolder.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerJointControlKeeper.h"
#include "Player/PlayerPushReceiver.h"
#include "Player/PlayerSeparateCapFlag.h"
#include "Player/PlayerWetControl.h"
#include "Util/DemoUtil.h"
#include "Util/JudgeUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerTypeUtil.h"
#include "Util/PlayerUtil.h"
#include "Util/ScenePlayerCameraFunction.h"
#include "Util/SensorMsgFunction.h"

namespace HackCapFunction {
void resetPositionAndCollision(al::LiveActor*, PlayerColliderHackCap*);
}

namespace {
NERVE_IMPL(HackCap, Catch);
NERVE_IMPL(HackCap, ThrowStay);

class HackCapNrvHide : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override {
        al::updateNerveState(keeper->getParent<HackCap>());
    }
};

NERVE_IMPL(HackCap, Return);
NERVE_IMPL(HackCap, LockOn);
NERVE_IMPL(HackCap, Rebound);
NERVE_IMPL(HackCap, Hack);
NERVE_IMPL(HackCap, SpinAttack);
NERVE_IMPL(HackCap, ThrowStart);
NERVE_IMPL(HackCap, Rescue);
NERVE_IMPL_(HackCap, TrampleReturn, Trample);
NERVE_IMPL(HackCap, ThrowTornado);
NERVE_END_IMPL(HackCap, ThrowSpiral);
NERVE_IMPL(HackCap, ThrowRolling);
NERVE_IMPL(HackCap, Throw);
NERVE_IMPL(HackCap, ThrowBrake);
NERVE_IMPL_(HackCap, Rethrow, Throw);
NERVE_IMPL(HackCap, ThrowAppend);
NERVE_IMPL(HackCap, ThrowRollingBrake);
NERVE_IMPL(HackCap, Trample);
NERVE_IMPL(HackCap, TrampleLockOn);
NERVE_IMPL(HackCap, Blow);
NERVES_MAKE_STRUCT(HackCap, Catch, ThrowStay, Hide, Return, LockOn, Rebound, Hack, SpinAttack,
                   ThrowStart, Rescue, TrampleReturn, ThrowTornado, ThrowSpiral, ThrowRolling,
                   Throw, ThrowBrake, Rethrow, ThrowAppend, ThrowRollingBrake, Trample,
                   TrampleLockOn, Blow);

const f32 sThrowRangeAppend[] = {500.0f, 300.0f, 500.0f, 200.0f, 180.0f};
const f32 sSeparateThrowRange[] = {700.0f, 550.0f};

bool isFlyingNerve(const HackCap* cap) {
    return al::isNerve(cap, &NrvHackCap.Throw) || al::isNerve(cap, &NrvHackCap.Rethrow) ||
           al::isNerve(cap, &NrvHackCap.ThrowBrake) || al::isNerve(cap, &NrvHackCap.ThrowRolling) ||
           al::isNerve(cap, &NrvHackCap.ThrowRollingBrake) ||
           al::isNerve(cap, &NrvHackCap.ThrowAppend) || al::isNerve(cap, &NrvHackCap.ThrowStay) ||
           al::isNerve(cap, &NrvHackCap.ThrowSpiral) ||
           al::isNerve(cap, &NrvHackCap.ThrowTornado) || al::isNerve(cap, &NrvHackCap.Blow);
}

__attribute__((noinline)) void updateEquipmentHatVisibility(
    bool isHide, al::LiveActor* equipmentHat, al::LiveActor* equipmentHatDepthShadow,
    const CapTargetInfo* targetInfo, bool keepAlive) {
    if (isHide) {
        if (al::isDead(equipmentHat))
            return;

        al::validateShadow(equipmentHat);
        if (al::isAlive(equipmentHatDepthShadow))
            equipmentHatDepthShadow->kill();
        al::hideModelIfShow(equipmentHat);
        if (!keepAlive)
            equipmentHat->kill();
        return;
    }

    if (al::isAlive(equipmentHat) && !al::isHideModel(equipmentHat))
        return;

    if (al::isDead(equipmentHat))
        equipmentHat->appear();
    al::showModelIfHide(equipmentHat);
    al::invalidateShadow(equipmentHat);
    if (targetInfo->isUseDepthShadow()) {
        if (al::isDead(equipmentHatDepthShadow))
            equipmentHatDepthShadow->appear();
    } else if (al::isAlive(equipmentHatDepthShadow)) {
        equipmentHatDepthShadow->kill();
    }
}
}  // namespace

void HackCap::recordCapJump(PlayerWallActionHistory*) {}

void HackCap::showPuppetCapSilhouette() {
    mIsHidePuppetCapSilhouette = false;
}

void HackCap::endHackThrowAndReturnHack() {
    mIsSeparateFlying = false;
    mPlayerBodySensor = nullptr;
    mHackCapJointControlKeeper->reset();

    al::startAction(this, "Capture");
    al::LiveActor* equipmentHatDepthShadow = mEquipmentHatDepthShadow;
    al::LiveActor* lockOnCapEyes = mLockOnCapEyes;
    al::startAction(mEquipmentHat, "Capture");
    al::startAction(equipmentHatDepthShadow, "Capture");
    if (al::isAlive(lockOnCapEyes) && !al::tryStartAction(lockOnCapEyes, "Capture"))
        al::tryStartActionIfNotPlaying(lockOnCapEyes, "Wait");

    al::setActionFrame(this, al::getActionFrameMax(this, "Capture"));
    al::setVelocityZero(this);
    followTarget();
    HackCapFunction::resetPositionAndCollision(this, mPlayerColliderHackCap);
    updateEquipmentHatVisibility(mIsHide, mEquipmentHat, mEquipmentHatDepthShadow,
                                 mCapTargetInfo1, false);

    al::LiveActor* lockOnCapEyesAfterReturn = mLockOnCapEyes;
    if (al::isAlive(lockOnCapEyesAfterReturn))
        lockOnCapEyesAfterReturn->kill();
    al::hideModelIfShow(this);
    al::invalidateHitSensors(this);
    al::setNerve(this, &NrvHackCap.Hack);
}

void HackCap::endHackShineGetDemo() {}

bool HackCap::isEnableRescuePlayer() const {
    return false;
}

void HackCap::makeFollowMtx(sead::Matrix34f* mtx) const {
    mCapTargetInfo1->makeLockOnMtx(mtx);
}

void HackCap::exeHide() {
    al::updateNerveState(this);
}

void HackCap::noticeHackMarioEnter() {
    _5bb = false;
    al::LiveActor* capEyes = mLockOnCapEyes;
    if (mCapTargetInfo1->isInvalidCapEye()) {
        if (al::isAlive(capEyes))
            capEyes->kill();
    } else if (al::isDead(capEyes)) {
        capEyes->appear();
    }
    al::startAction(mLockOnCapEyes, "Appear");
}

void HackCap::noticeHackDemoPuppetableEnd() {
    _5bb = false;
    al::LiveActor* capEyes = mLockOnCapEyes;
    if (mCapTargetInfo1->isInvalidCapEye()) {
        if (al::isAlive(capEyes))
            capEyes->kill();
    } else if (al::isDead(capEyes)) {
        capEyes->appear();
    }
    al::tryStartActionIfNotPlaying(mLockOnCapEyes, "Appear");
    al::setActionFrame(mLockOnCapEyes, al::getActionFrameMax(mLockOnCapEyes));
}

void HackCap::recordHack() {
    if (mCapTargetInfo1->isSetHackNameToCamera())
        PlayerCameraFunction::setCameraHackName(this, CapFunction::getHackObjInfo(this));
}

void HackCap::addHackStartDemo() {
    rs::addDemoActor(this, true);
    rs::addDemoActor(mEquipmentHat, true);
}

void HackCap::addLockOnKeepDemo() {
    rs::addDemoActor(this, true);
    rs::addDemoActor(mEquipmentHat, true);
}

void HackCap::endHack() {
    if (mCapTargetInfo1->isSetHackNameToCamera())
        PlayerCameraFunction::resetCameraHackName(this);
    hide(true);
    al::tryKillEmitterAndParticleAll(this);
    _2a2 = true;
    _2a3 = true;
}

void HackCap::syncHackDamageVisibility(bool visible) {
    mIsHackDamageVisible = visible;
}


void HackCap::startSpinAttack(const char* actionName) {
    al::showModelIfHide(this);
    al::startAction(mThrowingHatEyes, "Appear");
    al::setScaleAll(this, 1.0f);

    bool isSeparatePlay = false;
    _2a0 = true;
    _2a1 = true;
    _2a2 = false;
    _2a3 = false;
    _2a8 = false;
    if (actionName) {
        const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
        isSeparatePlay = (flags & 0xFF0000) == 0 && (flags & 0xFF) != 0;
    }
    _299 = isSeparatePlay;
    _2d8 = mInput->get_50();
    _1c8 = 0;
    _1c4 = 0;
    mRecentCollideActors.clear();
    _240.set(0.0f, 0.0f, 0.0f);
    _24c.set(0.0f, 0.0f, 0.0f);

    al::copyPose(this, mPlayerActor);
    al::setVelocityZero(this);
    if (actionName) {
        al::startAction(this, actionName);
        alPadRumbleFunction::startPadRumbleWithVolume(this, "EmBlowHitCommonS", 0.2f, 0.6f);
    }
    HackCapFunction::resetPositionAndCollision(this, mPlayerColliderHackCap);
    al::setNerve(this, &NrvHackCap.SpinAttack);
}


void HackCap::startThrowSeparatePlay(const sead::Vector3f& startPos,
                                     const sead::Vector3f& velocity, f32 speed,
                                     bool isThrowType) {
    sead::Vector3f throwOffset;
    throwOffset.set(0.0f, 0.0f, 0.0f);
    if (al::isNerve(this, &NrvHackCap.Hide))
        mStateHide->calcSeparateThrowOffset(&throwOffset);
    else {
        const sead::Vector3f& bodyPos = rs::getPlayerBodyPos(this);
        const sead::Vector3f& playerPos = rs::getPlayerPos(this);
        throwOffset.setSub(bodyPos, playerPos);
    }

    al::showModelIfHide(this);
    al::startAction(mThrowingHatEyes, "Appear");
    al::setScaleAll(this, 1.0f);
    _2a1 = true;
    _2a0 = true;
    _2a2 = false;
    _2a3 = false;
    _2a8 = false;
    _299 = false;
    _2d8 = mInput->get_50();
    _1c8 = 0;
    _1c4 = 0;
    mRecentCollideActors.clear();
    _240.set(0.0f, 0.0f, 0.0f);
    _24c.set(0.0f, 0.0f, 0.0f);
    al::copyPose(this, mPlayerActor);
    al::setVelocityZero(this);
    HackCapFunction::resetPositionAndCollision(this, mPlayerColliderHackCap);
    al::setNerve(this, &NrvHackCap.SpinAttack);
    startThrow(false, startPos, velocity, speed, sead::Vector2f::zero, sead::Vector2f::zero,
               sead::Vector3f::zero, isThrowType, throwOffset, SwingHandType::Value0, false, 0.0f,
               1);
}

void HackCap::startThrowSeparatePlayJump(const sead::Vector3f& startPos,
                                         const sead::Vector3f& velocity, f32 speed) {
    mHackCapTrigger->set(HackCapTrigger::Trigger0);
    startThrowSeparatePlay(startPos, velocity, speed, false);
}

void HackCap::forcePutOn() {
    hide(true);
    al::tryKillEmitterAndParticleAll(this);
    _2a2 = true;
    _2a3 = true;
    mBalloonFrame->kill();
}

bool HackCap::requestReturn(bool* isSuccess) {
    return tryReturn(false, isSuccess);
}

bool HackCap::tryReturn(bool force, bool* isSuccess) {
    if (isSuccess)
        *isSuccess = false;

    if (mIsSeparateFlying) {
        if (isSuccess)
            *isSuccess = true;
        return false;
    }

    if (al::isNerve(this, &NrvHackCap.ThrowStay)) {
        al::startHitReaction(this, "");
        al::setNerve(this, &NrvHackCap.Return);
        return true;
    }

    const bool useCatchSensor = mCapTargetInfo1->get_78();
    if (!mIsSeparateFlying && !al::isNerve(this, &NrvHackCap.Hack)) {
        if (!rs::sendMsgCapIgnoreCancelLockOn(const_cast<al::HitSensor*>(_1d0), mAttackSensor)) {
            rs::sendMsgCapCancelLockOn(const_cast<al::HitSensor*>(_1d0), mAttackSensor);
        } else if (force) {
            rs::sendMsgCapCancelLockOn(const_cast<al::HitSensor*>(_1d0), mAttackSensor);
        } else {
            if (isSuccess) {
                *isSuccess = !rs::sendMsgCapIgnoreCancelMissReaction(
                    const_cast<al::HitSensor*>(_1d0), mAttackSensor);
            }
            return false;
        }
    }

    if (_1d0)
        mRecentCollideActors.forcePushBack(al::getSensorHost(_1d0));

    if (useCatchSensor) {
        _290 = 0x708;
        al::offCollide(this);
        al::invalidateHitSensors(this);
        al::validateHitSensor(this, "Catch");
    } else {
        _290 = 3;
        al::offCollide(this);
        al::validateHitSensors(this);
        al::invalidateHitSensor(this, "SpiralTail1");
        al::invalidateHitSensor(this, "SpiralTail2");
        al::invalidateHitSensor(this, "SpiralTail3");
        al::invalidateHitSensor(this, "SpiralTail4");
        al::invalidateHitSensor(this, "SpiralTail5");
    }

    mCapTargetInfo1 = nullptr;
    _1d0 = nullptr;
    al::startHitReaction(this, "");

    if (!_299) {
        const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
        if ((flags & 0xFF0000) == 0 && (flags & 0xFF) != 0)
            _510 = false;
    }

    al::setNerve(this, &NrvHackCap.Return);
    return true;
}

void HackCap::startPuppet() {
    mIsPuppet = true;
    _5f9 = false;
    mIsHidePuppetCapSilhouette = false;
}

void HackCap::endPuppet() {
    mIsPuppet = false;
    _5f9 = false;
    mIsHidePuppetCapSilhouette = false;
    syncPuppetSilhouette();
}

void HackCap::hidePuppetCap() {
    _5f9 = true;
    if (al::isAlive(mEquipmentHat))
        al::hideModelIfShow(mEquipmentHat);
    else
        al::hideModelIfShow(this);
}

void HackCap::showPuppetCap() {
    _5f9 = true;
    if (al::isAlive(mEquipmentHat))
        al::showModelIfHide(mEquipmentHat);
    else
        al::showModelIfHide(this);
}

void HackCap::hidePuppetCapSilhouette() {
    mIsHidePuppetCapSilhouette = true;
}

void HackCap::startPuppetCheckpointWarp() {
    _5f9 = true;
    al::hideModelIfShow(this);
    al::hideModelIfShow(mEquipmentHat);
    al::hideModelIfShow(mEquipmentHatDepthShadow);
    al::tryKillEmitterAndParticleAll(this);
}

void HackCap::startHackShineGetDemo() {
    if (mIsSeparateFlying)
        endHackThrowAndReturnHack();
}

s32 HackCap::getPadRumblePort() const {
    return mPadRumbleKeeper->getPort();
}

f32 HackCap::getThrowHeight() const {
    return mHackCapThrowParam->getHackThrowHeight();
}

bool HackCap::isHackInvalidSeparatePlay() const {
    return mCapTargetInfo1 && mCapTargetInfo1->get_7e();
}

void HackCap::endHackThrow() {
    mIsSeparateFlying = false;
    mPlayerBodySensor = nullptr;
}

bool HackCap::requestLockOnHitReaction(const CapTargetInfo* targetInfo, const char* reaction) {
    if (!mCapTargetInfo1 || mCapTargetInfo1 != targetInfo)
        return false;

    al::startHitReaction(mEquipmentHat, reaction);
    return true;
}

// NON_MATCHING: exact 0x1a4 size; target stores `_1d0`, materializes "NoAction" in X21,
// then stores `mCapTargetInfo1`; current scheduling stores both members before the string.
void HackCap::forceHack(al::HitSensor* sensor, const CapTargetInfo* targetInfo) {
    mBalloonFrame->kill();
    resetLockOnParam();
    setupStartLockOn();

    _1d0 = sensor;
    const char* actionName = "NoAction";
    mCapTargetInfo1 = targetInfo;
    al::startAction(this, actionName);
    al::LiveActor* equipmentHatDepthShadow = mEquipmentHatDepthShadow;
    al::LiveActor* lockOnCapEyes = mLockOnCapEyes;
    al::startAction(mEquipmentHat, actionName);
    al::startAction(equipmentHatDepthShadow, actionName);
    if (al::isAlive(lockOnCapEyes) && !al::tryStartAction(lockOnCapEyes, actionName))
        al::tryStartActionIfNotPlaying(lockOnCapEyes, "Wait");

    const f32 frameMax = al::getActionFrameMax(this, actionName);
    al::setActionFrame(this, frameMax);
    al::setActionFrame(mEquipmentHat, frameMax);
    al::setActionFrame(equipmentHatDepthShadow, frameMax);
    al::setNerve(this, &NrvHackCap.LockOn);

    mIsHide = false;
    mIsHackDamageVisible = false;
    _5ba = false;
    _5bb = true;
    _5bc = 0;
    al::invalidateHitSensor(this, "Trample");
    al::setNerve(this, &NrvHackCap.Hack);
}

void HackCap::resetLockOnParam() {
    mCapExternalVelocity->reset();
    mCapTargetInfo1 = nullptr;
    _580 = 0;
    mLockOnCounter = 0;
    _2a6 = false;
    al::makeMtxRT(&_588, this);
}

void HackCap::setupStartLockOn() {
    al::setVelocityZero(this);
    al::offCollide(this);
    al::invalidateHitSensors(this);
    al::validateHitSensor(this, "Trample");
    _29e = false;
    _274 = 0;
    _264 = 0.0f;
    _268.set(0.0f, 0.0f);
    _270 = 0.0f;
    if (_298)
        al::tryStartActionIfNotPlaying(mThrowingHatEyes, "SpinL");
    else
        al::tryStartActionIfNotPlaying(mThrowingHatEyes, "SpinR");
}

void HackCap::prepareLockOn(al::HitSensor* sensor) {
    if (mIsSeparateFlying) {
        al::setNerve(this, &NrvHackCap.LockOn);
        rs::sendMsgTransferHack(mPlayerBodySensor, const_cast<al::HitSensor*>(_1d0));
        mIsSeparateFlying = false;
        mPlayerBodySensor = nullptr;
    }

    resetLockOnParam();
    if (!rs::sendMsgInitCapTarget(
            sensor, mAttackSensor,
            &mCapTargetInfo1)) {
        CapTargetInfoFunction::initIterCapTargetInfo(mCapTargetInfo2, nullptr,
                                                     al::getSensorHost(sensor), nullptr);
        mCapTargetInfo1 = mCapTargetInfo2;
    }

    _1d0 = sensor;
    if (al::isNerve(this, &NrvHackCap.SpinAttack) ||
        al::isNerve(this, &NrvHackCap.ThrowStart) || al::isNerve(this, &NrvHackCap.Hide))
        al::startAction(this, "FlyingWaitR");

    _2a8 = mCapTargetInfo1->get_7f();
    al::startHitReaction(this, "");
    setupStartLockOn();
    al::setNerve(this, &NrvHackCap.LockOn);
}

f32 HackCap::getThrowSpeed() const {
    if (_29f)
        return 60.0f;

    if (al::isNearZero(_268.x) && !al::isNearZero(_268.y))
        return mHackCapThrowParam->getRollSpeed();

    if (_29a)
        return mHackCapThrowParam->getWaterMaxSpeed();

    if (_29e || _4d0)
        return mHackCapThrowParam->getContinuousThrowSpeed();

    return mHackCapThrowParam->getMaxVel();
}

// NON_MATCHING: target uses signed `CMP #4`/`B.GE`; current compiler canonicalizes the
// equivalent `mLockOnCounter >= 4` to `CMP #3`/`B.GT`.
void HackCap::updateThrowJoint() {
    bool useNormalRotation = true;
    if (al::isNerve(this, &NrvHackCap.LockOn) && !isLockOnInterpolate())
        useNormalRotation = false;

    f32 rotateY;
    if (useNormalRotation) {
        static const f32 rotateYValues[] = {35.0f, 45.0f};
        rotateY = _29e ? 45.0f : rotateYValues[_4d0];
        if (_29a)
            rotateY = 20.0f;
    } else {
        rotateY = _29a ? 5.0f : 10.0f;
    }

    if (_298)
        rotateY = -rotateY;
    mHackCapJointControlKeeper->updateRotateY(rotateY);

    if (al::isNerve(this, &NrvHackCap.ThrowStay))
        mHackCapJointControlKeeper->updateRotateStayZ(0.25f);
    else
        mHackCapJointControlKeeper->updateRotateThrowZ(0.002f, 0.0f);
}

void HackCap::exeTrample() {
    if (al::isFirstStep(this)) {
        sead::Vector3f front(0.0f, 0.0f, 0.0f);
        al::calcJointFrontDir(&front, this, "AllRoot");
        al::faceToDirection(this, front);
        al::startAction(this, "Trample");
        al::startAction(mThrowingHatEyes, "Wait");
        al::setVelocityZero(this);
        mHackCapJointControlKeeper->resetRotateZ();
        al::startHitReaction(this, "通常Trample");

        const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
        if ((flags & 0xFF0000) == 0 && (flags & 0xFF) != 0)
            al::startHitReaction(this, "おすそ分けTrample");
    }

    if (!al::isLessStep(this, 15)) {
        if (al::isNerve(this, &NrvHackCap.TrampleReturn) || _299 ||
            (mPlayerSeparateCapFlag->getRawFlags() & 0xFF0000) != 0 ||
            (mPlayerSeparateCapFlag->getRawFlags() & 0xFF) == 0 || !_510)
            al::setNerve(this, &NrvHackCap.Return);
        else
            al::setNerve(this, &NrvHackCap.ThrowStay);
    }
}

void HackCap::exeTrampleLockOn() {
    if (al::isFirstStep(this)) {
        sead::Vector3f front(0.0f, 0.0f, 0.0f);
        al::calcJointFrontDir(&front, this, "AllRoot");
        al::faceToDirection(this, front);

        const char* actionName = "TrampleLockOn";
        al::LiveActor* equipmentHatDepthShadow = mEquipmentHatDepthShadow;
        al::LiveActor* lockOnCapEyes = mLockOnCapEyes;
        al::startAction(mEquipmentHat, actionName);
        al::startAction(equipmentHatDepthShadow, actionName);
        if (al::isAlive(lockOnCapEyes) && !al::tryStartAction(lockOnCapEyes, actionName))
            al::tryStartActionIfNotPlaying(lockOnCapEyes, "Wait");
    }

    if (!al::isLessStep(this, 15)) {
        if (!mCapTargetInfo1->isLockOn()) {
            const char* actionName = mCapTargetInfo1->getLockOnAnimName();
            al::LiveActor* equipmentHatDepthShadow = mEquipmentHatDepthShadow;
            al::LiveActor* lockOnCapEyes = mLockOnCapEyes;
            al::startAction(mEquipmentHat, actionName);
            al::startAction(equipmentHatDepthShadow, actionName);
            if (al::isAlive(lockOnCapEyes) && !al::tryStartAction(lockOnCapEyes, actionName))
                al::tryStartActionIfNotPlaying(lockOnCapEyes, "Wait");
        }
        al::setNerve(this, &NrvHackCap.LockOn);
    }
}

// NON_MATCHING: current 0x108 body hoists the first-step eye load after the combined flag
// test; target is 0x114 and duplicates the load across the low/high flag branches.
void HackCap::exeHack() {
    if (al::isFirstStep(this)) {
        updateEquipmentHatVisibility(mIsHide, mEquipmentHat, mEquipmentHatDepthShadow,
                                     mCapTargetInfo1, false);
        if (mIsHide || _5ba || mCapTargetInfo1->isInvalidCapEye()) {
            al::LiveActor* lockOnCapEyes = mLockOnCapEyes;
            if (al::isAlive(lockOnCapEyes))
                lockOnCapEyes->kill();
        } else {
            al::LiveActor* lockOnCapEyes = mLockOnCapEyes;
            if (al::isDead(lockOnCapEyes))
                lockOnCapEyes->appear();
        }
        al::hideModelIfShow(this);
        _5bc = 0;
    }

    if (al::isAlive(mLockOnCapEyes)) {
        if (al::isActionPlaying(mLockOnCapEyes, "Disappear")) {
            if (al::isActionEnd(mLockOnCapEyes)) {
                _5ba = true;
                al::startAction(mLockOnCapEyes, "Wait");
            }
        } else if (al::isActionPlaying(mLockOnCapEyes, "Appear") &&
                   al::isActionEnd(mLockOnCapEyes)) {
            al::startAction(mLockOnCapEyes, "Wait");
        }
    }
}

// NON_MATCHING: behavior is recovered, but target retains a different eye-actor register
// lifetime and branch layout; next hypothesis is to preserve branch-local member reloads.
void HackCap::updateCapPose() {
    mActorDitherAnimator->update();

    const bool isLockOnPose = al::isNerve(this, &NrvHackCap.LockOn) ||
                              al::isNerve(this, &NrvHackCap.Hack) ||
                              al::isNerve(this, &NrvHackCap.TrampleLockOn);
    if (!isLockOnPose) {
        if (al::isNerve(this, &NrvHackCap.Hide)) {
            rs::syncPlayerModelAlpha(this);
            rs::syncPlayerModelAlpha(mEquipmentHat);
        } else {
            const f32 alpha = mActorDitherAnimator->getDitherAlpha();
            al::setModelAlphaMask(this, alpha);
            al::setModelAlphaMask(mEquipmentHat, alpha);
        }
        if (mIsPuppet)
            syncPuppetSilhouette();
        return;
    }

    if (!_5bb) {
        bool hideEquipmentHat = false;
        if (al::isNerve(this, &NrvHackCap.Hack)) {
            hideEquipmentHat = mIsHide || mIsHackDamageVisible;
            updateEquipmentHatVisibility(hideEquipmentHat, mEquipmentHat,
                                         mEquipmentHatDepthShadow, mCapTargetInfo1, true);
            if (hideEquipmentHat || _5ba || _5bb) {
                if (al::isAlive(mLockOnCapEyes))
                    mLockOnCapEyes->kill();
            } else if (!mCapTargetInfo1->isInvalidCapEye() && al::isDead(mLockOnCapEyes)) {
                mLockOnCapEyes->appear();
            } else if (mCapTargetInfo1->isInvalidCapEye() && al::isAlive(mLockOnCapEyes)) {
                mLockOnCapEyes->kill();
            }
        } else {
            if (al::isNerve(this, &NrvHackCap.LockOn))
                hideEquipmentHat = mIsSeparateFlying || mCapTargetInfo1->get_72();
            else
                hideEquipmentHat = mIsSeparateFlying;

            updateEquipmentHatVisibility(hideEquipmentHat, mEquipmentHat,
                                         mEquipmentHatDepthShadow, mCapTargetInfo1, false);
            if (!hideEquipmentHat && mCapTargetInfo1->isLockOnOnly() &&
                !mCapTargetInfo1->isInvalidCapEye()) {
                if (al::isDead(mLockOnCapEyes))
                    mLockOnCapEyes->appear();
            } else if (al::isAlive(mLockOnCapEyes)) {
                mLockOnCapEyes->kill();
            }
        }
    }

    followTarget();
    HackCapFunction::resetPositionAndCollision(this, mPlayerColliderHackCap);
    al::setScale(mEquipmentHat, al::getScale(this));
    al::copyPose(mEquipmentHat, this);
    al::resetPosition(mEquipmentHat);
    al::resetPosition(mEquipmentHatDepthShadow);
    al::resetPosition(mLockOnCapEyes);

    if (mCapTargetInfo1->isExistModel()) {
        const f32 alpha = al::getModelAlphaMask(mCapTargetInfo1->getActor());
        al::setModelAlphaMask(this, alpha);
        al::setModelAlphaMask(mEquipmentHat, alpha);
    }

    if (mIsPuppet)
        syncPuppetSilhouette();
}

void HackCap::followTarget() {
    if (mLockOnCounter == 4) {
        sead::Matrix34f targetMtx;
        mCapTargetInfo1->makeLockOnMtx(&targetMtx);
        al::updatePoseMtx(this, &targetMtx);
        sead::Vector3f scale(1.0f, 1.0f, 1.0f);
        mCapTargetInfo1->calcLockOnFollowTargetScale(&scale);
        al::setScale(this, scale);
        return;
    }

    mLockOnCounter = al::converge(mLockOnCounter, 4, 1);
    const f32 rate = mLockOnCounter * 0.25f;

    sead::Matrix34f targetMtx;
    mCapTargetInfo1->makeLockOnMtx(&targetMtx);
    sead::Matrix34f blendedMtx = sead::Matrix34f::ident;
    al::blendMtx(&blendedMtx, _588, targetMtx, rate);

    sead::Vector3f trans;
    blendedMtx.getTranslation(trans);
    al::updatePoseTrans(this, trans);

    sead::Quatf quat = sead::Quatf::unit;
    _588.toQuat(quat);
    sead::Vector3f up;
    blendedMtx.getBase(up, 1);
    al::turnQuatYDirRate(&quat, quat, up, 1.0f);
    al::updatePoseQuat(this, quat);
}

void HackCap::calcHackFollowTrans(sead::Vector3f* trans, bool useLocalOffset) const {
    sead::Matrix34f followMtx = sead::Matrix34f::ident;
    mCapTargetInfo1->makeLockOnMtx(&followMtx);

    if (useLocalOffset && mCapTargetInfo1->isEscapeLocalOffset())
        al::calcTransLocalOffsetByMtx(trans, followMtx, mCapTargetInfo1->getEscapeLocalOffset());
    else
        followMtx.getTranslation(*trans);
}

void HackCap::updateCapEyeShowHide(bool isShow, s32 appearDelay) {
    if (_5ba) {
        _5bc = isShow ? al::converge(_5bc, appearDelay, 1) : 0;
        _5ba = _5bc < appearDelay;
        if (_5bc >= appearDelay)
            al::startAction(mLockOnCapEyes, "Appear");
    } else if (!isShow && !al::isActionPlaying(mLockOnCapEyes, "Disappear")) {
        al::startAction(mLockOnCapEyes, "Disappear");
        _5bc = 0;
    }
}

bool HackCap::isSeparateHipDropLand() const {
    return rs::isJustLand(mPlayerColliderHackCap) && mStateThrowStay->isEnableSendHipDropMsg();
}

void HackCap::prepareCooperateThrow() {
    hide(true);
    al::hideModelIfShow(this);
    al::offCollide(this);
    al::tryStartActionIfNotPlaying(this, "WaitSeparate");
}

bool HackCap::isHoldInputKeepLockOn() const {
    return _299 ? mInput->isHoldAction() : mInput->isHoldCapAction();
}

bool HackCap::isForceCapTouchJump() const {
    if (_299)
        return false;

    const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
    return (flags & 0xFF0000) == 0 && (flags & 0xFF) != 0 && mStateThrowStay->isHomingPlayerJump();
}

bool HackCap::isHoldSpinCapStay() const {
    if (_299)
        return mInput->isHoldAction();

    const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) != 0 || (flags & 0xFF) == 0)
        return mInput->isHoldSpinCap();

    return _510;
}

bool HackCap::isThrowTypeSpiral() const {
    return !al::isNearZero(_268.x);
}

bool HackCap::isThrowTypeRolling() const {
    return al::isNearZero(_268.x) && !al::isNearZero(_268.y);
}

void HackCap::setupThrowStart() {
    _290 = 0;
    _294 = 0;
    al::validateHitSensors(this);
    al::invalidateHitSensor(this, "SpiralTail1");
    al::invalidateHitSensor(this, "SpiralTail2");
    al::invalidateHitSensor(this, "SpiralTail3");
    al::invalidateHitSensor(this, "SpiralTail4");
    al::invalidateHitSensor(this, "SpiralTail5");
    al::validateShadow(this);
    al::onCollide(this);

    const char* actionLeft = _29e ? "FlyingWaitCapCatchL" : "FlyingWaitL";
    const char* actionRight = _29e ? "FlyingWaitCapCatchR" : "FlyingWaitR";
    al::startAction(this, _264 > 0.0f ? actionLeft : actionRight);
}

s32 HackCap::getThrowBrakeTime() const {
    if (_29f)
        return 2;

    if (al::isNearZero(_268.x) && !al::isNearZero(_268.y))
        return mHackCapThrowParam->getRollBrakeTime(_268.y > 0.0f);

    const bool isNormal = !_29a;
    return mHackCapThrowParam->getBreakTimeNormal(isNormal);
}

void HackCap::startThrowCapEyeThrowAction() {
    if (_298)
        al::tryStartActionIfNotPlaying(mThrowingHatEyes, "SpinL");
    else
        al::tryStartActionIfNotPlaying(mThrowingHatEyes, "SpinR");
}

void HackCap::endThrowSpiral() {
    al::invalidateHitSensor(this, "SpiralTail1");
    al::invalidateHitSensor(this, "SpiralTail2");
    al::invalidateHitSensor(this, "SpiralTail3");
    al::invalidateHitSensor(this, "SpiralTail4");
    al::invalidateHitSensor(this, "SpiralTail5");
}

bool HackCap::tryChangeSeparateThrow() {
    if (!_299)
        return false;

    const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) != 0 || (flags & 0xFF) == 0)
        return false;
    if (mInput->isNoInputSeparateCap())
        return false;

    _299 = false;
    mRecentCollideActors.clear();
    _510 = true;
    rs::resetJudge(mCapJudgePreInputSeparateThrow);
    return true;
}

f32 HackCap::getThrowBackSpeed() const {
    return _29a ? mHackCapThrowParam->getWaterMaxRetSpeed() : mHackCapThrowParam->getMaxRetSpeed();
}

s32 HackCap::getThrowStayTime() const {
    return _29f ? 3 : mHackCapThrowParam->getEndpointStopTime();
}

s32 HackCap::getThrowStayTimeMax() const {
    if (_299)
        return mHackCapThrowParam->getMaxEndpointStopTime();

    const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) != 0 || (flags & 0xFF) == 0)
        return mHackCapThrowParam->getMaxEndpointStopTime();

    return 120;
}

f32 HackCap::getThrowSpeedAppend() const {
    if (_29f)
        return 60.0f;
    if (!_29a && (_29e || _4d0))
        return mHackCapThrowParam->getContinuousThrowSpeed();
    return mHackCapThrowParam->getMaxVel();
}

f32 HackCap::getThrowRangeAppend() const {
    return sThrowRangeAppend[static_cast<u8>(_2a5)];
}

void HackCap::endHackThrowAndReturnHackOrHide() {
    if (mIsSeparateFlying)
        endHackThrowAndReturnHack();
    else
        hide(false);
}

void HackCap::clearThrowType() {
    _29e = false;
    _274 = 0;
    _264 = 0.0f;
    _268.set(0.0f, 0.0f);
    _270 = 0.0f;
    if (_298)
        al::tryStartActionIfNotPlaying(mThrowingHatEyes, "SpinL");
    else
        al::tryStartActionIfNotPlaying(mThrowingHatEyes, "SpinR");
}

void HackCap::calcReturnTargetPos(sead::Vector3f* targetPos) const {
    if (mIsSeparateFlying) {
        sead::Matrix34f followMtx = sead::Matrix34f::ident;
        mCapTargetInfo1->makeLockOnMtx(&followMtx);
        followMtx.getTranslation(*targetPos);
    } else {
        targetPos->set(rs::getPlayerHeadPos(this));
    }
}

void HackCap::endMove() {
    al::offCollide(this);
    al::invalidateShadow(this);
    al::invalidateHitSensors(this);
    mCapTargetInfo1 = nullptr;
    _1d0 = nullptr;
    _2a8 = false;
    al::setVelocityZero(this);
}

bool HackCap::isEnableCapTouchJumpInput() const {
    if (_299) {
        if (!mInput->isHoldAction())
            return false;
    } else {
        const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
        if ((flags & 0xFF0000) != 0 || (flags & 0xFF) == 0) {
            if (!mInput->isHoldSpinCap())
                return false;
        } else if (!_510) {
            return false;
        }
    }

    if (_299)
        return true;

    const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) == 0 && (flags & 0xFF) != 0) {
        if (!mInput->isTriggerCapSeparateJump())
            return mStateThrowStay->isHomingPlayerJump();
    }
    return true;
}

bool HackCap::isEnableThrow() const {
    return al::isNerve(this, &NrvHackCap.SpinAttack);
}

bool HackCap::isSpinAttack() const {
    return al::isNerve(this, &NrvHackCap.SpinAttack);
}

bool HackCap::isRescuePlayer() const {
    return al::isNerve(this, &NrvHackCap.Rescue);
}

bool HackCap::isHide() const {
    return al::isNerve(this, &NrvHackCap.Hide);
}

bool HackCap::isPutOn() const {
    return al::isNerve(this, &NrvHackCap.Hide) && _2a2;
}

bool HackCap::isNoPutOnHide() const {
    return al::isNerve(this, &NrvHackCap.Hide) && !_2a2;
}

bool HackCap::isSeparateHide() const {
    return al::isNerve(this, &NrvHackCap.Hide) && mStateHide->isSeparateMode();
}

bool HackCap::isRequestableReturn() const {
    if (!mCapTargetInfo1)
        return false;

    const bool isCatch = _2a6;
    if (!al::isNerve(this, &NrvHackCap.LockOn))
        return false;

    const u32 isAfterStart = al::isLessEqualStep(this, 5) ^ 1;
    return static_cast<bool>(static_cast<u32>(isCatch) * isAfterStart);
}

bool HackCap::isLockOnEnableHackTarget() const {
    return al::isNerve(this, &NrvHackCap.LockOn) && mCapTargetInfo1 &&
           !mCapTargetInfo1->isLockOnOnly();
}

bool HackCap::isWaitHackLockOn() const {
    if (!mCapTargetInfo1)
        return false;

    const bool isCatch = _2a6;
    if (!al::isNerve(this, &NrvHackCap.LockOn))
        return false;

    const u32 isAfterStart = al::isLessEqualStep(this, 5) ^ 1;
    if (!isCatch || !isAfterStart)
        return false;
    return !mCapTargetInfo1->isLockOnOnly();
}

bool HackCap::isCatched() const {
    return (al::isNerve(this, &NrvHackCap.Catch) || al::isNerve(this, &NrvHackCap.Hide)) && _2a3;
}

bool HackCap::isLockOnInterpolate() const {
    return (al::isNerve(this, &NrvHackCap.LockOn) || al::isNerve(this, &NrvHackCap.Hack) ||
            al::isNerve(this, &NrvHackCap.TrampleLockOn)) &&
           mLockOnCounter < 4;
}

bool HackCap::isFlying() const {
    return isFlyingNerve(this) || al::isNerve(this, &NrvHackCap.Return) ||
           al::isNerve(this, &NrvHackCap.Rebound);
}

bool HackCap::isEnableHackThrow(bool* isCapEyeValid) const {
    if (!al::isNerve(this, &NrvHackCap.Hack))
        return false;
    *isCapEyeValid = !mCapTargetInfo1->get_7e();
    return !mCapTargetInfo1->isInvalidHackThrow();
}

void HackCap::requestForceFollowSeparateHide() {
    if (al::isNerve(this, &NrvHackCap.Hide))
        mStateHide->requestForceFollowSeparate();
}

bool HackCap::isEnableThrowSeparate() const {
    const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) != 0 || (flags & 0xFF) == 0)
        return false;
    if (al::isNerve(this, &NrvHackCap.Hide))
        return true;
    return al::isNerve(this, &NrvHackCap.Catch);
}

void HackCap::exeRescue() {
    if (!rs::isPlayerSafetyPointRecovery(this))
        al::setNerve(this, &NrvHackCap.Hide);
}

void HackCap::startHack() {
    mIsHide = false;
    mIsHackDamageVisible = false;
    _5ba = false;
    _5bb = true;
    _5bc = 0;
    al::invalidateHitSensor(this, "Trample");
    al::setNerve(this, &NrvHackCap.Hack);
}

bool HackCap::isSeparateThrowFlying() const {
    if ((isFlyingNerve(this) || al::isNerve(this, &NrvHackCap.Return) ||
         al::isNerve(this, &NrvHackCap.Rebound)) &&
        !_299) {
        const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
        return (flags & 0xFF0000) == 0 && (flags & 0xFF) != 0;
    }
    return false;
}

bool HackCap::isEnablePreInput() const {
    if (_2a8)
        return false;
    if (_299)
        return true;

    const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) == 0 && (flags & 0xFF) != 0) {
        if (isFlyingNerve(this) || al::isNerve(this, &NrvHackCap.Return) ||
            al::isNerve(this, &NrvHackCap.Rebound)) {
            if (!al::isNerve(this, &NrvHackCap.Return))
                return false;
        }
    }
    return true;
}

// NON_MATCHING: exact behavior and size; target schedules `CMP upper` before masking the
// low byte, while current scheduling emits the low-byte `AND` first.
f32 HackCap::calcSeparateHideSpeedH(const sead::Vector3f& dir) const {
    const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) != 0 || (flags & 0xFF) == 0)
        return 0.0f;
    if (!al::isNerve(this, &NrvHackCap.Hide))
        return 0.0f;
    return al::calcSpeedExceptDir(this, dir);
}

bool HackCap::isEnableSpinAttack() const {
    if (al::isNerve(this, &NrvHackCap.Hide))
        return true;
    if (al::isNerve(this, &NrvHackCap.Catch))
        return true;
    if (_299)
        return false;

    const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) == 0 && (flags & 0xFF) != 0) {
        if (al::isNerve(this, &NrvHackCap.ThrowStay) && !mIsSeparateFlying)
            return al::calcDistance(this, mPlayerActor) <= 200.0f;
    }
    return false;
}

void HackCap::prepareTransferLockOn(al::HitSensor*) {
    if (mIsSeparateFlying) {
        al::setNerve(this, &NrvHackCap.LockOn);
        rs::sendMsgTransferHack(mPlayerBodySensor, const_cast<al::HitSensor*>(_1d0));
        mIsSeparateFlying = false;
        mPlayerBodySensor = nullptr;
    }
}

void HackCap::startThrowSeparatePlayHack(al::HitSensor* sensor, const sead::Vector3f& startPos,
                                         const sead::Vector3f& velocity, f32 speed) {
    if (al::isNerve(this, &NrvHackCap.Hack)) {
        mPlayerBodySensor = sensor;
        mIsSeparateFlying = true;
        mEquipmentHat->kill();
        mLockOnCapEyes->kill();
        startThrowSeparatePlay(startPos, velocity, speed, false);
    }
}

void HackCap::updateModelAlphaForSnapShot() {
    if (al::isNerve(this, &NrvHackCap.Hide)) {
        rs::syncPlayerModelAlpha(this);
        rs::syncPlayerModelAlpha(mEquipmentHat);
    } else {
        mActorDitherAnimator->update();
        const f32 alpha = mActorDitherAnimator->getDitherAlpha();
        al::setModelAlphaMask(this, alpha);
        al::setModelAlphaMask(mEquipmentHat, alpha);
    }
}

void HackCap::syncPuppetSilhouette() {
    const bool isHideSilhouette = mIsHidePuppetCapSilhouette;
    const bool isSelfAlive = al::isAlive(this);
    if (isHideSilhouette) {
        if (isSelfAlive && !al::isHideModel(this))
            al::hideSilhouetteModelIfShow(this);
        if (al::isAlive(mEquipmentHat) && !al::isHideModel(mEquipmentHat))
            al::hideSilhouetteModelIfShow(mEquipmentHat);
    } else {
        if (isSelfAlive && !mIsSeparateFlying && !al::isNerve(this, &NrvHackCap.Hack) &&
            !al::isHideModel(this))
            al::showSilhouetteModelIfHide(this);
        if (al::isAlive(mEquipmentHat) && !al::isHideModel(mEquipmentHat))
            al::showSilhouetteModelIfHide(mEquipmentHat);
    }
}

void HackCap::updateCollider() {
    sead::Vector3f pushedVelocity(0.0f, 0.0f, 0.0f);
    PlayerPushReceiver* pushReceiver = mCapPushReceiver;
    sead::Vector3f velocity = al::getVelocity(this) + mCapExternalVelocity->getTotalVelocity();
    pushReceiver->calcPushedVelocity(&pushedVelocity, velocity);
    rs::updateCollider(this, mPlayerColliderHackCap, pushedVelocity);
    mCapPushReceiver->clear();
    updateFrameOutLayout();

    if (!_5e8 && mStateThrowStay->isEnableSendHipDropMsg())
        mStateThrowStay->sendHipDropCollideMsg(mAttackSensor);

    if (isFlyingNerve(this) || al::isNerve(this, &NrvHackCap.Return) ||
        al::isNerve(this, &NrvHackCap.Rebound)) {
        HackCapAboveGroundChecker* checker = mHackCapAboveGroundChecker;
        velocity = -al::getGravity(this);
        checker->update(velocity);
    }
}

void HackCap::movement() {
    mCapEyeSensorHitHolder->clear();
    if (rs::isCollidedGround(mPlayerColliderHackCap)) {
        al::setMaterialCode(this, rs::getMaterialCodeGround(mPlayerColliderHackCap));
        al::updateMaterialCodePuddle(this);
    }

    al::LiveActor::movement();
    mHackCapTrigger->clearAfterMovemetTrigger();
    if (al::isNerve(this, &NrvHackCap.Hide))
        mCapExternalVelocity->reset();
    else
        mCapExternalVelocity->update();
    updateShadowMaskOffset();
    mSelfPlayerWetControl->update();
}

bool HackCap::sendMsgStartHack(al::HitSensor* sensor) {
    if (!mCapTargetInfo1)
        return false;

    const u8 isEnable = _2a6;
    if (!al::isNerve(this, &NrvHackCap.LockOn))
        return false;

    const u32 isEnableStep = al::isLessEqualStep(this, 5) ^ 1;
    if (!isEnable || !isEnableStep)
        return false;

    return rs::sendMsgStartHack(const_cast<al::HitSensor*>(_1d0), sensor);
}

f32 HackCap::getFlyingSpeedMax() const {
    if (isFlyingNerve(this) || al::isNerve(this, &NrvHackCap.Return) ||
        al::isNerve(this, &NrvHackCap.Rebound)) {
        if (_29f)
            return 60.0f;
        if (al::isNearZero(_268.x) && !al::isNearZero(_268.y))
            return mHackCapThrowParam->getRollSpeed();
        if (_29a)
            return mHackCapThrowParam->getWaterMaxSpeed();
        if (_29e || _4d0)
            return mHackCapThrowParam->getContinuousThrowSpeed();
        return mHackCapThrowParam->getMaxVel();
    }
    return mHackCapThrowParam->getMaxVel();
}

bool HackCap::stayRollingOrReflect() {
    if (_299) {
        if (mInput->isHoldAction()) {
            _294 = 20;
            return stayWallHit();
        }
    } else {
        const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
        if ((flags & 0xFF0000) != 0 || (flags & 0xFF) == 0) {
            if (mInput->isHoldSpinCap()) {
                _294 = 20;
                return stayWallHit();
            }
        } else {
            if (mStateThrowStay->isEnableSendHipDropMsg() || isHoldSpinCapStay())
                return true;
        }
    }

    al::setNerve(this, &NrvHackCap.Rebound);
    return true;
}

bool HackCap::receiveRequestTransferHack(al::HitSensor* sensor, al::HitSensor* playerSensor) {
    if (mIsSeparateFlying || !al::isNerve(this, &NrvHackCap.Hack) || rs::isActiveDemo(this) ||
        !rs::sendMsgCapStartLockOn(sensor, mAttackSensor))
        return false;

    if (al::isNerve(this, &NrvHackCap.Hack)) {
        mPlayerBodySensor = playerSensor;
        mIsSeparateFlying = true;
        mEquipmentHat->kill();
        mLockOnCapEyes->kill();
        startThrowSeparatePlay(sead::Vector3f::ez, sead::Vector3f::ey, 0.0f, false);
    }
    prepareLockOn(sensor);
    return true;
}

void HackCap::updateSeparateMode(const PlayerSeparateCapFlag* separateCapFlag) {
    const u32 flags = separateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) == 0 && (flags & 0xFF) != 0)
        mPadRumbleKeeper->setPort(al::getPlayerControllerPort(1));
    else
        mPadRumbleKeeper->setPort(al::getPlayerControllerPort(0));

    if (!separateCapFlag->isSeparateCapLocal() || separateCapFlag->isPuppetable()) {
        if (al::isNerve(this, &NrvHackCap.Hide) && mStateHide->isSeparateMode())
            _2a2 = false;
    }
}

bool HackCap::isEnableHackThrowAutoCatch() const {
    if (!mIsSeparateFlying)
        return false;

    sead::Matrix34f lockOnMtx = sead::Matrix34f::ident;
    mCapTargetInfo1->makeLockOnMtx(&lockOnMtx);
    sead::Vector3f lockOnPos;
    lockOnMtx.getTranslation(lockOnPos);
    return (lockOnPos - al::getTrans(this)).length() < 100.0f;
}

void HackCap::startCatch(const char* actionName, bool isHitReaction, const sead::Vector3f& hitPos) {
    mHackCapJointControlKeeper->reset();
    al::copyPose(this, mPlayerActor);
    al::startAction(this, actionName);
    if (isHitReaction)
        al::startHitReactionHitEffect(this, "帽子キャッチ", hitPos);
    if (al::isNerve(this, &NrvHackCap.Hide) && !_2a2)
        _2a2 = true;
    mEquipmentHat->kill();
    mLockOnCapEyes->kill();
    al::setNerve(this, &NrvHackCap.Catch);
}

void HackCap::startRescuePlayer() {
    if (al::isNerve(this, &NrvHackCap.Return))
        al::startHitReaction(this, "プレイヤー救出消滅");
    hide(true);
    al::hideModelIfShow(this);
    al::offCollide(this);
    al::tryStartActionIfNotPlaying(this, "WaitSeparate");
    al::setNerve(this, &NrvHackCap.Rescue);
}

void HackCap::hide(bool force) {
    if (al::isNerve(this, &NrvHackCap.Hide)) {
        if (!force)
            return;
        const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
        if ((flags & 0xFF0000) == 0 && (flags & 0xFF) != 0)
            return;
        if (!mStateHide->isSeparateMode())
            return;
        mStateHide->cancelSeparateMode();
    }

    if (mIsSeparateFlying)
        endHackThrowAndReturnHack();
    if (_1d0)
        tryReturn(force, nullptr);
    al::offCollide(this);
    al::invalidateShadow(this);
    al::invalidateHitSensors(this);
    mCapTargetInfo1 = nullptr;
    _1d0 = nullptr;
    _2a8 = false;
    al::setVelocityZero(this);
    al::hideModelIfShow(this);
    mEquipmentHat->kill();
    mLockOnCapEyes->kill();
    al::updateMaterialCodeWater(this, false);
    al::setNerve(this, &NrvHackCap.Hide);
}

bool HackCap::cancelCapState() {
    bool canCancel = al::isNerve(this, &NrvHackCap.SpinAttack) ||
                     al::isNerve(this, &NrvHackCap.Hide) || al::isNerve(this, &NrvHackCap.Catch);
    if (!canCancel && !_299) {
        const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
        canCancel = (flags & 0xFF0000) == 0 && (flags & 0xFF) != 0 &&
                    al::isNerve(this, &NrvHackCap.ThrowStay) && !mIsSeparateFlying &&
                    al::calcDistance(this, mPlayerActor) <= 200.0f;
    }
    if (!canCancel && !al::isNerve(this, &NrvHackCap.SpinAttack))
        return false;

    if (al::isNerve(this, &NrvHackCap.Hide))
        return false;

    if (al::isNerve(this, &NrvHackCap.ThrowStay) && !_299) {
        const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
        if ((flags & 0xFF0000) == 0 && (flags & 0xFF) != 0)
            return false;
    }

    hide(false);
    return true;
}

namespace {
void updateHackCapPose(al::LiveActor*, const al::LiveActor*, const PlayerJointControlKeeper*);
}

void HackCap::exeSpinAttack() {
    updateHackCapPose(this, mPlayerActor, mPlayerJointControlKeeper);
}

namespace {
void updateHackCapPose(al::LiveActor* cap, const al::LiveActor* player,
                       const PlayerJointControlKeeper* jointControlKeeper) {
    sead::Vector3f front = sead::Vector3f::zero;
    al::calcFrontDir(&front, player);
    sead::Vector3f up = sead::Vector3f::zero;
    jointControlKeeper->calcGroundPoseUp(&up);
    if (al::isParallelDirection(up, front, 0.01f)) {
        al::copyPose(cap, player);
        return;
    }

    sead::Matrix34f poseMtx = sead::Matrix34f::ident;
    al::makeMtxUpFrontPos(&poseMtx, up, front, al::getTrans(player));
    al::updatePoseMtx(cap, &poseMtx);
}
}  // namespace

void HackCap::exeCatch() {
    updateHackCapPose(this, mPlayerActor, mPlayerJointControlKeeper);
    if (al::isFirstStep(this)) {
        _2a0 = false;
        al::showModelIfHide(this);
        mEquipmentHat->kill();
        mLockOnCapEyes->kill();
    }
    if (al::isGreaterStep(this, static_cast<s32>(al::getActionFrame(this))))
        hide(false);
}

inline bool HackCap::isRecentCollideActor(al::HitSensor* sensor) const {
    if (!mRecentCollideActors)
        return false;
    al::LiveActor* actor = al::getSensorHost(sensor);
    for (s32 i = 0; i < mRecentCollideActors.size(); i++) {
        if (mRecentCollideActors(i) == actor)
            return true;
    }
    return false;
}

void HackCap::collideThrowStartArrow(al::HitSensor* sensor, const sead::Vector3f& hitPos,
                                     const sead::Vector3f& normal,
                                     const sead::Vector3f& resetPos) {
    if (!isRecentCollideActor(sensor) && rs::sendMsgCapStartLockOn(sensor, mAttackSensor)) {
        prepareLockOn(sensor);
        return;
    }

    if (al::isWallPolygon(normal, al::getGravity(mPlayerActor)))
        rs::sendMsgCapTouchWall(sensor, mAttackSensor, hitPos, normal);
    rs::sendMsgCapReflectCollide(sensor, mAttackSensor);
    rs::sendMsgCapAttackCollide(sensor, mAttackSensor);
    _2a7 = true;
    al::resetPosition(this, resetPos);
    rs::resetCollision(mPlayerColliderHackCap);
    al::setNerve(this, &NrvHackCap.Return);
}

bool HackCap::tryCollideWallLockOn() {
    al::HitSensor* wallSensor = rs::tryGetCollidedWallSensor(mPlayerColliderHackCap);
    if (!wallSensor)
        return false;

    if (mRecentCollideActors) {
        al::LiveActor* wallActor = al::getSensorHost(wallSensor);
        for (s32 i = 0; i < mRecentCollideActors.size(); i++) {
            if (mRecentCollideActors(i) == wallActor)
                return false;
        }
    }

    if (!rs::sendMsgCapStartLockOn(wallSensor, mAttackSensor))
        return false;
    prepareLockOn(wallSensor);
    return true;
}

f32 HackCap::getThrowRange() const {
    if (!_299) {
        const u32 flags = mPlayerSeparateCapFlag->getRawFlags();
        if ((flags & 0xFF0000) == 0 && (flags & 0xFF) != 0)
            return sSeparateThrowRange[_29a];
    }

    if (_29f)
        return 1200.0f;
    if (_4d0)
        return mHackCapThrowParam->getTornadoDist();

    if (al::isNearZero(_268.x, 0.001f) && !al::isNearZero(_268.y, 0.001f))
        return mHackCapThrowParam->getRollDist(_268.y > 0.0f);
    const bool isNormal = !_29a;
    return mHackCapThrowParam->getMaxDistNormal(isNormal);
}


void HackCap::activateInvincibleEffect() {
    if (al::isDead(mEquipmentHat))
        return;

    al::tryDeleteEffect(mEquipmentHat, "InvincibleStart");
    al::tryDeleteEffect(mEquipmentHat, "Invincible");

    if (mCapTargetInfo1) {
        const f32 rate = al::calcRate01(mCapTargetInfo1->getLockOnScale(), 1.0f, 2.25f);
        const f32 scaleValue = al::lerpValue(1.0f, 2.25f, rate);
        al::setEffectAllScale(mEquipmentHat, "InvincibleStart",
                              sead::Vector3f(scaleValue, scaleValue, scaleValue));
        al::setEffectAllScale(mEquipmentHat, "Invincible",
                              sead::Vector3f(scaleValue, scaleValue, scaleValue));
        if (mCapTargetInfo1->isDisableInvincibleEffect())
            return;
    }

    al::emitEffect(mEquipmentHat, "InvincibleStart", nullptr);
    al::emitEffect(mEquipmentHat, "Invincible", nullptr);
}

void HackCap::syncInvincibleEffect(bool isActive) {
    if (al::isDead(mEquipmentHat))
        return;

    if (mCapTargetInfo1) {
        const f32 rate = al::calcRate01(mCapTargetInfo1->getLockOnScale(), 1.0f, 2.25f);
        const f32 scaleValue = al::lerpValue(1.0f, 2.25f, rate);
        al::IUseEffectKeeper* effectKeeper = mEquipmentHat;
        const sead::Vector3f scale(scaleValue, scaleValue, scaleValue);
        al::setEffectAllScale(effectKeeper, "Invincible", scale);
        if (mCapTargetInfo1->isDisableInvincibleEffect())
            isActive = false;
    }

    al::IUseEffectKeeper* effectKeeper = mEquipmentHat;
    if (isActive)
        al::tryEmitEffect(effectKeeper, "Invincible", nullptr);
    else
        al::tryDeleteEffect(effectKeeper, "Invincible");
}

void HackCap::emitHackStartEffect() {
    al::startHitReaction(mEquipmentHat, "キャプチャー開始デモ演出エフェクト");
    const f32 rate = al::calcRate01(mCapTargetInfo1->getLockOnScale(), 1.3f, 1.5f);
    const f32 scaleValue = al::lerpValue(1.0f, 1.5f, rate);
    al::LiveActor* equipmentHat = mEquipmentHat;
    const sead::Vector3f scale(scaleValue, scaleValue, scaleValue);
    al::setEffectAllScale(equipmentHat, "MarioHackExpressionShot", scale);
}
