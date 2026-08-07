#include "Player/PlayerEffect.h"

#include "Library/Base/StringUtil.h"
#include "Library/Effect/EffectSystemInfo.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/MatrixUtil.h"
#include "Library/Nature/WaterSurfaceFinder.h"

#include "Player/IPlayerModelChanger.h"
#include "Player/PlayerModelHolder.h"
#include "Util/PlayerUtil.h"

PlayerEffect::PlayerEffect(al::LiveActor* player, const PlayerModelHolder* modelHolder,
                           const sead::Matrix34f* spinCapMtx)
    : mPlayer(player), mModelHolder(modelHolder), mWaterSurfaceMtx(sead::Matrix34f::ident),
      mWaterInOutMtx(sead::Matrix34f::ident), mSandSinkMtx(sead::Matrix34f::ident),
      mRunEffectName(sead::SafeString::cEmptyString), mInvincibleActor(nullptr),
      mIsInvincibleEffectActive(false), mIsInvincibleEffectSuspended(false),
      mIsStainEffectSuspended(false), mInvincibleEffectRestartDelay(0) {
    al::IUseEffectKeeper* effectKeeper = player;
    auto* modelActor = modelHolder->findModelActor("Normal");
    al::setEffectFollowMtxPtr(effectKeeper, "Invincible",
                              al::getJointMtxPtr(modelActor, "JointRoot"));
    modelActor = modelHolder->findModelActor("Normal");
    al::setEffectFollowMtxPtr(effectKeeper, "InvincibleStart",
                              al::getJointMtxPtr(modelActor, "JointRoot"));
    al::setEffectFollowMtxPtr(effectKeeper, "WaterSurfaceInOut", &mWaterInOutMtx);
    al::setEffectFollowMtxPtr(effectKeeper, "WaterSurfaceInOutLarge", &mWaterInOutMtx);
    al::setEffectFollowMtxPtr(effectKeeper, "SandWait", &mSandSinkMtx);
    al::setEffectFollowMtxPtr(effectKeeper, "SandWalk", &mSandSinkMtx);
    al::setEffectNamedMtxPtr(effectKeeper, "WaterSurface", &mWaterSurfaceMtx);

    modelActor = modelHolder->findModelActor("Normal");
    al::IUseEffectKeeper* modelEffectKeeper = modelActor;
    al::setEffectNamedMtxPtr(modelEffectKeeper, "WaterSurface", &mWaterSurfaceMtx);
    if (spinCapMtx)
        al::setEffectNamedMtxPtr(modelEffectKeeper, "SpinCap", spinCapMtx);

    modelActor = modelHolder->tryFindModelActor("Normal2D");
    if (modelActor)
        mInvincibleActor = al::getSubActor(modelActor, "無敵表現[2D]");
}

void PlayerEffect::emitEffectWaterInOut(const sead::Matrix34f& mtx, bool isLarge) {
    mWaterInOutMtx = mtx;
    al::emitEffect(mPlayer, isLarge ? "WaterSurfaceInOutLarge" : "WaterSurfaceInOut", nullptr);
}

void PlayerEffect::updateWaterSurfaceMtx(const al::WaterSurfaceFinder* waterSurfaceFinder) {
    al::makeMtxRT(&mWaterSurfaceMtx, mPlayer);
    if (waterSurfaceFinder->isFoundSurface() &&
        !al::isNearZeroOrLess(waterSurfaceFinder->getDistance()))
        mWaterSurfaceMtx.setTranslation(waterSurfaceFinder->getSurfacePosition());
}

void PlayerEffect::tryEmitInvincibleEffect() {
    if (mIsInvincibleEffectActive) {
        al::IUseEffectKeeper* effectKeeper = mPlayer;
        al::setEffectParticleScale(effectKeeper, "InvincibleStart", 0.001f);
        al::setEffectParticleScale(effectKeeper, "Invincible", 0.001f);
        al::tryDeleteEffect(effectKeeper, "InvincibleStart");
        al::tryDeleteEffect(effectKeeper, "Invincible");
    }

    mIsInvincibleEffectActive = true;
    al::setEffectParticleScale(mPlayer, "InvincibleStart", 1.0f);
    al::setEffectParticleScale(mPlayer, "Invincible", 1.0f);

    if (rs::isPlayer2D(mPlayer)) {
        al::LiveActor* invincibleActor = mInvincibleActor;
        if (!al::isAlive(invincibleActor)) {
            invincibleActor->appear();
            al::startAction(invincibleActor, "InvincibleWait");
        }

        al::IUseEffectKeeper* effectKeeper = mPlayer;
        al::setEffectParticleScale(effectKeeper, "InvincibleStart", 0.001f);
        al::setEffectParticleScale(effectKeeper, "Invincible", 0.001f);
        al::tryDeleteEffect(effectKeeper, "InvincibleStart");
        al::tryDeleteEffect(effectKeeper, "Invincible");
    } else {
        al::emitEffect(mPlayer, "InvincibleStart", nullptr);
        al::emitEffect(mPlayer, "Invincible", nullptr);
    }

    if (rs::isPlayerHack(mPlayer) || mIsInvincibleEffectSuspended)
        suspendInvincibleEffect();
}

void PlayerEffect::suspendInvincibleEffect() {
    mIsInvincibleEffectSuspended = true;
    if (!mIsInvincibleEffectActive)
        return;

    al::IUseEffectKeeper* effectKeeper = mPlayer;
    al::setEffectParticleScale(effectKeeper, "InvincibleStart", 0.001f);
    al::setEffectParticleScale(effectKeeper, "Invincible", 0.001f);
    al::tryDeleteEffect(effectKeeper, "InvincibleStart");
    al::tryDeleteEffect(effectKeeper, "Invincible");
    mInvincibleActor->kill();
}

void PlayerEffect::tryDeleteInvincibleEffect() {
    al::tryDeleteEffect(mPlayer, "InvincibleStart");
    al::tryDeleteEffect(mPlayer, "Invincible");
    mInvincibleActor->kill();
    mIsInvincibleEffectActive = false;
    mIsInvincibleEffectSuspended = false;
}

void PlayerEffect::restartInvincibleEffect() {
    mIsInvincibleEffectSuspended = false;
    if (!mIsInvincibleEffectActive)
        return;

    if (rs::isPlayer2D(mPlayer)) {
        al::LiveActor* invincibleActor = mInvincibleActor;
        if (!al::isAlive(invincibleActor)) {
            invincibleActor->appear();
            al::startAction(invincibleActor, "InvincibleWait");
        }
    } else {
        mInvincibleEffectRestartDelay = 2;
    }
}

void PlayerEffect::updateInvincibleEffect(const IPlayerModelChanger* modelChanger,
                                          bool isForceDelete) {
    if (!mIsInvincibleEffectActive)
        return;

    if (isForceDelete) {
        tryDeleteInvincibleEffect();
        return;
    }

    if (mInvincibleEffectRestartDelay >= 1) {
        mInvincibleEffectRestartDelay = al::converge(mInvincibleEffectRestartDelay, 0, 1);
        if (modelChanger->isHiddenModel())
            mInvincibleEffectRestartDelay =
                sead::Mathi::clampMin((mInvincibleEffectRestartDelay), 2);

        if (mInvincibleEffectRestartDelay == 0) {
            al::setEffectParticleScale(mPlayer, "InvincibleStart", 1.0f);
            al::setEffectParticleScale(mPlayer, "Invincible", 1.0f);
            al::tryEmitEffect(mPlayer, "Invincible", nullptr);
        }
    }

    if (mIsInvincibleEffectSuspended || !modelChanger->isChange())
        return;

    if (modelChanger->is2DModel()) {
        al::IUseEffectKeeper* effectKeeper = mPlayer;
        al::setEffectParticleScale(effectKeeper, "InvincibleStart", 0.001f);
        al::setEffectParticleScale(effectKeeper, "Invincible", 0.001f);
        al::tryDeleteEffect(effectKeeper, "InvincibleStart");
        al::tryDeleteEffect(effectKeeper, "Invincible");

        al::LiveActor* invincibleActor = mInvincibleActor;
        if (!al::isAlive(invincibleActor)) {
            invincibleActor->appear();
            al::startAction(invincibleActor, "InvincibleWait");
        }
        mInvincibleEffectRestartDelay = 0;
    } else {
        mInvincibleEffectRestartDelay = 2;
        mInvincibleActor->kill();
    }
}

void PlayerEffect::clearRunEffect() {
    if (mRunEffectName.isEmpty())
        return;

    auto* modelActor = mModelHolder->getCurrentModelActor();
    al::tryDeleteEffect(modelActor, mRunEffectName.cstr());
    mRunEffectName = sead::SafeString::cEmptyString;
}

bool PlayerEffect::isRunEffectDashFast() const {
    return al::isEqualString(mRunEffectName.cstr(), "DashFast");
}

void PlayerEffect::tryStartRunEffectRunStart() {
    sead::SafeString* runEffectName = &mRunEffectName;
    const PlayerModelHolder* modelHolder = mModelHolder;
    if (al::isEqualString(runEffectName->cstr(), "RunStart"))
        return;

    auto* modelActor = modelHolder->getCurrentModelActor();
    al::emitEffect(modelActor, "RunStart", nullptr);
    if (!runEffectName->isEmpty())
        al::tryDeleteEffect(modelActor, runEffectName->cstr());
    *runEffectName = sead::SafeString("RunStart");
}

void PlayerEffect::tryStartRunEffectRun() {
    sead::SafeString* runEffectName = &mRunEffectName;
    const PlayerModelHolder* modelHolder = mModelHolder;
    if (al::isEqualString(runEffectName->cstr(), "Run"))
        return;

    auto* modelActor = modelHolder->getCurrentModelActor();
    al::emitEffect(modelActor, "Run", nullptr);
    if (!runEffectName->isEmpty())
        al::tryDeleteEffect(modelActor, runEffectName->cstr());
    *runEffectName = sead::SafeString("Run");
}

void PlayerEffect::tryStartRunEffectDash() {
    sead::SafeString* runEffectName = &mRunEffectName;
    const PlayerModelHolder* modelHolder = mModelHolder;
    if (al::isEqualString(runEffectName->cstr(), "Dash"))
        return;

    auto* modelActor = modelHolder->getCurrentModelActor();
    al::emitEffect(modelActor, "Dash", nullptr);
    if (!runEffectName->isEmpty())
        al::tryDeleteEffect(modelActor, runEffectName->cstr());
    *runEffectName = sead::SafeString("Dash");
}

void PlayerEffect::tryStartRunEffectDashFast() {
    sead::SafeString* runEffectName = &mRunEffectName;
    const PlayerModelHolder* modelHolder = mModelHolder;
    if (al::isEqualString(runEffectName->cstr(), "DashFast"))
        return;

    auto* modelActor = modelHolder->getCurrentModelActor();
    al::emitEffect(modelActor, "DashFast", nullptr);
    if (!runEffectName->isEmpty())
        al::tryDeleteEffect(modelActor, runEffectName->cstr());
    *runEffectName = sead::SafeString("DashFast");
}

void PlayerEffect::tryStartRunEffectDashWaterSurface() {
    sead::SafeString* runEffectName = &mRunEffectName;
    const PlayerModelHolder* modelHolder = mModelHolder;
    if (al::isEqualString(runEffectName->cstr(), "DashFastPuddle"))
        return;

    auto* modelActor = modelHolder->getCurrentModelActor();
    al::emitEffect(modelActor, "DashFastPuddle", nullptr);
    if (!runEffectName->isEmpty())
        al::tryDeleteEffect(modelActor, runEffectName->cstr());
    *runEffectName = sead::SafeString("DashFastPuddle");
}

void PlayerEffect::tryEmitRollingEffect() {
    al::tryEmitEffect(mModelHolder->getCurrentModelActor(), "Rolling", nullptr);
}

void PlayerEffect::tryDeleteRollingEffect() {
    al::tryDeleteEffect(mModelHolder->getCurrentModelActor(), "Rolling");
}

void PlayerEffect::clearStainEffect() {
    al::tryDeleteEffect(mModelHolder->findModelActor("Normal"), "StateWet");
    al::tryDeleteEffect(mModelHolder->findModelActor("Normal"), "StatePoison");
    al::tryDeleteEffect(mModelHolder->findModelActor("Normal"), "StateFire");
    al::tryDeleteEffect(mModelHolder->findModelActor("Normal"), "StateIce");
}

void PlayerEffect::tryDeleteWetEffect() {
    al::tryDeleteEffect(mModelHolder->findModelActor("Normal"), "StateWet");
}

void PlayerEffect::tryDeleteStainPoisonEffect() {
    al::tryDeleteEffect(mModelHolder->findModelActor("Normal"), "StatePoison");
}

void PlayerEffect::tryDeleteStainFireEffect() {
    al::tryDeleteEffect(mModelHolder->findModelActor("Normal"), "StateFire");
}

void PlayerEffect::tryDeleteStainIceEffect() {
    al::tryDeleteEffect(mModelHolder->findModelActor("Normal"), "StateIce");
}

void PlayerEffect::tryEmitWetEffect() {
    if (mIsStainEffectSuspended)
        return;

    auto* modelActor = mModelHolder->findModelActor("Normal");
    if (!al::isEffectEmitting(modelActor, "StateWet"))
        al::tryEmitEffect(modelActor, "StateWet", nullptr);
}

void PlayerEffect::tryEmitStainPoisonEffect() {
    if (mIsStainEffectSuspended)
        return;

    auto* modelActor = mModelHolder->findModelActor("Normal");
    if (!al::isEffectEmitting(modelActor, "StatePoison"))
        al::tryEmitEffect(modelActor, "StatePoison", nullptr);
}

void PlayerEffect::tryEmitStainFireEffect() {
    if (!mIsStainEffectSuspended)
        al::tryEmitEffect(mModelHolder->findModelActor("Normal"), "StateFire", nullptr);
}

void PlayerEffect::tryEmitStainIceEffect() {
    if (mIsStainEffectSuspended)
        return;

    auto* modelActor = mModelHolder->findModelActor("Normal");
    if (!al::isEffectEmitting(modelActor, "StateIce"))
        al::tryEmitEffect(modelActor, "StateIce", nullptr);
}

void PlayerEffect::tryEmitSandSinkEffect(const sead::Vector3f& position, const sead::Vector3f& up,
                                         bool isWalk) {
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    al::calcFrontDir(&front, mPlayer);
    if (al::isParallelDirection(front, up, 0.01f))
        return;

    al::makeMtxUpFrontPos(&mSandSinkMtx, up, front, position);
    if (isWalk) {
        al::tryDeleteEffect(mPlayer, "SandWait");
        al::tryEmitEffect(mPlayer, "SandWalk", nullptr);
    } else {
        al::tryEmitEffect(mPlayer, "SandWait", nullptr);
        al::tryDeleteEffect(mPlayer, "SandWalk");
    }
}

void PlayerEffect::tryDeleteSandSinkEffect() {
    al::tryDeleteEffect(mPlayer, "SandWait");
    al::tryDeleteEffect(mPlayer, "SandWalk");
}

void PlayerEffect::tryDeleteDamageFireRunEffect() {
    al::tryDeleteEffect(mModelHolder->getCurrentModelActor(), "FireRunHipSmoke");
}
