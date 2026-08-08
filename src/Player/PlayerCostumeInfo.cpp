#include "Player/PlayerCostumeInfo.h"

#include <basis/seadNew.h>

#include "Library/Base/StringUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Resource/ResourceFunction.h"
#include "Library/Yaml/ByamlUtil.h"

PlayerBodyCostumeInfo::PlayerBodyCostumeInfo(const char* costumeName) : costumeName(costumeName) {}

PlayerHeadCostumeInfo::PlayerHeadCostumeInfo(const char* costumeName) : costumeName(costumeName) {}

PlayerCostumeInfo::PlayerCostumeInfo() = default;

void PlayerCostumeInfo::init(const PlayerBodyCostumeInfo* body, const PlayerHeadCostumeInfo* head) {
    mBodyInfo = body;
    mHeadInfo = head;
}

bool PlayerCostumeInfo::isEnableBigEar() const {
    return mBodyInfo->isBigEar && mHeadInfo->isEnableBigEar;
}

bool PlayerCostumeInfo::isEnableHairNoCap() const {
    return mHeadInfo->isEnableHairNoCap &&
           (mBodyInfo->isUseBodyHair && mBodyInfo->isExistHairNoCap);
}

bool PlayerCostumeInfo::isEnableCostume2D() const {
    if (mBodyInfo->isNoPairHead)
        return al::isEqualString(mHeadInfo->costumeName, "Mario");
    return al::isEqualString(mBodyInfo->costumeName, mHeadInfo->costumeName);
}

bool PlayerCostumeInfo::isNeedShrinkNose() const {
    return mHeadInfo->isShrinkNose;
}

bool PlayerCostumeInfo::isNeedBodyHair() const {
    if (!mBodyInfo->isUseBodyHair || (mBodyInfo->isMario64 && mHeadInfo->isMario64))
        return false;
    return true;
}

bool PlayerCostumeInfo::isNeedSyncBodyHair() const {
#ifdef MATCHING_HACK_NX_CLANG
    s64 v1 = *(s64*)&mBodyInfo->isUseBodyHair;
    if (!((char)v1))
        return false;
    if ((v1 & 0xFF00000000LL) && mHeadInfo->isMario64)
        return false;
    return !mBodyInfo->isMario64;
#else
    if (!mBodyInfo->isUseBodyHair)
        return false;
    if (mBodyInfo->isMario64 && mHeadInfo->isMario64)
        return false;
    return !mBodyInfo->isMario64;
#endif
}

bool PlayerCostumeInfo::isNeedFullFaceAnim() const {
    return mHeadInfo->isFullFace;
}

bool PlayerCostumeInfo::isHidePainNose() const {
    return mBodyInfo->isHidePainNose || mHeadInfo->isInvisibleHead;
}

bool PlayerCostumeInfo::isEnableEarring() const {
    if (mBodyInfo->isUseEarringPeach)
        return !mHeadInfo->isHideEarringPeach;
    return !mBodyInfo->isUseEarringLink || !mHeadInfo->isHideEarringLink;
}

bool PlayerCostumeInfo::isSyncFaceBeard() const {
    return mBodyInfo->isUseBeard && mHeadInfo->isHideBeard;
}

bool PlayerCostumeInfo::isSyncStrap() const {
    return mHeadInfo->isUseStrap && !mBodyInfo->isUseBeard;
}

bool PlayerCostumeInfo::isFollowJoeStrap() const {
    return mHeadInfo->isUseStrap && !mBodyInfo->isMario64;
}

bool PlayerCostumeInfo::isPreventHeadPain() const {
    return mHeadInfo->isPreventHead;
}

bool PlayerCostumeInfo::isInvisibleHead() const {
    return mHeadInfo->isInvisibleHead;
}

s32 PlayerCostumeInfo::calcWarmLevel(s32 baseLevel) const {
    if (mBodyInfo->isIgnoreTemperature)
        return false;
    return al::clamp(mBodyInfo->warmLevel + baseLevel, -3, 3);
}


namespace PlayerCostumeFunction {

PlayerBodyCostumeInfo* createBodyCostumeInfo(al::Resource* resource, const char* costumeName) {
    auto* info = new PlayerBodyCostumeInfo(costumeName);
    if (!al::isExistResourceYaml(resource, "InitCostumeInfoBody", nullptr))
        return info;

    al::ByamlIter iter{al::findResourceYaml(resource, "InitCostumeInfoBody", nullptr)};
    al::tryGetByamlS32(&info->warmLevel, iter, "WarmLevel");
    al::tryGetByamlBool(&info->isIgnoreTemperature, iter, "IsIgnoreTemperature");
    al::tryGetByamlBool(&info->isUseHeadSuffix, iter, "IsUseHeadSuffix");
    al::tryGetByamlBool(&info->isBigEar, iter, "IsBigEar");
    al::tryGetByamlBool(&info->isHideHeadHair, iter, "IsHideHeadHair");
    al::tryGetByamlBool(&info->isUseBodyHair, iter, "IsUseBodyHair");
    al::tryGetByamlBool(&info->isExistHairNoCap, iter, "IsExistHairNoCap");
    al::tryGetByamlBool(&info->isUseShortHead, iter, "IsUseShortHead");
    al::tryGetByamlBool(&info->isNoPairHead, iter, "IsNoPairHead");
    al::tryGetByamlBool(&info->isMario64, iter, "IsMario64");
    al::tryGetByamlBool(&info->isHidePainNose, iter, "IsHidePainNose");
    al::tryGetByamlBool(&info->isUseBeard, iter, "IsUseBeard");
    al::tryGetByamlBool(&info->isUseEarringPeach, iter, "IsUseEarringPeach");
    al::tryGetByamlBool(&info->isUseEarringLink, iter, "IsUseEarringLink");
    return info;
}

PlayerHeadCostumeInfo* createHeadCostumeInfo(al::Resource* resource, const char* costumeName,
                                              bool isInvisibleHead) {
    auto* info = new PlayerHeadCostumeInfo(costumeName);
    if (al::isExistResourceYaml(resource, "InitCostumeInfoHead", nullptr)) {
        al::ByamlIter iter{al::findResourceYaml(resource, "InitCostumeInfoHead", nullptr)};
        al::tryGetByamlBool(&info->isFullFace, iter, "IsFullFace");
        al::tryGetByamlBool(&info->isShrinkNose, iter, "IsShrinkNose");
        al::tryGetByamlBool(&info->isPreventHead, iter, "IsPreventHead");
        al::tryGetByamlBool(&info->isEnableBigEar, iter, "IsEnableBigEar");
        al::tryGetByamlBool(&info->isEnableHairNoCap, iter, "IsEnableHairNoCap");
        al::tryGetByamlBool(&info->isMario64, iter, "IsMario64");
        al::tryGetByamlBool(&info->isHaveShort, iter, "IsHaveShort");
        al::tryGetByamlBool(&info->isHideBeard, iter, "IsHideBeard");
        al::tryGetByamlBool(&info->isHideEarringPeach, iter, "IsHideEarringPeach");
        al::tryGetByamlBool(&info->isHideEarringLink, iter, "IsHideEarringLink");
        al::tryGetByamlBool(&info->isUseStrap, iter, "IsUseStrap");
    }
    info->isInvisibleHead = isInvisibleHead;
    return info;
}

}  // namespace PlayerCostumeFunction
