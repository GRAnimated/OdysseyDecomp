#include "Util/ItemUtil.h"

#include <math/seadMatrix.h>

#include "Library/Base/StringUtil.h"
#include "Library/Item/ItemUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseKeeper.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/LiveActor/LiveActorUtil.h"
#include "Library/Math/MathAngleUtil.h"
#include "Library/Math/MathRandomUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Matrix/MatrixUtil.h"
#include "Library/Model/ModelShapeUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Scene/SceneObjUtil.h"
#include "Library/Stage/StageSyncCounter.h"

#include "Item/Shine.h"
#include "Scene/SceneObjFactory.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolder.h"
#include "Util/Sensor.h"

const char* sItem2DNames[] = {
    "コイン2D[自動取得]",
    "コイン2D[自動取得]",
    "コイン2D[自動取得]",
    "コイン2D[自動取得]",
    "コイン2D[飛出し出現]",
    "コイン2D[自動取得]",
    "コイン2D[自動取得]",
    "コイン2D[自動取得]",
    "コイン2D[自動取得]",
    "コイン2D[自動取得]",
    "コイン2D[自動取得]",
    "コイン2D[自動取得]",
    "ライフアップアイテム[飛出し出現]",
    "ライフアップアイテム[逆向き飛出し出現]",
    "ライフアップアイテム2D[飛出し出現]",
    "コイン2D[自動取得]",
    "最大ライフアップアイテム2D[飛出し出現]",
};

namespace rs {

s32 getItemType(const al::ActorInitInfo& info) {
    const char* name = nullptr;
    if ((al::tryGetStringArg(&name, info, "ItemType") ||
         al::tryGetStringArg(&name, info, "ItemTypeNoShine") ||
         al::tryGetStringArg(&name, info, "ItemType2D3D")) &&
        !al::isEqualString(name, "None"))
        return getItemType(name);
    else
        return -1;
}

s32 getItemTypeFromName(const char* name) {
    if (al::isEqualString(name, "Coin"))
        return ItemType::Coin;
    if (al::isEqualString(name, "Coin2D"))
        return ItemType::Coin2D;
    if (al::isEqualString(name, "CoinBlow"))
        return ItemType::CoinBlow;
    if (al::isEqualString(name, "CoinBlowVeryLittle"))
        return ItemType::CoinBlowVeryLittle;
    if (al::isEqualString(name, "CoinPopUp"))
        return ItemType::CoinPopUp;
    if (al::isEqualString(name, "CoinPopUpWithoutHitReaction"))
        return ItemType::CoinPopUpWithoutHitReaction;
    if (al::isEqualString(name, "Coin3"))
        return ItemType::Coin3;
    if (al::isEqualString(name, "Coin5"))
        return ItemType::Coin5;
    if (al::isEqualString(name, "Coin10"))
        return ItemType::Coin10;
    if (al::isEqualString(name, "Coin10Auto"))
        return ItemType::Coin10Auto;
    if (al::isEqualString(name, "Coin100"))
        return ItemType::Coin100;
    if (al::isEqualString(name, "Coin5Count"))
        return ItemType::Coin5Count;
    if (al::isEqualString(name, "LifeUpItem"))
        return ItemType::LifeUpItem;
    if (al::isEqualString(name, "LifeUpItemBack"))
        return ItemType::LifeUpItemBack;
    if (al::isEqualString(name, "LifeUpItem2D"))
        return ItemType::LifeUpItem2D;
    if (al::isEqualString(name, "LifeMaxUpItem"))
        return ItemType::LifeMaxUpItem;
    if (al::isEqualString(name, "LifeMaxUpItem2D"))
        return ItemType::LifeMaxUpItem2D;
    if (al::isEqualString(name, "Shine"))
        return ItemType::Shine;
    if (al::isEqualString(name, "AirBubble"))
        return ItemType::AirBubble;
    if (al::isEqualString(name, "DotMarioCat"))
        return ItemType::DotMarioCat;
    if (al::isEqualString(name, "KuriboMini3"))
        return ItemType::KuriboMini3;
    if (al::isEqualString(name, "KuriboMini8"))
        return ItemType::KuriboMini8;
    if (al::isEqualString(name, "CoinStackBound"))
        return ItemType::CoinStackBound;
    if (al::isEqualString(name, "Random"))
        return ItemType::Random;
    return -1;
}

s32 getItemType(const char* name) {
    return getItemTypeFromName(name);
}

bool isItemTypeKuriboMini(s32* out, s32 type) {
    if (type == ItemType::KuriboMini3) {
        *out = ItemType::CoinBlowVeryLittle;
        return true;
    } else if (type == ItemType::KuriboMini8) {
        *out = ItemType::Coin10;
        return true;
    }
    return false;
}

bool tryInitItemAndAddToKeeper(al::LiveActor* actor, s32 itemType, const al::ActorInitInfo& info,
                               bool a4) {
    if (itemType != ItemType::Random) {
        actor->initItemKeeper(1);
        switch (itemType) {
        case ItemType::Coin:
        case ItemType::Coin10:
        case ItemType::Coin100:
            al::addItem(actor, info, "コイン[自動取得]", 0);
            return true;
        case ItemType::CoinBlow:
            al::addItem(actor, info, "コイン[放出]", 0);
            return true;
        case ItemType::CoinBlowVeryLittle:
            al::addItem(actor, info, "コイン[放出・極小]", 0);
            return true;
        case ItemType::CoinPopUp:
            al::addItem(actor, info, "コイン[飛出し出現]", 0);
            return true;
        case ItemType::CoinPopUpWithoutHitReaction:
            al::addItem(actor, info, "コイン[飛出し出現・出現音無し]", 0);
            return true;
        case ItemType::Coin3:
            al::addItem(actor, info, "コインx3[自動取得]", 0);
            return true;
        case ItemType::Coin5:
            al::addItem(actor, info, "コインx5[自動取得]", 0);
            return true;
        case ItemType::Coin10Auto:
            al::addItem(actor, info, "コインx10[自動取得]", 0);
            return true;
        case ItemType::Coin5Count:
            al::addItem(actor, info, "コイン[自動取得5枚]", 0);
            return true;
        case ItemType::LifeUpItem:
            if (a4)
                al::addItem(actor, info, "ライフアップアイテム[真上出現]", 0);
            else
                al::addItem(actor, info, "ライフアップアイテム[飛出し出現]", 0);
            return true;
        case ItemType::LifeUpItemBack:
            al::addItem(actor, info, "ライフアップアイテム[逆向き飛出し出現]", 0);
            return true;
        case ItemType::LifeMaxUpItem:
            if (a4)
                al::addItem(actor, info, "最大ライフアップアイテム[真上出現]", 0);
            else
                al::addItem(actor, info, "最大ライフアップアイテム[飛出し出現]", 0);
            return true;
        case ItemType::AirBubble:
            al::addItem(actor, info, "空気泡", 0);
            return true;
        case ItemType::DotMarioCat:
            if (al::isPercentProbability(10.0))
                al::addItem(actor, info, "ドットキャラクター(レア)", 0);
            return true;
        case ItemType::CoinStackBound:
            al::addItem(actor, info, "跳ねる積みコイン", 0);
            return true;
        default:
            return false;
        }
    }
    actor->initItemKeeper(2);
    al::addItem(actor, info, "ライフアップアイテム[飛出し出現]", "ライフアップ", nullptr, -1, 0);
    al::addItem(actor, info, "コイン[自動取得]", "コイン", nullptr, -1, 0);
    return true;
}

bool initItemByPlacementInfo(al::LiveActor* actor, const al::ActorInitInfo& info, bool a3) {
    return rs::tryInitItemByPlacementInfo(actor, info, a3);
}

bool tryInitItemByPlacementInfo(al::LiveActor* actor, const al::ActorInitInfo& info, bool a3) {
    const char* itemType = nullptr;
    if ((al::tryGetStringArg(&itemType, info, "ItemType") ||
         al::tryGetStringArg(&itemType, info, "ItemTypeNoShine") ||
         al::tryGetStringArg(&itemType, info, "ItemType2D3D")) &&
        !al::isEqualString(itemType, "None")) {
        s32 type = getItemType(itemType);
        if (type != -1 && type != ItemType::Shine)
            return tryInitItemAndAddToKeeper(actor, type, info, a3);
    }
    return false;
}

bool initItem2DByPlacementInfo(al::LiveActor* actor, const al::ActorInitInfo& info) {
    return tryInitItem2DByPlacementInfo(actor, info);
}

bool tryInitItem2DByPlacementInfo(al::LiveActor* actor, const al::ActorInitInfo& info) {
    const char* itemType = nullptr;
    if ((!al::tryGetStringArg(&itemType, info, "ItemType") &&
         !al::tryGetStringArg(&itemType, info, "ItemTypeNoShine") &&
         !al::tryGetStringArg(&itemType, info, "ItemType2D3D")) ||
        al::isEqualString(itemType, "None"))
        return false;

    s32 type = getItemType(itemType);
    if (type == -1 || type == ItemType::Shine)
        return false;

    if (type == ItemType::Random) {
        actor->initItemKeeper(2);
        al::addItem(actor, info, "ライフアップアイテム[飛出し出現]", "ライフアップ", nullptr, -1,
                    0);
        al::addItem(actor, info, "コイン2D[自動取得]", "コイン", nullptr, -1, 0);
        return true;
    }
    actor->initItemKeeper(1);
    if (type < ItemType::LifeMaxUpItem2D && (0x17111 >> type)) {
        al::addItem(actor, info, sItem2DNames[type], 0);
        return true;
    }
    return false;
}

bool tryInitItem(al::LiveActor* actor, s32 itemType, const al::ActorInitInfo& info, bool a4) {
    return itemType != -1 && itemType != ItemType::Shine &&
           tryInitItemAndAddToKeeper(actor, itemType, info, a4);
}

Shine* tryInitShineByPlacementInfoWithItemMenu(const al::ActorInitInfo& info) {
    const char* itemType = nullptr;
    if ((!al::tryGetStringArg(&itemType, info, "ItemType") &&
         !al::tryGetStringArg(&itemType, info, "ItemTypeNoShine") &&
         !al::tryGetStringArg(&itemType, info, "ItemType2D3D")) ||
        al::isEqualString(itemType, "None") || getItemType(itemType) != ItemType::Shine)
        return nullptr;

    return initShineByPlacementInfo(info);
}

Shine* initShineByPlacementInfo(const al::ActorInitInfo& info) {
    Shine* shine = new Shine("シャイン");
    al::initCreateActorWithPlacementInfo(shine, info);
    shine->makeActorDead();

    return shine;
}

Shine* tryInitLinkShine(const al::ActorInitInfo& info, const char* name, s32 linkIndex) {
    if (al::isExistLinkChild(info, name, linkIndex)) {
        Shine* shine = new Shine("シャイン");
        al::initLinksActor(shine, info, name, linkIndex);
        if (al::isEqualString(name, ShineFunction::getMovePointLinkName()))
            shine->initAppearDemoFromHost(info, al::getTrans(shine));
        shine->makeActorDead();
        return shine;
    }
    return nullptr;
}

Shine* tryInitLinkShineHintPhoto(const al::ActorInitInfo& info, const char* name, s32 linkIndex) {
    if (al::isExistLinkChild(info, name, linkIndex)) {
        Shine* shine = new Shine("シャイン");
        shine->setHintPhotoShine(info);
        al::initLinksActor(shine, info, name, linkIndex);
        shine->makeActorDead();
        return shine;
    }
    return nullptr;
}

Shine* initLinkShine(const al::ActorInitInfo& info, const char* name, s32 linkIndex) {
    return tryInitLinkShine(info, name, linkIndex);
}

Shine* initLinkShopShine(const al::ActorInitInfo& info, const char* name) {
    if (al::isExistLinkChild(info, name, 0)) {
        Shine* shine = new Shine("シャイン");
        shine->setShopShine();
        al::initLinksActor(shine, info, name, 0);
        if (al::isEqualString(name, ShineFunction::getMovePointLinkName()))
            shine->initAppearDemoFromHost(info, al::getTrans(shine));
        shine->makeActorDead();
        return shine;
    }
    return nullptr;
}

Shine* initLinkShineChipShine(const al::ActorInitInfo& info) {
    const char* str;
    if (al::isExistLinkChild(info, "ShineActor", 0))
        str = "ShineActor";
    else if (al::isExistLinkChild(info, "ShineDotActor", 0))
        str = "ShineDotActor";
    else if (al::isExistLinkChild(info, "ShineActorWithCamera", 0))
        str = "ShineActorWithCamera";
    else
        return nullptr;

    Shine* shine = new Shine("シャイン(シャインチップ)");
    al::initLinksActor(shine, info, str, 0);
    shine->makeActorDead();
    return shine;
}

Shine* initLinkBossShine(const al::ActorInitInfo& info) {
    return tryInitLinkShine(info, "ShineActor", 0);
}

void createShineEffectInsideObject(Shine* shine, const al::LiveActor* actor,
                                   const al::ActorInitInfo& info) {
    const char* str = actor->getName();
    al::getObjectName(&str, info);

    if (al::isExistModel(actor))
        str = al::getModelName(actor);

    shine->createShineEffectInsideObject(info, al::getTrans(actor), str);
}

void appearPopupShine(Shine* shine) {
    shine->appearPopup();
}

void appearPopupShine(Shine* shine, const al::LiveActor* actor) {
    sead::Vector3f trans = al::getTrans(actor);
    shine->appearPopup(trans);
}

void appearPopupShine(Shine* shine, const sead::Vector3f& trans) {
    shine->appearPopup(trans);
}

void appearPopupShineWithoutDemo(Shine* shine) {
    shine->appearPopupWithoutDemo();
}

void appearPopupShineWithoutDemo(Shine* shine, const al::LiveActor* actor) {
    sead::Vector3f trans = al::getTrans(actor);
    trans.y += 100.0f;
    al::resetPosition(shine, trans);
    shine->appearPopupWithoutDemo();
}

void appearPopupShineWithoutDemo(Shine* shine, const sead::Vector3f& trans) {
    al::resetPosition(shine);
    shine->appearPopupWithoutDemo();
}

void appearPopupShineWithoutWarp(Shine* shine) {
    shine->appearPopupWithoutWarp();
}

void appearPopupShineGrandByBoss(Shine* shine, s32 a2) {
    shine->appearPopupGrandByBoss(a2);
}

void appearWaitShine(Shine* shine) {
    shine->appearWait();
}

void appearWaitShine(Shine* shine, const sead::Vector3f& trans) {
    shine->appearWait(trans);
}

bool isEndAppearShine(const Shine* shine) {
    return shine->isEndAppear();
}

bool isGotShine(const Shine* shine) {
    return shine->isGot();
}

bool isAliveShine(const Shine* shine) {
    return al::isAlive(shine);
}

bool isMainShine(const Shine* shine) {
    // Shine + 0x294 = mIsMainShine;
    return false;
}

void updateHintTrans(const Shine* shine, const sead::Vector3f& trans) {
    shine->updateHintTrans(trans);
}

void appearShineAndJoinBossDemo(Shine* shine, const char* name, const sead::Quatf& quat,
                                const sead::Vector3f& trans) {
    shine->appearAndJoinBossDemo(name, quat, trans);
}

void endShineBossDemo(Shine* shine) {
    shine->endBossDemo();
}

void endShineBossDemoAndStartFall(Shine* shine, f32 a2) {
    shine->endBossDemoAndStartFall(a2);
}

void setAppearItemFactorByMsg(const al::LiveActor* actor, const al::SensorMsg* msg,
                              const al::HitSensor* sensor) {
    if (rs::isMsgAttackDirect(msg))
        al::setAppearItemFactor(actor, "直接攻撃", sensor);
    else
        al::setAppearItemFactor(actor, "間接攻撃", sensor);
}

const sead::Vector3f sItemOffset = {0.0f, 120.0f, 0.0f};

void setAppearItemFactorAndOffsetByMsg(const al::LiveActor* actor, const al::SensorMsg* msg,
                                       const al::HitSensor* sensor) {
    setAppearItemFactorByMsg(actor, msg, sensor);

    sead::Quatf quat = sead::Quatf::unit;
    al::calcQuat(&quat, actor);

    sead::Vector3f offset = sItemOffset;
    al::rotateVectorQuat(&offset, quat);

    al::setAppearItemOffset(actor, offset);
}

void setAppearItemFactorAndOffsetForCombo(const al::LiveActor* actor, const al::SensorMsg* msg,
                                          const al::HitSensor* sensor, bool a4) {
    if (a4)
        al::setAppearItemFactor(actor, "スーパーコンボ", sensor);
    else
        al::setAppearItemFactor(actor, "コンボ", sensor);

    sead::Quatf quat = sead::Quatf::unit;
    al::calcQuat(&quat, actor);

    sead::Vector3f offset = sItemOffset;
    al::rotateVectorQuat(&offset, quat);

    al::setAppearItemOffset(actor, offset);
}

void appearItemFromObj(al::LiveActor* actor, const sead::Vector3f& trans, const sead::Quatf& quat,
                       f32 offset) {
    sead::Vector3f upDir = sead::Vector3f(0.0f, 0.0f, 0.0f);
    al::calcUpDir(&upDir, actor);
    al::setAppearItemOffset(actor, upDir * offset);
    al::appearItem(actor, trans, quat, nullptr);
}

void appearItemFromObj(al::LiveActor* actor, al::HitSensor* sensor, f32 offset) {
    sead::Vector3f upDir = sead::Vector3f(0.0f, 0.0f, 0.0f);
    al::calcUpDir(&upDir, actor);
    al::setAppearItemOffset(actor, upDir * offset);
    al::setAppearItemAttackerSensor(actor, sensor);
    al::appearItem(actor);
}

void appearItemFromObj(al::LiveActor* actor, al::HitSensor* sensor, const sead::Vector3f& offset) {
    sead::Vector3f localOffset;
    al::calcTransLocalOffset(&localOffset, actor, offset);
    al::setAppearItemOffset(actor, localOffset - al::getTrans(actor));
    al::setAppearItemAttackerSensor(actor, sensor);
    al::appearItem(actor);
}

void appearItemFromObjGravity(al::LiveActor* actor, al::HitSensor* sensor,
                              const sead::Vector3f& offset) {
    sead::Vector3f localOffset;
    al::calcTransLocalOffset(&localOffset, actor, offset);
    al::setAppearItemOffset(actor, localOffset - al::getTrans(actor));
    al::setAppearItemAttackerSensor(actor, sensor);

    sead::Vector3f frontDir;
    al::calcFrontDir(&frontDir, actor);

    sead::Vector3f up;
    sead::Vector3f front;
    if (al::isParallelDirection(frontDir, -al::getGravity(actor), 0.01f)) {
        up = -al::getGravity(actor);
        front = sead::Vector3f::ez;
    } else {
        up = -al::getGravity(actor);
        front = frontDir;
    }

    sead::Quatf quat;
    al::makeQuatUpFront(&quat, up, front);
    al::appearItem(actor, al::getTrans(actor), quat, nullptr);
}

// TODO: Requires RandomItemSelector
void appearItemFromObjGravity(al::LiveActor* actor, al::HitSensor* sensor, f32 offset) {}

bool tryAppearMultiCoinFromObj(al::LiveActor* actor, const sead::Vector3f& trans, s32 a3, f32 a4) {
    return tryAppearMultiCoinFromObj(actor, trans, sead::Quatf::unit, a3, a4);
}

bool tryAppearMultiCoinFromObj(al::LiveActor* actor, const sead::Vector3f& trans,
                               const sead::Quatf& quat, s32 a3, f32 a4) {
    if (a3 % 10)
        return false;

    sead::Vector3f upDir = sead::Vector3f(0.0f, 0.0f, 0.0f);
    al::calcUpDir(&upDir, actor);

    al::setAppearItemOffset(actor, upDir * a4);
    al::appearItem(actor, trans, quat, nullptr);
    return true;
}

bool tryAppearMultiCoinFromObj(al::LiveActor* actor, al::HitSensor* sensor, s32 a3, f32 a4) {
    if (a3 % 10)
        return false;

    sead::Vector3f upDir = sead::Vector3f(0.0f, 0.0f, 0.0f);
    al::calcUpDir(&upDir, actor);

    al::setAppearItemOffset(actor, upDir * a4);
    al::setAppearItemAttackerSensor(actor, sensor);
    al::appearItem(actor);
    return true;
}

bool tryAppearMultiCoinFromObj(al::LiveActor* actor, al::HitSensor* sensor, s32 a4,
                               const sead::Vector3f& offset) {
    if (a4 % 10)
        return false;

    sead::Vector3f localOffset;
    al::calcTransLocalOffset(&localOffset, actor, offset);

    al::setAppearItemOffset(actor, localOffset - al::getTrans(actor));
    al::setAppearItemAttackerSensor(actor, sensor);
    al::appearItem(actor);
    return true;
}

void syncCoin2DAnimFrame(al::LiveActor* actor, const char* name) {
    if (al::isActionPlaying(actor, name)) {
        al::StageSyncCounter* obj =
            (al::StageSyncCounter*)al::getSceneObj(actor, SceneObjID_alStageSyncCounter);
        al::setVisAnimFrameForAction(actor,
                                     obj->getCounter() % (s32)al::getVisAnimFrameMax(actor, name));
    }
}

const char* getStageCoinCollectArchiveName(const al::LiveActor* actor) {
    GameDataHolder* gameDataHolder =
        (GameDataHolder*)al::getSceneObj(actor, SceneObjID_GameDataHolder);

    return gameDataHolder->getCoinCollectArchiveName(GameDataFunction::getCurrentWorldId(actor));
}

const char* getStageCoinCollectEmptyArchiveName(const al::LiveActor* actor) {
    GameDataHolder* gameDataHolder =
        (GameDataHolder*)al::getSceneObj(actor, SceneObjID_GameDataHolder);

    return gameDataHolder->getCoinCollectEmptyArchiveName(
        GameDataFunction::getCurrentWorldId(actor));
}

const char* getStageCoinCollect2DArchiveName(const al::LiveActor* actor) {
    GameDataHolder* gameDataHolder =
        (GameDataHolder*)al::getSceneObj(actor, SceneObjID_GameDataHolder);

    return gameDataHolder->getCoinCollect2DArchiveName(GameDataFunction::getCurrentWorldId(actor));
}

const char* getStageCoinCollect2DEmptyArchiveName(const al::LiveActor* actor) {
    GameDataHolder* gameDataHolder =
        (GameDataHolder*)al::getSceneObj(actor, SceneObjID_GameDataHolder);

    return gameDataHolder->getCoinCollect2DEmptyArchiveName(
        GameDataFunction::getCurrentWorldId(actor));
}

s32 getStageShineAnimFrame(const al::LiveActor* actor) {
    GameDataHolder* gameDataHolder =
        (GameDataHolder*)al::getSceneObj(actor, SceneObjID_GameDataHolder);

    return gameDataHolder->getShineAnimFrame(GameDataFunction::getCurrentWorldId(actor));
}

s32 getStageShineAnimFrame(const al::LiveActor* actor, s32 worldId) {
    GameDataHolder* gameDataHolder =
        (GameDataHolder*)al::getSceneObj(actor, SceneObjID_GameDataHolder);

    return gameDataHolder->getShineAnimFrame(worldId);
}

s32 getStageShineAnimFrame(const al::LiveActor* actor, const char* stageName) {
    GameDataHolder* gameDataHolder =
        (GameDataHolder*)al::getSceneObj(actor, SceneObjID_GameDataHolder);

    s32 worldId = gameDataHolder->getWorldList()->tryFindWorldIndexByMainStageName(stageName);

    return gameDataHolder->getShineAnimFrame(worldId);
}

void setStageShineAnimFrame(al::LiveActor* actor, const char* stageName, s32 a3, bool a4) {}

const char* getStageShineArchiveName(al::LiveActor* actor, const char* stageName) {
    GameDataHolder* gameDataHolder =
        (GameDataHolder*)al::getSceneObj(actor, SceneObjID_GameDataHolder);

    s32 worldId = gameDataHolder->getWorldList()->tryFindWorldIndexByMainStageName(stageName);

    return worldId == GameDataFunction::getWorldIndexPeach() ? "PowerStar" : "Shine";
}

const char* getStageShineEmptyArchiveName(al::LiveActor* actor, const char* stageName) {
    GameDataHolder* gameDataHolder =
        (GameDataHolder*)al::getSceneObj(actor, SceneObjID_GameDataHolder);

    s32 worldId = gameDataHolder->getWorldList()->tryFindWorldIndexByMainStageName(stageName);

    return worldId == GameDataFunction::getWorldIndexPeach() ? "PowerStarEmpty" : "ShineEmpty";
}

void setMaterialAsEmptyModel(al::LiveActor* actor) {
    al::setModelAlphaMask(actor, 0.499f);
}

void setProjectionMtxAsEmptyModel2d(al::LiveActor* actor, const sead::Vector2f& vec) {
    sead::Matrix44f matrix = sead::Matrix44f::ident;

    sead::Vector3f upDir = sead::Vector3f::zero;
    sead::Vector3f frontDir = sead::Vector3f::zero;

    al::calcUpDir(&upDir, actor);
    al::calcFrontDir(&frontDir, actor);

    al::makeMtxProj(&matrix, vec, upDir, frontDir);

    const sead::Vector3f& trans = al::getTrans(actor);
    sead::Matrix44CalcCommon<f32>::setCol(matrix, 3,
                                          sead::Vector4f(trans.x, trans.y, trans.z, 1.0f));
    al::setModelProjMtx0(actor->getModelKeeper(), matrix);
}

}  // namespace rs
