#include "System/GameConfigData.h"

#include "Library/Layout/LayoutActor.h"
#include "Library/Scene/SceneUtil.h"
#include "Library/Yaml/ByamlUtil.h"
#include "Library/Yaml/Writer/ByamlWriter.h"
#include "System/GameDataHolder.h"
#include "System/GameDataHolderAccessor.h"

void GameConfigData::init() {
    mCameraStickSensitivityLevel = -1;
    mIsCameraReverseInputH = false;
    mIsCameraReverseInputV = false;
    mIsValidCameraGyro = true;
    mCameraGyroSensitivityLevel = -1;
    mIsUseOpenListAdditionalButton = false;
    mIsValidPadRumble = true;
    mPadRumbleLevel = 0;
}

bool GameConfigData::isCameraReverseInputH() const {
    return mIsCameraReverseInputH;
}

void GameConfigData::onCameraReverseInputH() {
    mIsCameraReverseInputH = true;
}

void GameConfigData::offCameraReverseInputH() {
    mIsCameraReverseInputH = false;
}

bool GameConfigData::isCameraReverseInputV() const {
    return mIsCameraReverseInputV;
}

void GameConfigData::onCameraReverseInputV() {
    mIsCameraReverseInputV = true;
}

void GameConfigData::offCameraReverseInputV() {
    mIsCameraReverseInputV = false;
}

s32 GameConfigData::getCameraStickSensitivityLevel() const {
    return mCameraStickSensitivityLevel;
}

void GameConfigData::setCameraStickSensitivityLevel(s32 level) {
    mCameraStickSensitivityLevel = level;
}

bool GameConfigData::isValidCameraGyro() const {
    return mIsValidCameraGyro;
}

void GameConfigData::validateCameraGyro() {
    mIsValidCameraGyro = true;
}

void GameConfigData::invalidateCameraGyro() {
    mIsValidCameraGyro = false;
}

s32 GameConfigData::getCameraGyroSensitivityLevel() const {
    return mCameraGyroSensitivityLevel;
}

void GameConfigData::setCameraGyroSensitivityLevel(s32 level) {
    mCameraGyroSensitivityLevel = level;
}

bool GameConfigData::isUseOpenListAdditionalButton() const {
    return mIsUseOpenListAdditionalButton;
}

void GameConfigData::onUseOpenListAdditionalButton() {
    mIsUseOpenListAdditionalButton = true;
}

void GameConfigData::offUseOpenListAdditionalButton() {
    mIsUseOpenListAdditionalButton = false;
}

bool GameConfigData::isValidPadRumble() const {
    return mIsValidPadRumble;
}

void GameConfigData::validatePadRumble() {
    mIsValidPadRumble = true;
}

void GameConfigData::invalidatePadRumble() {
    mIsValidPadRumble = false;
}

s32 GameConfigData::getPadRumbleLevel() const {
    return mPadRumbleLevel;
}

void GameConfigData::setPadRumbleLevel(s32 level) {
    mPadRumbleLevel = level;
}

void GameConfigData::write(al::ByamlWriter* writer) {
    writer->pushHash("GameConfigData");
    writer->addInt("CameraStickSensitivityLevel", mCameraStickSensitivityLevel);
    writer->addBool("IsCameraReverseInputH", mIsCameraReverseInputH);
    writer->addBool("IsCameraReverseInputV", mIsCameraReverseInputV);
    writer->addBool("IsValidCameraGyro", mIsValidCameraGyro);
    writer->addInt("CameraGyroSensitivityLevel", mCameraGyroSensitivityLevel);
    writer->addBool("IsUseOpenListAdditionalButton", mIsUseOpenListAdditionalButton);
    writer->addBool("IsPadRumble", mIsValidPadRumble);
    writer->addInt("PadRumbleLevel", mPadRumbleLevel);
    writer->pop();
}

void GameConfigData::read(const al::ByamlIter& conf) {
    mCameraStickSensitivityLevel = -1;
    mIsCameraReverseInputH = false;
    mIsCameraReverseInputV = false;
    mIsValidCameraGyro = true;
    mCameraGyroSensitivityLevel = -1;
    mIsUseOpenListAdditionalButton = false;
    mIsValidPadRumble = true;
    mPadRumbleLevel = 0;

    al::ByamlIter iter;
    al::tryGetByamlIterByKey(&iter, conf, "GameConfigData");
    al::tryGetByamlS32(&mCameraStickSensitivityLevel, iter, "CameraStickSensitivityLevel");
    al::tryGetByamlBool(&mIsCameraReverseInputH, iter, "IsCameraReverseInputH");
    al::tryGetByamlBool(&mIsCameraReverseInputV, iter, "IsCameraReverseInputV");
    al::tryGetByamlBool(&mIsValidCameraGyro, iter, "IsValidCameraGyro");
    al::tryGetByamlS32(&mCameraGyroSensitivityLevel, iter, "CameraGyroSensitivityLevel");
    al::tryGetByamlBool(&mIsUseOpenListAdditionalButton, iter, "IsUseOpenListAdditionalButton");
    al::tryGetByamlBool(&mIsValidPadRumble, iter, "IsPadRumble");
    al::tryGetByamlS32(&mPadRumbleLevel, iter, "PadRumbleLevel");
}

namespace rs {
GameConfigData* getGameConfigData(const al::LayoutActor* actor) {
    return GameDataHolderAccessor(actor).getHolder()->getConfigData();
}

void saveGameConfigData(const al::LayoutActor* actor) {
    return GameDataHolderAccessor(actor).getHolder()->setRequireSave();
}

void applyGameConfigData(al::Scene* scene, const GameConfigData* gameConfigData) {
    al::setCameraStickSensitivityLevel(scene, gameConfigData->getCameraStickSensitivityLevel());
    if (gameConfigData->isCameraReverseInputH())
        al::onCameraReverseInputH(scene);
    else
        al::offCameraReverseInputH(scene);

    if (gameConfigData->isCameraReverseInputV())
        al::onCameraReverseInputV(scene);
    else
        al::offCameraReverseInputV(scene);

    al::setCameraGyroSensitivityLevel(scene, gameConfigData->getCameraGyroSensitivityLevel());

    if (gameConfigData->isValidCameraGyro())
        al::validateCameraGyro(scene);
    else
        al::invalidateCameraGyro(scene);

    if (gameConfigData->isValidPadRumble())
        al::validatePadRumble(scene);
    else
        al::invalidatePadRumble(scene);

    al::setPadRumblePowerLevel(scene, gameConfigData->getPadRumbleLevel());
}

bool isUseOpenListAdditionalButton(const al::IUseSceneObjHolder* holder) {
    return GameDataHolderAccessor(holder)
        .getHolder()
        ->getConfigData()
        ->isUseOpenListAdditionalButton();
}
}  // namespace rs
