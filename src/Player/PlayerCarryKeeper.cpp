#include "Player/PlayerCarryKeeper.h"

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
class PlayerCarryKeeperNrvWait : public al::Nerve {
public:
    void execute(al::NerveKeeper*) const override {}
};
NERVE_IMPL(PlayerCarryKeeper, Start)
NERVE_IMPL_(PlayerCarryKeeper, ThrowSwing, Throw)
NERVE_IMPL(PlayerCarryKeeper, Throw)
NERVE_IMPL(PlayerCarryKeeper, Release)
NERVE_IMPL(PlayerCarryKeeper, Carry)
PlayerCarryKeeperNrvCarry Carry;

struct {
    PlayerCarryKeeperNrvWait Wait;
    PlayerCarryKeeperNrvStart Start;
    PlayerCarryKeeperNrvThrowSwing ThrowSwing;
    PlayerCarryKeeperNrvThrow Throw;
    PlayerCarryKeeperNrvRelease Release;
} NrvPlayerCarryKeeper;
}  // namespace

PlayerCarryKeeper::PlayerCarryKeeper(const al::LiveActor* player, al::HitSensor* carrySensor,
                                     PlayerAnimator* animator,
                                     const IPlayerModelChanger* modelChanger,
                                     const IUsePlayerCeilingCheck* ceilingCheck,
                                     PlayerJointParamHandLegAngle* handLegAngleParam)
    : al::NerveExecutor("持ち運び管理"), mPlayer(const_cast<al::LiveActor*>(player)),
      mAnimator(animator), mCarrySensor(carrySensor), mHeldSensor(nullptr),
      mModelChanger(const_cast<IPlayerModelChanger*>(modelChanger)),
      mCeilingCheck(const_cast<IUsePlayerCeilingCheck*>(ceilingCheck)),
      mHandLegAngleParam(handLegAngleParam), mCarryDelay(0), carryAboveFlag(false),
      carryWallKeepFlag(false), mHandJointAngle(0.0f, 0.0f, 0.0f), mCollideLockCounter(0),
      mCarryActorPos(0.0f, 0.0f, 0.0f) {
    initNerve(&NrvPlayerCarryKeeper.Wait, 0);
}

void PlayerCarryKeeper::update() {
    mCarryDelay = al::converge(mCarryDelay, 0, 1);
    updateNerve();
}

// NON_MATCHING: exact 0x194 size and recovered evaluation order; target/current differ only in the
// placement of the shared reset/getTrans block. An explicit reset-predicate branch inversion grew the
// body to 0x1B0 without changing the linear call-order swap and was reverted; remaining blocker is
// compiler block placement/lifetime in a validator-clean structured CFG.
bool PlayerCarryKeeper::updateCollideLockUp(const IUsePlayerCollision* collider,
                                            const PlayerPushReceiver* pushReceiver) {
    al::LiveActor* player = mPlayer;
    sead::Vector3f collidePush;
    const sead::Vector3f* gravity = nullptr;
    const sead::Vector3f* velocity = nullptr;
    f32 gravityPushDot = 0.0f;

    if (al::isNerve(this, &NrvPlayerCarryKeeper.Release) || !mHeldSensor ||
        rs::isCollidedGround(collider) ||
        (collidePush.set(0.0f, 0.0f, 0.0f), pushReceiver->calcOnlyCollidePushVec(&collidePush),
         al::isNearZero(collidePush, 0.001f)) ||
        (al::getTrans(player) - mCarryActorPos).squaredLength() > 900.0f ||
        (gravity = &al::getGravity(player), gravityPushDot = gravity->dot(collidePush),
         velocity = &al::getVelocity(player), gravityPushDot > 0.0f) ||
        gravity->dot(*velocity) < 0.0f) {
        mCollideLockCounter = 0;
        mCarryActorPos = al::getTrans(mPlayer);
        return false;
    }

    mCollideLockCounter = al::converge(mCollideLockCounter, 10, 1);
    return mCollideLockCounter > 9;
}

bool PlayerCarryKeeper::startCarry(al::HitSensor* heldSensor) {
    if (mCarryDelay > 0)
        return false;

    if (al::sendMsgPlayerCarryFrontWallKeep(heldSensor, mCarrySensor)) {
        mHeldSensor = heldSensor;
        bool result = true;
        carryAboveFlag = false;
        carryWallKeepFlag = result;
        mCollideLockCounter = 0;
        const sead::Vector3f& trans = al::getTrans(mPlayer);
        mCarryActorPos = trans;
        al::setNerve(this, &NrvPlayerCarryKeeper.Start);
        return result;
    }

    if (al::sendMsgPlayerCarryFront(heldSensor, mCarrySensor)) {
        al::LiveActor* player = mPlayer;
        mHeldSensor = heldSensor;
        mCarryFlags = 0;
        mCollideLockCounter = 0;
        const sead::Vector3f& trans = al::getTrans(player);
        mCarryActorPos = trans;
        al::setNerve(this, &NrvPlayerCarryKeeper.Start);
        return true;
    }

    if (!mCeilingCheck->isEnableHoldUp() || !al::sendMsgPlayerCarryUp(heldSensor, mCarrySensor))
        return false;

    mHeldSensor = heldSensor;
    bool result = true;
    carryAboveFlag = result;
    carryWallKeepFlag = false;
    mCollideLockCounter = 0;
    const sead::Vector3f& trans = al::getTrans(mPlayer);
    mCarryActorPos = trans;
    al::setNerve(this, &NrvPlayerCarryKeeper.Start);
    return result;
}

bool PlayerCarryKeeper::startThrow(bool swing) {
    if (al::isNerve(this, &NrvPlayerCarryKeeper.Start) && al::isNewNerve(this))
        return false;

    mCarryDelay = 10;
    if (swing)
        al::setNerve(this, &NrvPlayerCarryKeeper.ThrowSwing);
    else
        al::setNerve(this, &NrvPlayerCarryKeeper.Throw);
    return true;
}

void PlayerCarryKeeper::startCancelAndRelease() {
    al::sendMsgHoldCancel(mHeldSensor, mCarrySensor);
    PlayerJointParamHandLegAngle* handLegAngleParam = mHandLegAngleParam;
    handLegAngleParam->handAngle.set(0.0f, 0.0f, 0.0f);
    handLegAngleParam->legAngle.set(0.0f, 0.0f, 0.0f);
    handLegAngleParam->blendRate = 1.0f;
    mHandJointAngle.set(0.0f, 0.0f, 0.0f);
    al::setNerve(this, &NrvPlayerCarryKeeper.Release);
}

void PlayerCarryKeeper::startRelease() {
    PlayerJointParamHandLegAngle* handLegAngleParam = mHandLegAngleParam;
    handLegAngleParam->handAngle.set(0.0f, 0.0f, 0.0f);
    handLegAngleParam->legAngle.set(0.0f, 0.0f, 0.0f);
    handLegAngleParam->blendRate = 1.0f;
    mHandJointAngle.set(0.0f, 0.0f, 0.0f);
    al::setNerve(this, &NrvPlayerCarryKeeper.Release);
}

void PlayerCarryKeeper::startReleaseDemo() {
    al::sendMsgPlayerReleaseDemo(mHeldSensor, mCarrySensor);
    PlayerJointParamHandLegAngle* handLegAngleParam = mHandLegAngleParam;
    handLegAngleParam->handAngle.set(0.0f, 0.0f, 0.0f);
    handLegAngleParam->legAngle.set(0.0f, 0.0f, 0.0f);
    handLegAngleParam->blendRate = 1.0f;
    mHandJointAngle.set(0.0f, 0.0f, 0.0f);
    al::setNerve(this, &NrvPlayerCarryKeeper.Release);
}

void PlayerCarryKeeper::startReleaseDamage() {
    al::sendMsgPlayerReleaseDamage(mHeldSensor, mCarrySensor);
    PlayerJointParamHandLegAngle* handLegAngleParam = mHandLegAngleParam;
    handLegAngleParam->handAngle.set(0.0f, 0.0f, 0.0f);
    handLegAngleParam->legAngle.set(0.0f, 0.0f, 0.0f);
    handLegAngleParam->blendRate = 1.0f;
    mHandJointAngle.set(0.0f, 0.0f, 0.0f);
    al::setNerve(this, &NrvPlayerCarryKeeper.Release);
}

void PlayerCarryKeeper::startReleaseDead() {
    al::sendMsgPlayerReleaseDead(mHeldSensor, mCarrySensor);
    PlayerJointParamHandLegAngle* handLegAngleParam = mHandLegAngleParam;
    handLegAngleParam->handAngle.set(0.0f, 0.0f, 0.0f);
    handLegAngleParam->legAngle.set(0.0f, 0.0f, 0.0f);
    handLegAngleParam->blendRate = 1.0f;
    mHandJointAngle.set(0.0f, 0.0f, 0.0f);
    al::setNerve(this, &NrvPlayerCarryKeeper.Release);
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
    return carryAboveFlag ? "CarryUpStart" : "CarryFrontStart";
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
    return !al::isNerve(this, &NrvPlayerCarryKeeper.Release) && mHeldSensor;
}

bool PlayerCarryKeeper::isCarryWallKeep() const {
    return al::isNerve(this, &NrvPlayerCarryKeeper.Release) || !mHeldSensor || carryWallKeepFlag;
}

bool PlayerCarryKeeper::isCarryFront() const {
    return !al::isNerve(this, &NrvPlayerCarryKeeper.Release) && mHeldSensor && !carryAboveFlag;
}

bool PlayerCarryKeeper::isCarryUp() const {
    return !al::isNerve(this, &NrvPlayerCarryKeeper.Release) && mHeldSensor && carryAboveFlag;
}

bool PlayerCarryKeeper::isThrow() const {
    return al::isNerve(this, &NrvPlayerCarryKeeper.Throw) ||
           al::isNerve(this, &NrvPlayerCarryKeeper.ThrowSwing);
}

bool PlayerCarryKeeper::isThrowHandR() const {
    return isCarryFront() && isThrow() && mAnimator->isSubAnimPlaying() &&
           mAnimator->isSubAnim(getCarryThrowAnimName());
}

const char* PlayerCarryKeeper::getCarryThrowAnimName() const {
    return carryAboveFlag ? "CarryUpThrow" : "CarryFrontThrow";
}

bool PlayerCarryKeeper::isThrowHold() const {
    return isThrow() && al::isLessEqualStep(this, getThrowReleaseFrame());
}

s32 PlayerCarryKeeper::getThrowReleaseFrame() const {
    return carryAboveFlag ? 4 : 5;
}

bool PlayerCarryKeeper::isThrowRelease() const {
    return isThrow() && al::isGreaterEqualStep(this, getThrowReleaseFrame());
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

void PlayerCarryKeeper::updateHandJointAngle() {
    mHandLegAngleParam->handAngle = mHandJointAngle;
    mHandLegAngleParam->blendRate = 0.5f;
}

void PlayerCarryKeeper::exeCarry() {
    if (mModelChanger->is2DModel())
        return;

    if (al::isFirstStep(this))
        mAnimator->startUpperBodyAnim(getCarryAnimName());
    updateHandJointAngle();
}

const char* PlayerCarryKeeper::getCarryAnimName() const {
    return carryAboveFlag ? "CarryUp" : "CarryFront";
}

void PlayerCarryKeeper::exeThrow() {
    if (mModelChanger->is2DModel()) {
        if (mHeldSensor) {
            al::sendMsgPlayerRelease(mHeldSensor, mCarrySensor);
            mHeldSensor = nullptr;
        }
        al::setNerve(this, &NrvPlayerCarryKeeper.Wait);
        return;
    }

    if (al::isFirstStep(this)) {
        mAnimator->clearUpperBodyAnim();
        mAnimator->startSubAnim(getCarryThrowAnimName());
        PlayerJointParamHandLegAngle* handLegAngleParam = mHandLegAngleParam;
        handLegAngleParam->handAngle.set(0.0f, 0.0f, 0.0f);
        handLegAngleParam->legAngle.set(0.0f, 0.0f, 0.0f);
        handLegAngleParam->blendRate = 1.0f;
    }

    if (mHeldSensor && al::isGreaterEqualStep(this, getThrowReleaseFrame())) {
        al::HitSensor* carrySensor = mCarrySensor;
        al::HitSensor* heldSensor = mHeldSensor;
        if (!al::isNerve(this, &NrvPlayerCarryKeeper.ThrowSwing) ||
            !al::sendMsgPlayerReleaseBySwing(heldSensor, carrySensor)) {
            al::sendMsgPlayerRelease(heldSensor, carrySensor);
        }
        mHeldSensor = nullptr;
    }

    if (!mAnimator->isSubAnimPlaying() || !mAnimator->isSubAnim(getCarryThrowAnimName()) ||
        mAnimator->isSubAnimEnd()) {
        if (!mHeldSensor) {
            carryAboveFlag = false;
            carryWallKeepFlag = false;
            al::setNerve(this, &NrvPlayerCarryKeeper.Wait);
        }
    }
}

void PlayerCarryKeeper::exeRelease() {
    const bool is2D = mModelChanger->is2DModel();
    mHeldSensor = nullptr;
    if (is2D) {
        al::setNerve(this, &NrvPlayerCarryKeeper.Wait);
        return;
    }

    mAnimator->clearUpperBodyAnim();
    mCarryDelay = 10;
    carryAboveFlag = false;
    carryWallKeepFlag = false;
    al::setNerve(this, &NrvPlayerCarryKeeper.Wait);
}
