#pragma once

#include <basis/seadTypes.h>

#include "Player/IPlayerModelChanger.h"

namespace al {
class LiveActor;
}
class ActorDimensionKeeper;
class PlayerInput;
class PlayerModelHolder;

class PlayerModelChanger2D3D : public IPlayerModelChanger {
public:
    PlayerModelChanger2D3D(const al::LiveActor* player, const PlayerInput* input,
                           PlayerModelHolder* modelHolder,
                           const ActorDimensionKeeper* dimensionKeeper);

    void update(bool isChangeEnabled);
    const char* getModelName();
    void changeModel(al::LiveActor* modelActor);
    void updateDead();
    bool requestDamage();
    bool requestKinokoSuper();
    bool requestFireFlower();
    bool requestMini();
    bool requestDeath();
    void syncPose(const al::LiveActor* actor);
    bool isFireFlower() const override;
    bool isMini() const override;
    bool isChange() const override;
    bool is2DModel() const override;
    bool isHiddenModel() const override;
    bool isHiddenShadowMask() const override;
    void resetPosition() override;
    void hideModel() override;
    void hideSilhouette() override;
    void hideShadowMask() override;
    void showModel() override;
    void showSilhouette() override;
    void showShadowMask() override;

private:
    const al::LiveActor* mPlayer;
    const PlayerInput* mInput;
    bool mIsChange = false;
    bool mIs2DModel = false;
    bool mIsDeathRequested = false;
    u8 _1b;
    s32 mModel = 0;
    s32 mRequestedModel = 0;
    u32 _24;
    al::LiveActor* mModelActor = nullptr;
    const char* mBlinkModelName = nullptr;
    const char* mTargetModelName = nullptr;
    PlayerModelHolder* mModelHolder;
    const ActorDimensionKeeper* mDimensionKeeper;
    s32 mBlinkTimer = 0;
    s32 mChangeCooldown = 0;
};

