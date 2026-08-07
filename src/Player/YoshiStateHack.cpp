#include "Player/YoshiStateHack.h"

#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Enemy/HackerDepthShadowMapCtrl.h"
#include "Player/IUsePlayerCollision.h"
#include "Player/YoshiStateHackPlay.h"
#include "Util/PlayerHackFunction.h"

namespace {
NERVE_IMPL(YoshiStateHack, LockOn);
NERVE_IMPL(YoshiStateHack, Hack);
NERVES_MAKE_NOSTRUCT(YoshiStateHack, LockOn, Hack);
}  // namespace

void YoshiStateHack::appear() {
    al::ActorStateBase::appear();
    al::setNerve(this, &LockOn);
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

void YoshiStateHack::exeHack() {
    al::updateNerveState(this);
}

// NON_MATCHING: target preserves the post-call bit test and explicit true return, while Clang tail-calls the bool-returning play-state method; next seek the original interface return type/control-flow discriminator.
bool YoshiStateHack::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (!al::isNerve(this, &Hack))
        return false;
    if (!mStateHackPlay->attackSensor(self, other))
        return false;
    return true;
}
