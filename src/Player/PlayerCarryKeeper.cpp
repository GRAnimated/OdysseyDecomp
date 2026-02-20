#include "Player/PlayerCarryKeeper.h"

#include <math/seadVector.h>

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/IPlayerModelChanger.h"
#include "Player/IUsePlayerCeilingCheck.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerPushReceiver.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(PlayerCarryKeeper, Wait);
NERVE_IMPL(PlayerCarryKeeper, Start);
NERVE_IMPL_(PlayerCarryKeeper, ThrowSwing, Throw);
NERVE_IMPL(PlayerCarryKeeper, Throw);
NERVE_IMPL(PlayerCarryKeeper, Release);
NERVE_IMPL(PlayerCarryKeeper, Carry);

NERVES_MAKE_NOSTRUCT(PlayerCarryKeeper, Carry);
NERVES_MAKE_STRUCT(PlayerCarryKeeper, Wait, Start, ThrowSwing, Throw, Release);
}  // namespace

// Observations:
//
// - Claude missed multiple inlines, most notably startRelease()
//   in 4 functions and isThrow() in 3 functions
//
// - Claude placed the Carry nerve inside the struct at the end,
//   causing a mismatch in PlayerCarryKeeper::exeStart
//
// - It chose to not create a struct for PlayerJointParamHandLegAngle,
//   instead only interpreting it as a float array

PlayerCarryKeeper::PlayerCarryKeeper(const al::LiveActor* player, al::HitSensor* carrySensor,
                                     PlayerAnimator* animator,
                                     const IPlayerModelChanger* modelChanger,
                                     const IUsePlayerCeilingCheck* ceilingCheck,
                                     PlayerJointParamHandLegAngle* handLegAngleParam)
    : al::NerveExecutor("PlayerCarryKeeper") {
    mPlayer = player;
    mAnimator = animator;
    mCarrySensor = carrySensor;
    mHeldSensor = nullptr;
    mModelChanger = modelChanger;
    mCeilingCheck = ceilingCheck;
    mHandLegAngleParam = handLegAngleParam;
    _48 = 0;
    mIsCarryAbove = false;
    mIsCarryWallKeep = false;
    _50 = {0.0f, 0.0f, 0.0f};
    _5c = 0;
    _60 = {0.0f, 0.0f, 0.0f};
    initNerve(&NrvPlayerCarryKeeper.Wait, 0);
}

void PlayerCarryKeeper::update() {
    _48 = al::converge(_48, 0, 1);
    al::NerveExecutor::updateNerve();
}

// NON_MATCHING: https://decomp.me/scratch/AigkT
bool PlayerCarryKeeper::updateCollideLockUp(const IUsePlayerCollision* collider,
                                            const PlayerPushReceiver* pushReceiver) {
    if (al::isNerve(this, &NrvPlayerCarryKeeper.Release) || !mHeldSensor ||
        rs::isCollidedGround(collider)) {
        _5c = 0;
        _60 = al::getTrans(mPlayer);
        return false;
    }

    sead::Vector3f push_vec = sead::Vector3f::zero;
    pushReceiver->calcOnlyCollidePushVec(&push_vec);

    const sead::Vector3f& gravity = al::getGravity(mPlayer);

    if (al::isNearZero(push_vec, 0.001f) ||
        (al::getTrans(mPlayer) - _60).squaredLength() > 900.0f || gravity.dot(push_vec) > 0.0f ||
        gravity.dot(al::getVelocity(mPlayer)) < 0.0f) {
        _5c = 0;
        _60 = al::getTrans(mPlayer);
        return false;
    }

    _5c = al::converge(_5c, 10, 1);
    return _5c > 9;
}

void PlayerCarryKeeper::updateHandJointAngle() {
    auto* param = reinterpret_cast<float*>(mHandLegAngleParam);
    param[0] = _50.x;
    param[1] = _50.y;
    param[2] = _50.z;
    reinterpret_cast<float*>(mHandLegAngleParam)[6] = 0.5f;
}

bool PlayerCarryKeeper::startCarry(al::HitSensor* heldSensor) {
    if (_48 > 0)
        return false;

    if (al::sendMsgPlayerCarryFrontWallKeep(heldSensor, mCarrySensor)) {
        mHeldSensor = heldSensor;
        mIsCarryAbove = false;
        mIsCarryWallKeep = true;
        _5c = 0;
        _60 = al::getTrans(mPlayer);
        al::setNerve(this, &NrvPlayerCarryKeeper.Start);
        return true;
    }

    if (al::sendMsgPlayerCarryFront(heldSensor, mCarrySensor)) {
        mHeldSensor = heldSensor;
        mIsCarryAbove = false;
        mIsCarryWallKeep = false;
        _5c = 0;
        _60 = al::getTrans(mPlayer);
        al::setNerve(this, &NrvPlayerCarryKeeper.Start);
        return true;
    }

    if (mCeilingCheck->isEnableHoldUp() && al::sendMsgPlayerCarryUp(heldSensor, mCarrySensor)) {
        mHeldSensor = heldSensor;
        mIsCarryAbove = true;
        mIsCarryWallKeep = false;
        _5c = 0;
        _60 = al::getTrans(mPlayer);
        al::setNerve(this, &NrvPlayerCarryKeeper.Start);
        return true;
    }

    return false;
}

bool PlayerCarryKeeper::startThrow(bool swing) {
    if (al::isNerve(this, &NrvPlayerCarryKeeper.Start) && al::isNewNerve(this))
        return false;

    _48 = 10;
    if (swing)
        al::setNerve(this, &NrvPlayerCarryKeeper.ThrowSwing);
    else
        al::setNerve(this, &NrvPlayerCarryKeeper.Throw);
    return true;
}

void PlayerCarryKeeper::startCancelAndRelease() {
    al::sendMsgHoldCancel(mHeldSensor, mCarrySensor);
    startRelease();
}

void PlayerCarryKeeper::startRelease() {
    auto* param = reinterpret_cast<float*>(mHandLegAngleParam);
    param[0] = 0.0f;
    param[1] = 0.0f;
    param[2] = 0.0f;
    param[3] = 0.0f;
    param[4] = 0.0f;
    param[5] = 0.0f;
    param[6] = 1.0f;
    _50 = {0.0f, 0.0f, 0.0f};
    al::setNerve(this, &NrvPlayerCarryKeeper.Release);
}

void PlayerCarryKeeper::startReleaseDemo() {
    al::sendMsgPlayerReleaseDemo(mHeldSensor, mCarrySensor);
    startRelease();
}

void PlayerCarryKeeper::startReleaseDamage() {
    al::sendMsgPlayerReleaseDamage(mHeldSensor, mCarrySensor);
    startRelease();
}

void PlayerCarryKeeper::startReleaseDead() {
    al::sendMsgPlayerReleaseDead(mHeldSensor, mCarrySensor);
    startRelease();
}

void PlayerCarryKeeper::startDemoKeepCarry() {
    al::sendMsgPlayerCarryKeepDemo(mHeldSensor, mCarrySensor);
}

void PlayerCarryKeeper::startDemoShineGet() {
    if (rs::sendMsgPlayerCarryShineGetStart(mHeldSensor, mCarrySensor))
        mAnimator->clearUpperBodyAnim();
}

void PlayerCarryKeeper::endDemoShineGet() {
    if (rs::sendMsgPlayerCarryShineGetEnd(mHeldSensor, mCarrySensor))
        mAnimator->startUpperBodyAnim(getCarryStartAnimName());
}

const char* PlayerCarryKeeper::getCarryStartAnimName() const {
    return mIsCarryAbove ? "CarryUpStart" : "CarryFrontStart";
}

void PlayerCarryKeeper::startCameraSubjective() {
    rs::sendMsgPlayerCarryCameraSubjectiveStart(mHeldSensor, mCarrySensor);
}

void PlayerCarryKeeper::endCameraSubjective() {
    rs::sendMsgPlayerCarryCameraSubjectiveEnd(mHeldSensor, mCarrySensor);
}

void PlayerCarryKeeper::sendPlayerWarp() {
    al::sendMsgPlayerCarryWarp(mHeldSensor, mCarrySensor);
}

bool PlayerCarryKeeper::isCarry() const {
    return !al::isNerve(this, &NrvPlayerCarryKeeper.Release) && mHeldSensor != nullptr;
}

bool PlayerCarryKeeper::isCarryWallKeep() const {
    return al::isNerve(this, &NrvPlayerCarryKeeper.Release) || !mHeldSensor || mIsCarryWallKeep;
}

bool PlayerCarryKeeper::isCarryFront() const {
    return !al::isNerve(this, &NrvPlayerCarryKeeper.Release) && mHeldSensor != nullptr &&
           !mIsCarryAbove;
}

bool PlayerCarryKeeper::isCarryUp() const {
    return !al::isNerve(this, &NrvPlayerCarryKeeper.Release) && mHeldSensor != nullptr &&
           mIsCarryAbove;
}

bool PlayerCarryKeeper::isThrow() const {
    return al::isNerve(this, &NrvPlayerCarryKeeper.Throw) ||
           al::isNerve(this, &NrvPlayerCarryKeeper.ThrowSwing);
}

bool PlayerCarryKeeper::isThrowHandR() const {
    if (!al::isNerve(this, &NrvPlayerCarryKeeper.Release) && mHeldSensor != nullptr &&
        !mIsCarryAbove && isThrow() && mAnimator->isSubAnimPlaying()) {
        return mAnimator->isSubAnim(getCarryThrowAnimName());
    }
    return false;
}

const char* PlayerCarryKeeper::getCarryThrowAnimName() const {
    return mIsCarryAbove ? "CarryUpThrow" : "CarryFrontThrow";
}

bool PlayerCarryKeeper::isThrowHold() const {
    if (!isThrow())
        return false;
    return al::isLessEqualStep(this, getThrowReleaseFrame());
}

s32 PlayerCarryKeeper::getThrowReleaseFrame() const {
    return mIsCarryAbove ? 4 : 5;
}

bool PlayerCarryKeeper::isThrowRelease() const {
    if (!isThrow())
        return false;
    return al::isGreaterEqualStep(this, getThrowReleaseFrame());
}

void PlayerCarryKeeper::exeWait() {}

void PlayerCarryKeeper::exeStart() {
    if (mModelChanger->is2DModel()) {
        al::setNerve(this, &Carry);
        return;
    }

    if (al::isFirstStep(this))
        mAnimator->startUpperBodyAnim(getCarryStartAnimName());

    updateHandJointAngle();

    if (mAnimator->isUpperBodyAnimEnd())
        al::setNerve(this, &Carry);
}

void PlayerCarryKeeper::exeCarry() {
    if (mModelChanger->is2DModel())
        return;

    if (al::isFirstStep(this))
        mAnimator->startUpperBodyAnim(getCarryAnimName());

    updateHandJointAngle();
}

const char* PlayerCarryKeeper::getCarryAnimName() const {
    return mIsCarryAbove ? "CarryUp" : "CarryFront";
}

// NON_MATCHING: https://decomp.me/scratch/6WYB7
void PlayerCarryKeeper::exeThrow() {
    if (mModelChanger->is2DModel()) {
        if (mHeldSensor != nullptr) {
            al::sendMsgPlayerRelease(mHeldSensor, mCarrySensor);
            mHeldSensor = nullptr;
        }
        al::setNerve(this, &NrvPlayerCarryKeeper.Wait);
        return;
    }

    if (al::isFirstStep(this)) {
        mAnimator->clearUpperBodyAnim();
        mAnimator->startSubAnim(getCarryThrowAnimName());
        auto* param = reinterpret_cast<float*>(mHandLegAngleParam);
        param[0] = 0.0f;
        param[1] = 0.0f;
        param[2] = 0.0f;
        param[3] = 0.0f;
        param[4] = 0.0f;
        param[5] = 0.0f;
        param[6] = 1.0f;
    }

    if (mHeldSensor != nullptr && al::isGreaterEqualStep(this, getThrowReleaseFrame())) {
        if (al::isNerve(this, &NrvPlayerCarryKeeper.ThrowSwing)) {
            if (!al::sendMsgPlayerReleaseBySwing(mHeldSensor, mCarrySensor))
                al::sendMsgPlayerRelease(mHeldSensor, mCarrySensor);
        } else {
            al::sendMsgPlayerRelease(mHeldSensor, mCarrySensor);
        }
        mHeldSensor = nullptr;
    }

    if (!mAnimator->isSubAnimPlaying() || !mAnimator->isSubAnim(getCarryThrowAnimName()) ||
        mAnimator->isSubAnimEnd()) {
        if (mHeldSensor == nullptr) {
            mIsCarryAbove = false;
            mIsCarryWallKeep = false;
            al::setNerve(this, &NrvPlayerCarryKeeper.Wait);
        }
    }
}

void PlayerCarryKeeper::exeRelease() {
    bool is2d = mModelChanger->is2DModel();
    mHeldSensor = nullptr;
    if (is2d) {
        al::setNerve(this, &NrvPlayerCarryKeeper.Wait);
        return;
    }
    mAnimator->clearUpperBodyAnim();
    _48 = 10;
    mIsCarryAbove = false;
    mIsCarryWallKeep = false;
    al::setNerve(this, &NrvPlayerCarryKeeper.Wait);
}
