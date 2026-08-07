#include "Player/PlayerModelChangerHakoniwa.h"

#include "Library/Bgm/BgmLineFunction.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Shadow/ActorShadowUtil.h"
#include "Library/Shadow/ShadowKeeper.h"
#include "Library/Se/SeFunction.h"

#include "Player/PlayerCostumeInfo.h"
#include "Player/PlayerFunction.h"
#include "Player/PlayerModelHolder.h"
#include "Player/PlayerPainPartsKeeper.h"
#include "Util/ActorDimensionUtil.h"

PlayerModelChangerHakoniwa::PlayerModelChangerHakoniwa(
    const al::LiveActor* actor, PlayerModelHolder* modelHolder,
    PlayerPainPartsKeeper* painPartsKeeper, PlayerCostumeInfo* costumeInfo,
    const IUseDimension* dimension)
    : mLiveActor(actor), mIsChange(false), mIsMode2D(false), mLiveActor2(nullptr),
      mPlayerModelHolder(modelHolder), mPlayerPainPartsKeeper(painPartsKeeper),
      mPlayerCostumeInfo(costumeInfo), mIUseDimension(dimension),
      mIsVisibilityNeedsSync(false), mIsModelVisible(true), mIsSilhouetteVisible(true),
      mIsShadowMaskVisible(true), mIsBlinkingFromDamage(false), mDamageTimer(0),
      mIsMusicStarted(false), mIsNeedHairControl(false), mIsNeedSyncBodyHair(false),
      mIsSyncFaceBeard(false), mIsSyncStrap(false), mIsPreventHeadPain(false) {
    mLiveActor2 = mPlayerModelHolder->getCurrentModelActor();
    mLiveActor2->makeActorAlive();
    mIsMode2D = mPlayerModelHolder->isCurrentModelLabelSubString("2D");
    mIsChange = true;
    mIsNeedHairControl = PlayerFunction::isNeedHairControl(
        mPlayerCostumeInfo->getBodyInfo(), mPlayerCostumeInfo->getHeadInfo()->costumeName);
    mIsNeedSyncBodyHair = mPlayerCostumeInfo->isNeedSyncBodyHair();
    mIsSyncFaceBeard = mPlayerCostumeInfo->isSyncFaceBeard();
    mIsSyncStrap = mPlayerCostumeInfo->isSyncStrap();
    mIsPreventHeadPain = mPlayerCostumeInfo->isPreventHeadPain();
}

void PlayerModelChangerHakoniwa::initStartModel() {
    mIsMode2D = rs::is2D(mIUseDimension);
    mPlayerModelHolder->changeModel(getModelName());
    al::LiveActor* modelActor = mPlayerModelHolder->getCurrentModelActor();
    al::copyPose(modelActor, mLiveActor);
    mLiveActor2->makeActorDead();
    modelActor->makeActorAlive();
    syncShowHide(modelActor);
    mLiveActor2 = modelActor;
    mIsChange = true;
}

const char* PlayerModelChangerHakoniwa::getModelName() {
    if (!mIsMode2D)
        return "Normal";
    return "Normal2D";
}

void PlayerModelChangerHakoniwa::changeModel(al::LiveActor* modelActor) {
    mLiveActor2->makeActorDead();
    modelActor->makeActorAlive();
    syncShowHide(modelActor);
    mLiveActor2 = modelActor;
    mIsChange = true;
}

void PlayerModelChangerHakoniwa::update(bool updateDimension, bool seOnly) {
    mIsChange = false;

    if (PlayerFunction::isPlayerDeadStatus(mLiveActor) && mIsMode2D) {
        al::LiveActor* currentModelActor = mLiveActor2;
        al::LiveActor* mini2D = mPlayerModelHolder->findModelActor("Mini2D");
        if (currentModelActor != mini2D) {
            mPlayerModelHolder->changeModel("Mini2D");
            mini2D = mPlayerModelHolder->getCurrentModelActor();
            mLiveActor2->makeActorDead();
            mini2D->makeActorAlive();
            syncShowHide(mini2D);
            mLiveActor2 = mini2D;
            mIsChange = true;
        }
    }

    if (!updateDimension)
        return;

    if (mIsMode2D != rs::is2D(mIUseDimension)) {
        mIsMode2D = !mIsMode2D;
        if (mIsMode2D) {
            al::startHitReaction(mLiveActor, "2D");
            al::startBgmSituation(mLiveActor, "In2DArea", false);
            alSeFunction::endSituation(mLiveActor, "2D3Dの外", -1);
            alSeFunction::startSituation(mLiveActor, "2D3D", -1);
        } else {
            if (seOnly)
                al::startHitReaction(mLiveActor, "3D変身[SEのみ]");
            else
                al::startHitReaction(mLiveActor, "3D変身");
            al::endBgmSituation(mLiveActor, "In2DArea", false);
            alSeFunction::startSituation(mLiveActor, "2D3Dの外", -1);
            alSeFunction::endSituation(mLiveActor, "2D3D", -1);
        }

        mPlayerModelHolder->changeModel(getModelName());
        al::LiveActor* modelActor = mPlayerModelHolder->getCurrentModelActor();
        mLiveActor2->makeActorDead();
        modelActor->makeActorAlive();
        syncShowHide(modelActor);
        mLiveActor2 = modelActor;
        mIsChange = true;
    }

    if (!mIsMusicStarted) {
        if (mIsMode2D) {
            al::startBgmSituation(mLiveActor, "In2DArea", true);
            alSeFunction::endSituation(mLiveActor, "2D3Dの外", -1);
            alSeFunction::startSituation(mLiveActor, "2D3D", -1);
        } else {
            al::endBgmSituation(mLiveActor, "In2DArea", true);
            alSeFunction::startSituation(mLiveActor, "2D3Dの外", -1);
            alSeFunction::endSituation(mLiveActor, "2D3D", -1);
        }
        mIsMusicStarted = true;
    }
}

void PlayerModelChangerHakoniwa::syncHost(bool syncVisibility) {
    if (mIsVisibilityNeedsSync && syncVisibility) {
        syncShowHide(mLiveActor2);
        mIsVisibilityNeedsSync = false;
    }
    if (!mIsMode2D)
        al::setScale(mLiveActor2, al::getScale(mLiveActor));
    al::copyPose(mLiveActor2, mLiveActor);
    if (mIsChange) {
        mLiveActor2->makeActorAlive();
        al::resetPosition(mLiveActor2);
    }
}

void PlayerModelChangerHakoniwa::syncShowHide(al::LiveActor* modelActor) {
    if (mIsModelVisible)
        al::showModelIfHide(modelActor);
    else
        al::hideModelIfShow(modelActor);

    al::ShadowKeeper* shadowKeeper = modelActor->getShadowKeeper();
    if (mIsShadowMaskVisible) {
        if (shadowKeeper && shadowKeeper->getDepthShadowMapCtrl())
            al::showDepthShadowMap(modelActor, PlayerFunction::getPlayerDepthGroundShadowName());
    } else if (shadowKeeper && shadowKeeper->getDepthShadowMapCtrl()) {
        al::hideDepthShadowMap(modelActor, PlayerFunction::getPlayerDepthGroundShadowName());
    }

    if (mIsSilhouetteVisible)
        al::showSilhouetteModel(modelActor);
    else
        al::hideSilhouetteModel(modelActor);
}

void PlayerModelChangerHakoniwa::updateDamageStopDemo() {
    mIsChange = false;
    if (mDamageTimer) {
        mDamageTimer = al::converge(mDamageTimer, 0, 2);
        if (static_cast<u32>(mDamageTimer & 7) >= static_cast<u32>(mDamageTimer >> 3))
            hideModel();
        else
            showModel();
        if (!mDamageTimer)
            showModel();
    } else {
        mIsBlinkingFromDamage = false;
    }
}

void PlayerModelChangerHakoniwa::syncHostDamageStopDemo(bool syncVisibility) {
    if (mIsVisibilityNeedsSync && syncVisibility) {
        syncShowHide(mLiveActor2);
        mIsVisibilityNeedsSync = false;
    }
    al::copyPose(mLiveActor2, mLiveActor);
    al::resetPosition(mLiveActor2);
}

void PlayerModelChangerHakoniwa::resetPosition() {
    al::copyPose(mLiveActor2, mLiveActor);
    al::resetPosition(mLiveActor2);
    if (isHiddenModel())
        return;

    const s32 subActorNum = al::getSubActorNum(mLiveActor2);
    for (s32 i = 0; i < subActorNum; i++)
        al::resetPosition(al::getSubActor(mLiveActor2, i));
    mPlayerPainPartsKeeper->resetPosition();
}

void PlayerModelChangerHakoniwa::syncModelBoneVisibility() {
    if (is2DModel())
        return;

    al::LiveActor* modelActor = mPlayerModelHolder->getCurrentModelActor();
    al::LiveActor* head = al::tryGetSubActor(modelActor, "頭");
    if (!head)
        return;

    if (mIsNeedSyncBodyHair)
        PlayerFunction::syncBodyHairVisibility(al::getSubActor(modelActor, "髪"), head);
    if (mIsSyncFaceBeard)
        PlayerFunction::syncMarioFaceBeardVisibility(al::getSubActor(modelActor, "顔"), head);
    if (mIsSyncStrap)
        PlayerFunction::syncMarioHeadStrapVisibility(head);
    if (mIsNeedHairControl)
        PlayerFunction::hideHairVisibility(head);
}

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
