#include "Player/YoshiStateHack.h"

#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/Shadow/ActorShadowUtil.h"

#include "Enemy/HackerDepthShadowMapCtrl.h"
#include "Player/IUsePlayerCollision.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerHackStartShaderCtrl.h"
#include "Player/PlayerModelChangerYoshi.h"
#include "Player/Yoshi.h"
#include "Player/YoshiStateHackPlay.h"
#include "Util/DemoUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackFunction.h"
#include "Util/SensorMsgFunction.h"
#include "Util/YoshiUtil.h"

namespace {
NERVE_IMPL(YoshiStateHack, LockOn);
NERVE_IMPL(YoshiStateHack, Hack);
NERVE_IMPL(YoshiStateHack, Demo);
NERVE_IMPL(YoshiStateHack, DemoHackStart);
NERVES_MAKE_NOSTRUCT(YoshiStateHack, DemoHackStart);

struct {
    YoshiStateHackNrvLockOn LockOn;
    YoshiStateHackNrvHack Hack;
    YoshiStateHackNrvDemo Demo;
    PlayerHackStartShaderParam shaderParam;
} NrvYoshiStateHack = {{}, {}, {}, {false, 200.0f, 10, 20}};
}  // namespace

YoshiStateHack::YoshiStateHack(const al::ActorInitInfo& info, al::LiveActor* actor,
                               al::LiveActor* modelActor, const CapTargetInfo* capTargetInfo,
                               const PlayerConst* playerConst,
                               const al::WaterSurfaceFinder* waterSurfaceFinder,
                               PlayerTrigger* trigger, PlayerModelChangerYoshi* modelChanger,
                               PlayerAnimator* animator, IUsePlayerCollision* collision,
                               PlayerEffect* effect, PlayerJumpMessageRequest* jumpMessageRequest)
    : al::ActorStateBase("Hack", actor), mModelActor(modelActor), mPlayerConst(playerConst),
      mWaterSurfaceFinder(waterSurfaceFinder), mCollision(collision),
      mCapTargetInfo(capTargetInfo), mModelChanger(modelChanger), mAnimator(animator) {
    initNerve(&NrvYoshiStateHack.LockOn, 1);
    mStateHackPlay = new YoshiStateHackPlay(info, actor, modelActor, &mPlayerHack, playerConst,
                                            trigger, animator, collision, effect,
                                            jumpMessageRequest);
    al::initNerveState(this, mStateHackPlay, &NrvYoshiStateHack.Hack, "操作中");
    mHackStartShaderCtrl = new PlayerHackStartShaderCtrl(modelActor, &NrvYoshiStateHack.shaderParam);
    mDepthShadowMapCtrl =
        new HackerDepthShadowMapCtrl(modelActor, "DepthGround", 50.0f, 0.3f, 0.5f);
    al::startMtpAnim(modelActor, "Normal");
    al::startVisAnim(modelActor, "HackOff");
}

void YoshiStateHack::appear() {
    al::ActorStateBase::appear();
    al::setNerve(this, &NrvYoshiStateHack.LockOn);
}

void YoshiStateHack::updatePrevMovement() {
    if (!isDead())
        mStateHackPlay->updatePrevMovement();
}

void YoshiStateHack::updateAfterMovement() {
    if (!isDead()) {
        mStateHackPlay->updateAfterMovement();
        if (mPlayerHack)
            mDepthShadowMapCtrl->update(mCollision->getPlayerCollider());
    }
}

bool YoshiStateHack::isEnableUpdateCollider() const {
    if (mPlayerHack)
        return !rs::isActiveHackStartDemo(mPlayerHack);
    return true;
}

bool YoshiStateHack::isActiveHeadCorrection() const {
    if (isDead())
        return false;
    return mStateHackPlay->isActiveHeadCorrection();
}

bool YoshiStateHack::isCollisionShapeTongueJump() const {
    if (isDead())
        return false;
    return mStateHackPlay->isCollisionShapeTongueJump();
}

bool YoshiStateHack::tryGetLookAtTonguePos(sead::Vector3f* position) const {
    if (isDead())
        return false;
    return mStateHackPlay->tryGetLookAtTonguePos(position);
}

bool YoshiStateHack::tryCalcTonguePullForce(f32* force, sead::Vector3f* direction) const {
    if (isDead())
        return false;
    return mStateHackPlay->tryCalcTonguePullForce(force, direction);
}

void YoshiStateHack::calcGroundPoseRate(f32* frontRate, f32* sideRate) const {
    mStateHackPlay->calcGroundPoseRate(frontRate, sideRate);
}

void YoshiStateHack::startFruitShineGetDemo() {
    mStateHackPlay->startFruitShineGetDemo();
}

void YoshiStateHack::exeLockOn() {}

void YoshiStateHack::exeDemo() {
    if (!rs::isHackStartDemoEnterMario(mPlayerHack))
        return;

    mAnimator->startAnim("HackStart");
    auto* modelActor = mModelActor;
    const char* animName = "HackOn";
    al::startMtpAnim(modelActor, animName);
    al::startVisAnim(modelActor, animName);
    al::offDepthShadowModel(mModelActor);
    al::validateDepthShadowMap(mModelActor);
    mHackStartShaderCtrl->start();
    al::setNerve(this, &DemoHackStart);
}

void YoshiStateHack::exeDemoHackStart() {
    mHackStartShaderCtrl->update();
    if (!mAnimator->isAnimEnd())
        return;

    mHackStartShaderCtrl->end();
    rs::endHackStartDemo(mPlayerHack, mModelActor);
    rs::noticeCurrentHackYoshi(static_cast<Yoshi*>(mActor));
    al::setNerve(this, &NrvYoshiStateHack.Hack);
}

void YoshiStateHack::exeHack() {
    al::updateNerveState(this);
}

// NON_MATCHING: target/current are both 96 bytes, but current tail-calls YoshiStateHackPlay::attackSensor while target calls it, tests bit 0, and normalizes the bool result; wider-integer return experiments collapse to an AND instead, so next hypothesis is the original wrapper/control-flow source shape.
bool YoshiStateHack::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (!al::isNerve(this, &NrvYoshiStateHack.Hack))
        return false;
    if (!mStateHackPlay->attackSensor(self, other))
        return false;
    return true;
}

// NON_MATCHING: target is 468 bytes; current natural bool-wrapper form is expected to remain 476 bytes because the final YoshiStateHackPlay::receiveMsg call tail-calls after the epilogue instead of target BL/TBZ/normalization; wider-return declaration experiments produced a 464-byte AND form, so next hypothesis is a natural wrapper/lifetime form that preserves post-call normalization.
bool YoshiStateHack::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                                al::HitSensor* self) {
    if (!rs::isSensorTypeYoshiMsgReceivable(self))
        return false;

    if (rs::tryReceiveMsgInitCapTargetAndSetCapTargetInfo(message, mCapTargetInfo))
        return true;

    if (rs::isMsgHackSyncDamageVisibility(message)) {
        rs::syncDamageVisibility(mModelActor, mPlayerHack);
        return true;
    }

    if (rs::isMsgCapKeepLockOn(message))
        return true;

    if (rs::isMsgEnableMapCheckPointWarp(message))
        return rs::isCollidedGround(mCollision);

    if (mPlayerHack && rs::isMsgHackMarioInWater(message))
        return rs::isInPuddleHeight(mWaterSurfaceFinder, mPlayerConst);

    if (al::isNerve(this, &NrvYoshiStateHack.LockOn) && rs::isMsgStartHack(message)) {
        al::invalidateClipping(mActor);
        al::invalidateClipping(mModelActor);
        mModelChanger->showSilhouette();
        mPlayerHack = rs::startHack(self, other, mModelActor);
        rs::startHackStartDemo(mPlayerHack, mModelActor);
        rs::addDemoActor(mActor, false);
        mAnimator->startAnim("Wait");
        al::setNerve(this, &NrvYoshiStateHack.Demo);
        return true;
    }

    if (!al::isNerve(this, &NrvYoshiStateHack.Hack) || !mPlayerHack)
        return false;
    if (!mStateHackPlay->receiveMsg(message, other, self))
        return false;
    return true;
}

bool YoshiStateHack::receiveMsgSetNerveState(const al::SensorMsg* message, al::HitSensor* other,
                                             al::HitSensor* self) {
    if (!rs::isSensorTypeYoshiMsgReceivable(self))
        return false;
    return rs::isMsgCapStartLockOn(message);
}

// NON_MATCHING: target/current are both 276 bytes with matching branch/call behavior, but the current compiler assigns the message and isReturnEgg callee-saved registers opposite the target; next hypothesis is a natural parameter/lifetime expression that changes x21/x22 allocation without altering semantics.
bool YoshiStateHack::receiveMsgEndState(bool* isReturnEgg, const al::SensorMsg* message,
                                        al::HitSensor* other, al::HitSensor* self) {

    if (!rs::isSensorTypeYoshiMsgReceivable(self))
        return false;

    if (rs::isMsgCapCancelLockOn(message))
        return true;

    auto** playerHack = &mPlayerHack;
    if (!*playerHack)
        return false;

    if (rs::isMsgCancelHack(message) || rs::isMsgHackMarioCheckpointFlagWarp(message)) {
        prepareEndHack();
        rs::endHack(playerHack);
        return true;
    }

    if (rs::isMsgHackMarioDemo(message)) {
        *isReturnEgg = true;
        prepareEndHack();
        rs::endHack(playerHack);
        return true;
    }

    if (!rs::isMsgHackMarioDead(message)) {
        if (rs::isMsgHackMarioInWater(message)) {
            if (rs::isInPuddleHeight(mWaterSurfaceFinder, mPlayerConst))
                return false;

            prepareEndHack();
            rs::endHack(playerHack);
            return true;
        }

        if (!rs::isMsgCancelHackByDokan(message))
            return false;
    }

    *isReturnEgg = true;
    prepareEndHack();
    rs::endHack(playerHack);
    al::startHitReaction(mActor, "[ヨッシー]死亡");
    return true;
}

void YoshiStateHack::prepareEndHack() {
    if (mPlayerHack)
        rs::tryEndHackStartDemo(mPlayerHack, mModelActor);
    al::validateClipping(mModelActor);
    al::onDepthShadowModel(mModelActor);
    al::invalidateDepthShadowMap(mModelActor);
    mModelChanger->hideSilhouette();
    al::showModelIfHide(mModelActor);
    al::setVelocityZero(mActor);
    mStateHackPlay->prepareEndHack();
    auto* modelActor = mModelActor;
    al::startMtpAnim(modelActor, "Normal");
    al::startVisAnim(modelActor, "HackOff");
}

bool YoshiStateHack::reactionCollidedCollisionCode() {
    if (isDead() || !al::isNerve(this, &NrvYoshiStateHack.Hack))
        return false;

    auto** playerHack = &mPlayerHack;
    if (!*playerHack)
        return false;

    if (rs::isTouchHackCancelCollisionCode(mActor, mCollision)) {
        prepareEndHack();
        rs::endHack(playerHack);
        al::startHitReaction(mActor, "[ヨッシー]死亡");
        return true;
    }

    if (rs::isTouchHackDamageCollisionCode(mActor, mCollision) ||
        mStateHackPlay->reactionCollidedCollisionCodeDamage())
        rs::requestDamage(*playerHack);

    return false;
}
