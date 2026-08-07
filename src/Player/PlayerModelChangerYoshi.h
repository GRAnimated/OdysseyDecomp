#pragma once

#include "Player/IPlayerModelChanger.h"

namespace al {
class LiveActor;
}

class PlayerModelHolder;

class PlayerModelChangerYoshi : public IPlayerModelChanger {
public:
    PlayerModelChangerYoshi(const al::LiveActor* host, PlayerModelHolder* modelHolder);

    void syncHost();
    void syncModelFlag(al::LiveActor* modelActor);
    void appearModel();
    void killModel();
    void resetPosition() override;
    void hideModel() override;
    void hideSilhouette() override;
    void showModel() override;
    void showSilhouette() override;
    bool isHiddenModel() const override;
    void changeModel(al::LiveActor* modelActor);
    bool isFireFlower() const override;
    bool isMini() const override;
    bool isChange() const override;
    bool is2DModel() const override;
    bool isHiddenShadowMask() const override;
    void hideShadowMask() override;
    void showShadowMask() override;

private:
    const al::LiveActor* mHost;
    al::LiveActor* mModelActor = nullptr;
    PlayerModelHolder* mModelHolder;
    bool mIsVisibilityNeedsSync = false;
    bool mIsModelVisible = false;
    bool mIsSilhouetteVisible = false;
};

static_assert(sizeof(PlayerModelChangerYoshi) == 0x28);
