#pragma once

#include <basis/seadTypes.h>

#include "Library/HostIO/HioNode.h"

#include "Player/IPlayerModelChanger.h"

namespace al {
class LiveActor;
}
class PlayerModelHolder;
class PlayerPainPartsKeeper;
class PlayerCostumeInfo;
class IUseDimension;

class PlayerModelChangerHakoniwa : public IPlayerModelChanger, public al::HioNode {
public:
    PlayerModelChangerHakoniwa(const al::LiveActor* actor, PlayerModelHolder* modelHolder,
                               PlayerPainPartsKeeper* painPartsKeeper, PlayerCostumeInfo* costumeInfo,
                               const IUseDimension* dimension);
    void initStartModel();
    const char* getModelName();
    void changeModel(al::LiveActor* modelActor);
    void update(bool updateDimension, bool seOnly);
    void syncHost(bool syncVisibility);
    void syncShowHide(al::LiveActor* modelActor);
    void startDamageStopDemo();
    void updateDamageStopDemo();
    void syncHostDamageStopDemo(bool syncVisibility);
    s32 calcCostumeWarmLevel(s32 level) const;
    void resetPosition() override;
    bool isDamageStopDemo() const { return mIsBlinkingFromDamage; }

    void hideModel() override;
    void hideSilhouette() override;
    void hideShadowMask() override;
    void showModel() override;
    void showSilhouette() override;
    void showShadowMask() override;
    void syncModelBoneVisibility();
    bool isFireFlower() const override;
    bool isMini() const override;
    bool isChange() const override;
    bool is2DModel() const override;
    bool isHiddenModel() const override;
    bool isHiddenShadowMask() const override;

private:
    const al::LiveActor* mLiveActor;
    bool mIsChange;
    bool mIsMode2D;
    al::LiveActor* mLiveActor2;
    PlayerModelHolder* mPlayerModelHolder;
    PlayerPainPartsKeeper* mPlayerPainPartsKeeper;
    PlayerCostumeInfo* mPlayerCostumeInfo;
    const IUseDimension* mIUseDimension;
    bool mIsVisibilityNeedsSync;
    bool mIsModelVisible;
    bool mIsSilhouetteVisible;
    bool mIsShadowMaskVisible;
    bool mIsBlinkingFromDamage;
    s32 mDamageTimer;
    bool mIsMusicStarted;
    bool mIsNeedHairControl;
    bool mIsNeedSyncBodyHair;
    bool mIsSyncFaceBeard;
    bool mIsSyncStrap;
    bool mIsPreventHeadPain;
};

static_assert(sizeof(PlayerModelChangerHakoniwa) == 0x58);
