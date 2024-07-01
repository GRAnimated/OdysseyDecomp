#include "Sequence/GameSequenceInfo.h"
#include "Library/Base/StringUtil.h"
#include "Library/LiveActor/ActorSceneInfo.h"
#include "Library/LiveActor/LiveActor.h"
#include "System/GameDataHolder.h"
#include "System/GameDataHolderAccessor.h"

GameSequenceInfo::GameSequenceInfo() {
    init();
}

void GameSequenceInfo::init() {
    mSceneStatus = 0;
    mIsStatusBossBattle = false;
    mIsStatusInvalidSave = false;
    mIsWipeOpenEnd = true;
    mShopStatus = ShopStatus::ShopStatus_None;
}

void GameSequenceInfo::resetSceneStatus() {
    mSceneStatus = 0;
    mIsStatusBossBattle = false;
    mIsStatusInvalidSave = false;
}

void GameSequenceInfo::setSceneStatusBossBattle() {
    mSceneStatus = 1;
    mIsStatusBossBattle = true;
}

void GameSequenceInfo::setSceneStatusBossBattleEnd() {
    resetSceneStatus();
}

void GameSequenceInfo::setSceneStatusBossBattleEndForPlayerAnim() {
    mIsStatusBossBattle = false;
    mIsStatusInvalidSave = true;
}

void GameSequenceInfo::setSceneStatusInvalidSave() {
    mIsStatusInvalidSave = true;
}

bool GameSequenceInfo::isSceneStatusBossBattle() const {
    return mSceneStatus == 1;
}

bool GameSequenceInfo::isSceneStatusBossBattleForPlayerAnim() const {
    return mIsStatusBossBattle;
}

bool GameSequenceInfo::isSceneStatusInvalidSave() const {
    return mIsStatusInvalidSave;
}

bool GameSequenceInfo::isShopStatus(s32 shopStatus) const {
    return mShopStatus == shopStatus;
}

void GameSequenceInfo::setShopStatus(s32 shopStatus) {
    mShopStatus = ShopStatus(shopStatus);
}

namespace rs {
void setSceneStatusBossBattle(const al::LiveActor* actor) {
    ((GameDataHolder*)actor->getSceneInfo()->mGameDataHolder)
        ->getSequenceInfo()
        ->setSceneStatusBossBattle();
}

void setSceneStatusBossBattleEnd(const al::LiveActor* actor) {
    ((GameDataHolder*)actor->getSceneInfo()->mGameDataHolder)
        ->getSequenceInfo()
        ->setSceneStatusBossBattleEnd();
}

void setSceneStatusBossBattleEndForPlayerAnim(const al::LiveActor* actor) {
    ((GameDataHolder*)actor->getSceneInfo()->mGameDataHolder)
        ->getSequenceInfo()
        ->setSceneStatusBossBattleEndForPlayerAnim();
}

bool isSceneStatusBossBattle(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getSequenceInfo()->isSceneStatusBossBattle();
}

bool isSceneStatusBossBattleForPlayerAnim(const al::LiveActor* actor) {
    return ((GameDataHolder*)actor->getSceneInfo()->mGameDataHolder)
        ->getSequenceInfo()
        ->isSceneStatusBossBattleForPlayerAnim();
}

bool isSceneStatusInvalidSave(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getSequenceInfo()->isSceneStatusInvalidSave();
}

void setSceneStatusInvalidSave(GameDataHolderAccessor accessor) {
    accessor.getHolder()->getSequenceInfo()->setSceneStatusInvalidSave();
}

bool isInvalidSaveStage(const char* stageName) {
    if (al::isEqualString(stageName, "MoonWorldBasementStage"))
        return true;
    else
        return al::isEqualString(stageName, "SnowWorldLobby001Stage");
}

bool isWipeOpenEnd(const al::LiveActor* actor) {
    return ((GameDataHolder*)actor->getSceneInfo()->mGameDataHolder)
        ->getSequenceInfo()
        ->isWipeOpenEnd();
}

void endWipeOpen(const GameDataHolder* holder) {
    holder->getSequenceInfo()->setWipeOpenEnd(true);
}

void startWipeOpen(const GameDataHolder* holder) {
    holder->getSequenceInfo()->setWipeOpenEnd(false);
}

bool isShopStatusBuyItem(const al::LiveActor* actor) {
    return isShopStatusBuyShine(actor) || isShopStatusBuyShineMany(actor) ||
           isShopStatusBuyMaxLifeUpItem(actor);
}

bool isShopStatusBuyShine(const al::LiveActor* actor) {
    return ((GameDataHolder*)actor->getSceneInfo()->mGameDataHolder)
        ->getSequenceInfo()
        ->isShopStatus(GameSequenceInfo::ShopStatus::ShopStatus_BuyShine);
}

bool isShopStatusBuyShineMany(const al::LiveActor* actor) {
    return ((GameDataHolder*)actor->getSceneInfo()->mGameDataHolder)
        ->getSequenceInfo()
        ->isShopStatus(GameSequenceInfo::ShopStatus::ShopStatus_BuyShineMany);
}

bool isShopStatusBuyMaxLifeUpItem(const al::LiveActor* actor) {
    return ((GameDataHolder*)actor->getSceneInfo()->mGameDataHolder)
        ->getSequenceInfo()
        ->isShopStatus(GameSequenceInfo::ShopStatus::ShopStatus_BuyMaxLifeUpItem);
}

void setShopStatusBuyShine(const al::LiveActor* actor) {
    ((GameDataHolder*)actor->getSceneInfo()->mGameDataHolder)
        ->getSequenceInfo()
        ->setShopStatus(GameSequenceInfo::ShopStatus::ShopStatus_BuyShine);
}

void setShopStatusBuyShineMany(const al::LiveActor* actor) {
    ((GameDataHolder*)actor->getSceneInfo()->mGameDataHolder)
        ->getSequenceInfo()
        ->setShopStatus(GameSequenceInfo::ShopStatus::ShopStatus_BuyShineMany);
}

void setShopStatusBuyMaxLifeUpItem(const al::LiveActor* actor) {
    ((GameDataHolder*)actor->getSceneInfo()->mGameDataHolder)
        ->getSequenceInfo()
        ->setShopStatus(GameSequenceInfo::ShopStatus::ShopStatus_BuyMaxLifeUpItem);
}

void setShopStatusNone(const al::LiveActor* actor) {
    ((GameDataHolder*)actor->getSceneInfo()->mGameDataHolder)
        ->getSequenceInfo()
        ->setShopStatus(GameSequenceInfo::ShopStatus::ShopStatus_None);
}

}  // namespace rs
