#pragma once

#include <basis/seadTypes.h>

#include "System/ByamlSave.h"

namespace al {
class ByamlIter;
class IUseSceneObjHolder;
class LayoutActor;
class Scene;
}  // namespace al

class GameConfigData : public ByamlSave {
public:
    GameConfigData();
    void init();
    bool isCameraReverseInputH() const;
    void onCameraReverseInputH();
    void offCameraReverseInputH();
    bool isCameraReverseInputV() const;
    void onCameraReverseInputV();
    void offCameraReverseInputV();
    s32 getCameraStickSensitivityLevel() const;
    void setCameraStickSensitivityLevel(s32 level);
    bool isValidCameraGyro() const;
    void validateCameraGyro();
    void invalidateCameraGyro();
    s32 getCameraGyroSensitivityLevel() const;
    void setCameraGyroSensitivityLevel(s32 level);
    bool isUseOpenListAdditionalButton() const;
    void onUseOpenListAdditionalButton();
    void offUseOpenListAdditionalButton();
    bool isValidPadRumble() const;
    void validatePadRumble();
    void invalidatePadRumble();
    s32 getPadRumbleLevel() const;
    void setPadRumbleLevel(s32 level);
    void write(al::ByamlWriter* writer) override;
    void read(const al::ByamlIter& conf) override;

private:
    s32 mCameraStickSensitivityLevel = -1;
    bool mIsCameraReverseInputH = false;
    bool mIsCameraReverseInputV = false;
    bool mIsValidCameraGyro = true;
    s32 mCameraGyroSensitivityLevel = -1;
    bool mIsUseOpenListAdditionalButton = false;
    bool mIsValidPadRumble = true;
    s32 mPadRumbleLevel = 0;
};

namespace rs {
GameConfigData* getGameConfigData(const al::LayoutActor* actor);
void saveGameConfigData(const al::LayoutActor* actor);
void applyGameConfigData(al::Scene* scene, const GameConfigData* gameConfigData);
bool isUseOpenListAdditionalButton(const al::IUseSceneObjHolder* holder);
}  // namespace rs
