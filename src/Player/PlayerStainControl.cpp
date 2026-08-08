#include "Player/PlayerStainControl.h"

#include "Library/Base/StringUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/Math/MathUtil.h"

#include "Player/PlayerEffect.h"
#include "Player/PlayerFunction.h"

void PlayerStainControl::recordExplosion() {
    mRequestStainType = StainTypeFire;
    mRequestRate = 1.0f;
}

void PlayerStainControl::recordDamageFire() {
    mRequestStainType = StainTypeFire;
    mRequestRate = 0.005f;
}

void PlayerStainControl::recordDamageFireDead() {
    recordExplosion();
}

void PlayerStainControl::recordBlackSmoke() {
    mRequestStainType = StainTypeFire;
    mRequestRate = 0.02f;
}

void PlayerStainControl::recordPoison() {
    mRequestStainType = StainTypePoison;
    mRequestRate = 1.0f;
}

void PlayerStainControl::recordIceWater() {
    mRequestStainType = StainTypeIce;
    mRequestRate = 0.001f;
}

void PlayerStainControl::recordBlizzard() {
    if (mIsBlizzard)
        return;

    mIsBlizzard = true;
    mBlizzardValueA = 0.00025f;
    mBlizzardValueB = 0;
    mBlizzardCounter = mStainParams.unsafeAt(StainTypeIce)->blizzardCount;
    mBlizzardRate = 0.1f;
}

void PlayerStainControl::recordSandMove(const char* materialCode) {
    if (al::isInRange(mCurrentStainType, StainTypePoison, StainTypeIce) ||
        al::isInRange(mRequestStainType, StainTypePoison, StainTypeIce))
        return;

    const StainType stainType(static_cast<s32>(getSandType(materialCode)));
    if (stainType == StainTypeNone)
        return;
    mRequestStainType = stainType;

    mRequestRate = 0.005f;
}

bool PlayerStainControl::isEnableLowPriorityStain() const {
    return !al::isInRange(mCurrentStainType, StainTypePoison, StainTypeIce) &&
           !al::isInRange(mRequestStainType, StainTypePoison, StainTypeIce);
}

u64 PlayerStainControl::getSandType(const char* materialCode) const {
    if (al::isEqualString(materialCode, "SandDesert"))
        return StainTypeSandDesert;
    if (al::isEqualString(materialCode, "SandSea"))
        return StainTypeSand;
    if (al::isEqualString(materialCode, "SandMoon"))
        return StainTypeSandMoon;
    if (al::isEqualString(materialCode, "SandLake"))
        return StainTypeSand;
    return al::isEqualString(materialCode, "Sand") ? StainTypeSand : StainTypeNone;
}

void PlayerStainControl::recordSnowMove(const char* materialCode) {
    if (!al::isEqualString("Snow", materialCode) || mIsBlizzard)
        return;

    mIsBlizzard = true;
    mBlizzardValueA = 0.0005f;
    mBlizzardValueB = 360;
    mBlizzardCounter = static_cast<s32>(
        static_cast<f32>(mStainParams.unsafeAt(StainTypeIce)->blizzardCount) / 0.1f);
    mBlizzardRate = 0.1f;
}

void PlayerStainControl::recordSnowBySensor() {
    mIsBlizzard = true;
    mBlizzardValueA = 0.03f;
    mBlizzardValueB = 360;
    mBlizzardCounter = static_cast<s32>(
        static_cast<f32>(mStainParams.unsafeAt(StainTypeIce)->blizzardCount) / 0.1f);
    mBlizzardRate = 0.1f;
}

void PlayerStainControl::recordSandHeavyLand(const char* materialCode) {
    clearCurrentStain(true);

    if (al::isInRange(mCurrentStainType, StainTypePoison, StainTypeIce) ||
        al::isInRange(mRequestStainType, StainTypePoison, StainTypeIce))
        return;

    const StainType stainType(static_cast<s32>(getSandType(materialCode)));
    if (stainType == StainTypeNone)
        return;
    mRequestStainType = stainType;

    mRequestRate = 1.0f;
}

void PlayerStainControl::clearCurrentStain(bool emitReaction) {
    if (static_cast<u32>(mCurrentStainType) != StainTypeNone) {
        const PlayerStainControlParam* param = mStainParams.unsafeAt(mCurrentStainType);
        (PlayerFunction::setupPlayerStain)(mModelActor, mStainParts, param->type, param->color,
                                           param->setupRate, 0.0f, 0.0f, 0.0f);

        if (emitReaction)
            tryEmitClearStainEffect(static_cast<StainType>(mCurrentStainType), mCurrentRate);

        tryDeleteStainEffect(static_cast<StainType>(mCurrentStainType), 0.0f);
    }

    mCurrentStainType = StainTypeNone;
    _7c = 0;
    _74 = 0;
    _78 = 0;
    mCurrentRate = 0.0f;
    _70 = 0;
}

void PlayerStainControl::recordSandMoonBySensor() {
    mRequestStainType = StainTypeSandMoon;
    mRequestRate = 1.0f;
}

void PlayerStainControl::recordInWater() {
    mIsInWater = true;
}

void PlayerStainControl::recordInWet() {
    if (mRequestStainType == StainTypeNone &&
        (mCurrentStainType == StainTypeNone || mIsStainInvalid))
        mIsInWet = true;
}

void PlayerStainControl::noticeStartHack() {
    mIsHack = true;
    if (mCurrentStainType == StainTypeNone)
        return;
    tryDeleteStainEffect(static_cast<StainType>(mCurrentStainType), 0.0f);
}

void PlayerStainControl::tryDeleteStainEffect(StainType stainType, f32 rate) {
    switch (stainType) {
    case StainTypePoison:
        if (!(rate > 0.25f))
            mEffect->tryDeleteStainPoisonEffect();
        break;
    case StainTypeFire:
        if (!(rate > 0.25f))
            mEffect->tryDeleteStainFireEffect();
        break;
    case StainTypeIce:
        if (!(rate > 0.25f))
            mEffect->tryDeleteStainIceEffect();
        break;
    default:
        break;
    }
}

void PlayerStainControl::noticeEndHack() {
    mIsHack = false;
    clearCurrentStain(false);
    mRequestStainType = StainTypeNone;
    mIsBlizzard = false;
}

void PlayerStainControl::clearStain() {
    clearCurrentStain(false);
    mRequestStainType = StainTypeNone;
    mIsBlizzard = false;
}

void PlayerStainControl::noticeMainShineGet() {
    if (static_cast<u32>(mCurrentStainType) - StainTypePoison <= 1) {
        clearCurrentStain(false);
        mRequestStainType = StainTypeNone;
        mIsBlizzard = false;
    }
    update();
}

void PlayerStainControl::clearStainRequest() {
    mRequestStainType = StainTypeNone;
    mIsBlizzard = false;
}

void PlayerStainControl::tryEmitStainEffect(StainType stainType) {
    if (mIsHack)
        return;

    switch (stainType) {
    case StainTypePoison:
        mEffect->tryDeleteStainPoisonEffect();
        mEffect->tryEmitStainPoisonEffect();
        break;
    case StainTypeFire:
        if (!(mCurrentRate < 0.75f)) {
            mEffect->tryDeleteStainFireEffect();
            mEffect->tryEmitStainFireEffect();
        }
        break;
    case StainTypeIce:
        if (!(mCurrentRate < 0.3f))
            mEffect->tryEmitStainIceEffect();
        break;
    default:
        break;
    }
}

bool PlayerStainControl::isEnableValidateStain() const {
    return !mIsStainInvalid && mCurrentStainType != StainTypeNone;
}

bool PlayerStainControl::isEnableInvalidateStain() const {
    return mIsStainInvalid && mCurrentStainType == StainTypeNone;
}

void PlayerStainControl::tryEmitClearStainEffect(StainType stainType, f32 rate) {
    switch (stainType) {
    case StainTypePoison:
        if (!(rate <= 0.25f))
            al::startHitReaction(mPlayer, "汚れ落とし(毒)");
        break;
    case StainTypeFire:
        if (!(rate <= 0.25f))
            al::startHitReaction(mPlayer, "汚れ落とし(スス)");
        break;
    case StainTypeIce:
        if (!(rate <= 0.25f))
            al::startHitReaction(mPlayer, "汚れ落とし(凍結)");
        break;
    default:
        break;
    }
}

