#include "Player/PlayerPuppet.h"

#include <prim/seadMemUtil.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"

#include "Player/IPlayerModelChanger.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerDamageKeeper.h"
#include "Player/PlayerEffect.h"
#include "Util/ActorDimensionKeeper.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/WorldEndBorderKeeper.h"

// NON_MATCHING: target/current are both 164 bytes, but recovery-vector zero stores are scheduled after the flag stores; next source-level hypothesis is recovering the original in-class default/member-initializer arrangement.
PlayerPuppet::PlayerPuppet(al::LiveActor* actor, HackCap* hackCap, PlayerAnimator* playerAnimator,
                           IUsePlayerCollision* playerCollision,
                           ActorDimensionKeeper* actorDimensionKeeper,
                           IPlayerModelChanger* playerModelChanger,
                           WorldEndBorderKeeper* worldEndBorderKeeper,
                           PlayerCounterForceRun* playerCounterForceRun,
                           PlayerDamageKeeper* playerDamageKeeper, PlayerEffect* playerEffect,
                           const PlayerInput* playerInput, const PlayerConst* playerConst)
    : mActor(actor), mHackCap(hackCap), mPlayerAnimator(playerAnimator),
      mIUsePlayerCollision(playerCollision), mActorDimensionKeeper(actorDimensionKeeper),
      mIPlayerModelChanger(playerModelChanger), mWorldEndBorderKeeper(worldEndBorderKeeper),
      mPlayerCounterForceRun(playerCounterForceRun), mPlayerDamageKeeper(playerDamageKeeper),
      mPlayerEffect(playerEffect), mJudgePreInputJump(nullptr), mPlayerInput(playerInput),
      mPlayerConst(playerConst), _70(nullptr), _78(nullptr), _80(false), _81(false), _82(false),
      _84(0.0f, 0.0f, 0.0f), _90(0.0f, 0.0f, 0.0f), mAreaObj(nullptr),
      mIsBindEndOnGround(false), mIsBindEndJump(false), mIsBindEndCapThrow(false),
      mIsValidCollisionCheck(true), _ac(false), mIsRequestDamage(false), _ae(false),
      mIsSensorValid(false), _b0(false), _b1(false), _b2(false), _b3(false), _b4(false),
      _b5(false), _b6(false), mLookAtTargetPosition(0.0f, 0.0f, 0.0f),
      mBindEndJumpInfo(new PlayerBindEndJumpInfo()) {}

void PlayerPuppet::start(al::HitSensor* sender, al::HitSensor* receiver) {
    *reinterpret_cast<volatile u8*>(&_b0) = 0;
    _70 = sender;
    _78 = receiver;
    *reinterpret_cast<volatile u8*>(&_b2) = 0;
    *reinterpret_cast<volatile u64*>(&mIsBindEndOnGround) = 0x101000001000000;
    *reinterpret_cast<volatile u32*>(&_b3) = 0x10101;
}

void PlayerPuppet::end() {
    al::sendMsgBindEnd(_78, _70);
    cancel();
}

void PlayerPuppet::cancel() {
    _70 = nullptr;
    _78 = nullptr;
    mPlayerEffect->resumeStainEffect();
    _b3 = true;
    _b5 = true;
    _b6 = false;
}

void PlayerPuppet::setTrans(const sead::Vector3f& trans) {
    al::updatePoseTrans(mActor, trans);
}

void PlayerPuppet::setPose(const sead::Quatf& pose) {
    al::updatePoseQuat(mActor, pose);
}

void PlayerPuppet::setVelocity(const sead::Vector3f& velocity) {
    al::setVelocity(mActor, velocity);
}

void PlayerPuppet::resetPosition(const sead::Vector3f& trans) {
    al::resetPosition(mActor, trans);
    rs::resetCollision(mIUsePlayerCollision);
    mIPlayerModelChanger->resetPosition();
    mWorldEndBorderKeeper->reset();
}

const sead::Vector3f& PlayerPuppet::getTrans() const {
    return al::getTrans(mActor);
}

const sead::Vector3f& PlayerPuppet::getVelocity() const {
    return al::getVelocity(mActor);
}

const sead::Vector3f& PlayerPuppet::getGravity() const {
    return al::getGravity(mActor);
}

void PlayerPuppet::calcFront(sead::Vector3f* front) const {
    al::calcFrontDir(front, mActor);
}

void PlayerPuppet::calcUp(sead::Vector3f* up) const {
    al::calcUpDir(up, mActor);
}

void PlayerPuppet::startAction(const sead::SafeString& action) const {
    if (mPlayerAnimator->isSubAnimPlaying())
        mPlayerAnimator->endSubAnim();
    mPlayerAnimator->startAnim(action);
}

bool PlayerPuppet::isActionEnd() const {
    return mPlayerAnimator->isAnimEnd();
}

bool PlayerPuppet::isActionPlaying(const char* action) const {
    return mPlayerAnimator->isAnim(sead::SafeString(action));
}

void PlayerPuppet::setAnimRate(f32 rate) const {
    mPlayerAnimator->setAnimRate(rate);
}

f32 PlayerPuppet::getAnimFrameMax() const {
    return mPlayerAnimator->getAnimFrameMax();
}

void PlayerPuppet::startPlayerHitReaction(const char* name) {
    al::startHitReaction(mActor, name);
}

void PlayerPuppet::hide() {
    if (mPlayerDamageKeeper->isDamageInvalid())
        mPlayerDamageKeeper->reset(nullptr);
    mIPlayerModelChanger->hideModel();
}

void PlayerPuppet::show() {
    if (mPlayerDamageKeeper->isDamageInvalid())
        mPlayerDamageKeeper->reset(nullptr);
    mIPlayerModelChanger->showModel();
}

bool PlayerPuppet::isHidden() const {
    return mIPlayerModelChanger->isHiddenModel();
}

void PlayerPuppet::hideSilhouette() {
    if (mPlayerDamageKeeper->isDamageInvalid())
        mPlayerDamageKeeper->reset(nullptr);
    mIPlayerModelChanger->hideSilhouette();
}

void PlayerPuppet::showSilhouette() {
    if (mPlayerDamageKeeper->isDamageInvalid())
        mPlayerDamageKeeper->reset(nullptr);
    mIPlayerModelChanger->showSilhouette();
}

void PlayerPuppet::hideShadow() {
    mIPlayerModelChanger->hideShadowMask();
}

void PlayerPuppet::showShadow() {
    mIPlayerModelChanger->showShadowMask();
}

void PlayerPuppet::validateCollisionCheck() {
    mIsValidCollisionCheck = true;
}

void PlayerPuppet::invalidateCollisionCheck() {
    mIsValidCollisionCheck = false;
}

bool PlayerPuppet::isValidCollisionCheck() {
    return mIsValidCollisionCheck;
}

bool PlayerPuppet::isCollidedGround() {
    return rs::isCollidedGround(mIUsePlayerCollision);
}

const sead::Vector3f& PlayerPuppet::getCollidedGroundNormal() {
    return rs::getCollidedGroundNormal(mIUsePlayerCollision);
}

bool PlayerPuppet::requestDamage() {
    if (mPlayerDamageKeeper->isDamageInvalid())
        return false;
    mIsRequestDamage = true;
    return true;
}

void PlayerPuppet::setBindEndJump(const sead::Vector3f& velocity, s32 frames) {
    mIsBindEndJump = true;
    mBindEndJumpInfo->_0 = velocity;
    mBindEndJumpInfo->_c = frames;
    mBindEndJumpInfo->gravity = mPlayerConst->getJumpGravity();
}

void PlayerPuppet::setBindEndWallJump(const sead::Vector3f& velocity, s32 frames) {
    mIsBindEndJump = true;
    mBindEndJumpInfo->_0 = velocity;
    mBindEndJumpInfo->_c = frames;
    mBindEndJumpInfo->gravity = mPlayerConst->getWallJumpGravity();
}

void PlayerPuppet::validate2D() {
    mActorDimensionKeeper->validate();
}

void PlayerPuppet::keepOn2D() {
    mActorDimensionKeeper->forceChange2DKeep();
}

void PlayerPuppet::endKeepOn2D() {
    mActorDimensionKeeper->forceEndChange2DKeep();
}

void PlayerPuppet::requestUpdateRecoveryInfo(bool isKidsMode, bool isRecovery,
                                                   const sead::Vector3f& position,
                                                   const sead::Vector3f& up,
                                                   const al::AreaObj* areaObj) {
    _80 = true;
    _81 = isKidsMode;
    _82 = isRecovery;
    _84 = position;
    _90 = up;
    mAreaObj = areaObj;
}

bool PlayerPuppet::tryUpdateRecoveryInfo(bool* isKidsMode, bool* isRecovery,
                                         sead::Vector3f* position, sead::Vector3f* up,
                                         const al::AreaObj** areaObj) {
    if (!_80)
        return false;

    *isKidsMode = _81;
    *isRecovery = _82;
    position->z = _84.z;
    sead::MemUtil::copy(position, &_84, sizeof(f32) * 2);
    up->z = _90.z;
    sead::MemUtil::copy(up, &_90, sizeof(f32) * 2);
    *areaObj = mAreaObj;
    _80 = false;
    return true;
}

bool PlayerPuppet::isBinding() const {
    return _70 && _78;
}

bool PlayerPuppet::isNoCollide() const {
    return _70 && _78 && !mIsValidCollisionCheck;
}

void PlayerPuppet::clearRequestDamage() {
    mIsRequestDamage = false;
}

bool PlayerPuppet::isRequestDamage() const {
    return mIsRequestDamage;
}

void PlayerPuppet::setBindEndOnGround() {
    mIsBindEndOnGround = true;
}

bool PlayerPuppet::isBindEndOnGround() const {
    return mIsBindEndOnGround;
}

bool PlayerPuppet::isBindEndJump() const {
    return mIsBindEndJump;
}

void PlayerPuppet::validateSensor() {
    mIsSensorValid = true;
}

void PlayerPuppet::invalidateSensor() {
    mIsSensorValid = false;
}
