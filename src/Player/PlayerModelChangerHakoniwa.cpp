#include "Player/PlayerModelChangerHakoniwa.h"

#include "Player/PlayerCostumeInfo.h"

s32 PlayerModelChangerHakoniwa::calcCostumeWarmLevel(s32 level) const {
    return mPlayerCostumeInfo->calcWarmLevel(level);
}

bool PlayerModelChangerHakoniwa::isFireFlower() const {
    return false;
}

bool PlayerModelChangerHakoniwa::isMini() const {
    return false;
}

bool PlayerModelChangerHakoniwa::isChange() const {
    return mIsChange;
}

bool PlayerModelChangerHakoniwa::is2DModel() const {
    return mIsMode2D;
}

void PlayerModelChangerHakoniwa::hideModel() {
    mIsVisibilityNeedsSync = true;
    mIsModelVisible = false;
    mIsSilhouetteVisible = false;
    mIsShadowMaskVisible = false;
}

void PlayerModelChangerHakoniwa::hideSilhouette() {
    mIsVisibilityNeedsSync = true;
    mIsSilhouetteVisible = false;
}

void PlayerModelChangerHakoniwa::hideShadowMask() {
    mIsVisibilityNeedsSync = true;
    mIsShadowMaskVisible = false;
}

void PlayerModelChangerHakoniwa::showModel() {
    mIsVisibilityNeedsSync = true;
    mIsModelVisible = true;
    mIsSilhouetteVisible = true;
    mIsShadowMaskVisible = true;
}

void PlayerModelChangerHakoniwa::showSilhouette() {
    mIsVisibilityNeedsSync = true;
    mIsSilhouetteVisible = true;
}

void PlayerModelChangerHakoniwa::showShadowMask() {
    mIsVisibilityNeedsSync = true;
    mIsShadowMaskVisible = true;
}

bool PlayerModelChangerHakoniwa::isHiddenModel() const {
    return !mIsModelVisible;
}

bool PlayerModelChangerHakoniwa::isHiddenShadowMask() const {
    return !mIsShadowMaskVisible;
}

void PlayerModelChangerHakoniwa::startDamageStopDemo() {
    mIsBlinkingFromDamage = true;
    mDamageTimer = 63;
}
