#include "Player/PlayerPuppet.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"

#include "Player/PlayerAnimator.h"
#include "Util/ActorDimensionKeeper.h"
#include "Util/PlayerCollisionUtil.h"
#include "Player/IPlayerModelChanger.h"

void PlayerPuppet::setTrans(const sead::Vector3f& trans) {
    al::updatePoseTrans(mActor, trans);
}

void PlayerPuppet::setPose(const sead::Quatf& pose) {
    al::updatePoseQuat(mActor, pose);
}

void PlayerPuppet::setVelocity(const sead::Vector3f& velocity) {
    al::setVelocity(mActor, velocity);
}

bool PlayerPuppet::isActionEnd() const {
    return mPlayerAnimator->isAnimEnd();
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

void PlayerPuppet::invalidateCollisionCheck() {
    mIsValidCollisionCheck = false;
}

bool PlayerPuppet::isValidCollisionCheck() {
    return mIsValidCollisionCheck;
}

bool PlayerPuppet::isCollidedGround() {
    return rs::isCollidedGround(mIUsePlayerCollision);
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

void PlayerPuppet::clearRequestDamage() {
    mIsRequestDamage = false;
}

bool PlayerPuppet::isRequestDamage() const {
    return mIsRequestDamage;
}

bool PlayerPuppet::isBindEndOnGround() const {
    return mIsBindEndOnGround;
}

bool PlayerPuppet::isBindEndJump() const {
    return mIsBindEndJump;
}

void PlayerPuppet::invalidateSensor() {
    mIsSensorValid = false;
}

void PlayerPuppet::validateCollisionCheck() {
    mIsValidCollisionCheck = true;
}

void PlayerPuppet::setBindEndOnGround() {
    mIsBindEndOnGround = true;
}

void PlayerPuppet::validateSensor() {
    mIsSensorValid = true;
}


bool PlayerPuppet::isHidden() const {
    return mIPlayerModelChanger->isHiddenModel();
}

void PlayerPuppet::hideShadow() {
    mIPlayerModelChanger->hideShadowMask();
}

void PlayerPuppet::showShadow() {
    mIPlayerModelChanger->showShadowMask();
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

const sead::Vector3f& PlayerPuppet::getCollidedGroundNormal() {
    return rs::getCollidedGroundNormal(mIUsePlayerCollision);
}

bool PlayerPuppet::isBinding() const {
    return _70 && _78;
}
