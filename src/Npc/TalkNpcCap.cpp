#include "Npc/TalkNpcCap.h"

#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>

#include "Library/Base/StringUtil.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorResourceFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Matrix/MatrixUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Yaml/ByamlIter.h"
#include "Library/Yaml/ByamlUtil.h"

static const CapTypeData sShibakenCapData = {"ShibakenCap", "ObjectData/ShibakenCap",
                                             u8"カウボーイ帽"};
static const CapTypeData sShoppingCapData = {"ShoppingCap", "ObjectData/ShoppingCap",
                                             u8"ショップ店員帽子"};

static CapTypeData sCapTypeTable[] = {
    {"PonchoCap", "ObjectData/MarioPonchoCap", u8"ポンチョ帽子"},
    {"GunmanCap", "ObjectData/MarioGunmanCap", u8"ガンマン帽子"},
    {"AlohaCap", "ObjectData/MarioAlohaCap", u8"アロハ帽子"},
    {"CookCap", "ObjectData/MarioCookCap", u8"コック帽子"},
    {"ExplorerCap", "ObjectData/MarioExplorerCap", u8"探検家帽子"},
    {"KingCap", "ObjectData/MarioKingCap", u8"王冠"},
    {"MechanicCap", "ObjectData/MarioMechanicCap", u8"メカニック帽子"},
    {"PainterCap", "ObjectData/MarioPainterCap", u8"画家帽子"},
    {"SailorCap", "ObjectData/MarioSailorCap", u8"水兵帽子"},
    {"SantaCap", "ObjectData/MarioSantaCap", u8"サンタ帽子"},
    {"VeilCap", "ObjectData/EnemyCapVeil", u8"ヴェール"},
    {"TailCoatCap", "ObjectData/MarioTailCoatCap", u8"紳士帽子"},
    {"TuxedoCap", "ObjectData/MarioTuxedoCap", u8"タキシード帽子"},
    {"Headphone", "ObjectData/Headphone", u8"ヘッドフォン"},
    {"Clown", "ObjectData/MarioClownCap", u8"ピエロ帽子"},
    {"Suit", "ObjectData/MarioSuitCap", u8"スーツ帽子"},
    {"ClownHair", "ObjectData/MarioClownHead", u8"ピエロアフロ"},
    {"SunVisor", "ObjectData/EnemyCapKiller", u8"サンバイザー"},
    {"ScientistCap", "ObjectData/MarioScientistCap", u8"博士帽子"},
    {"SnowSuitCap", "ObjectData/MarioSnowSuitCap", u8"防寒着帽子"},
    {"RacerCap", "ObjectData/MarioRacerCap", u8"レーサー帽子"},
    {"RaceManCap", "ObjectData/RaceManCap", u8"レースノコノコ帽子"},
    {"SpaceSuitCap", "ObjectData/MarioSpaceSuitCap", u8"宇宙服帽子"},
    {"VeilCap", "ObjectData/EnemyCapVeil", u8"ヴェール"},
    {"ArmorCap", "ObjectData/MarioArmorCap", u8"武者帽子"},
    {"MakerCap", "ObjectData/MarioMakerCap", u8"ビルダー帽子"},
    {"MarioCap", "ObjectData/MarioCap", u8"マリオ帽子"},
    {"PilotCap", "ObjectData/MarioPilotCap", u8"パイロット帽子"},
    {"SwimwearCap", "ObjectData/MarioSwimwearCap", u8"水泳帽子"},
    {"HappiCap", "ObjectData/MarioHappiCap", u8"法被帽子"},
    {"GolfCap", "ObjectData/MarioGolfCap", u8"ゴルフ帽子"},
    {"CowboyCap", "ObjectData/EnemyCapCowboy", u8"カウボーイ帽子"},
    {"64Cap", "ObjectData/Mario64Cap", u8"64帽子"},
    {"CaptainCap", "ObjectData/MarioCaptainCap", u8"海賊帽子"},
    {"ForestManCap", "ObjectData/ForestManCap", u8"森の民帽子"},
    sShoppingCapData,
};

TalkNpcCap::TalkNpcCap(const CapTypeData* capInfo)
    : al::LiveActor(capInfo->japaneseName), mCapInfo(capInfo) {}

TalkNpcCap* TalkNpcCap::tryCreate(const al::LiveActor* parentActor,
                                  const al::ActorInitInfo& initInfo) {
    s32 capType = -1;
    TalkNpcCap* cap = nullptr;
    if (al::tryGetArg(&capType, initInfo, "CapType") && (u32)capType <= 35) {
        const CapTypeData* capInfo = &sCapTypeTable[capType];
        cap = new TalkNpcCap(capInfo);
        al::initCreateActorNoPlacementInfo(cap, initInfo);
        cap->initAttach(parentActor);
    }
    return cap;
}

void TalkNpcCap::initAttach(const al::LiveActor* parentActor) {
    mBaseMtx = parentActor->getBaseMtx();
    if (al::isExistModelResourceYaml(parentActor, "InitPartsFixInfo", mCapInfo->name)) {
        const u8* res = al::getModelResourceYaml(parentActor, "InitPartsFixInfo", mCapInfo->name);
        al::ByamlIter iter(res);
        const char* jointName = nullptr;
        if (al::tryGetByamlString(&jointName, iter, "JointName") && jointName)
            mBaseMtx = al::getJointMtxPtr(parentActor, jointName);
        al::tryGetByamlV3f(&mLocalRotate, iter, "LocalRotate");
        al::tryGetByamlV3f(&mLocalTrans, iter, "LocalTrans");
        sead::Vector3f localScale = {1.0f, 1.0f, 1.0f};
        al::tryGetByamlV3f(&localScale, iter, "LocalScale");
        mScale = localScale.x;
    }
    makeActorAlive();
}

TalkNpcCap* TalkNpcCap::createForAchievementNpc(const al::LiveActor* parentActor,
                                                const al::ActorInitInfo& initInfo) {
    TalkNpcCap* cap = new TalkNpcCap(&sCapTypeTable[4]);
    al::initCreateActorNoPlacementInfo(cap, initInfo);
    cap->initAttach(parentActor);
    return cap;
}

TalkNpcCap* TalkNpcCap::createForHintNpc(const al::LiveActor* parentActor,
                                         const al::ActorInitInfo& initInfo) {
    TalkNpcCap* cap = new TalkNpcCap(&sCapTypeTable[4]);
    al::initCreateActorNoPlacementInfo(cap, initInfo);
    cap->initAttach(parentActor);
    return cap;
}

TalkNpcCap* TalkNpcCap::createForShibaken(const al::LiveActor* parentActor,
                                          const al::ActorInitInfo& initInfo) {
    s32 capType = -1;
    TalkNpcCap* cap = nullptr;
    if (al::tryGetArg(&capType, initInfo, "CapType") && (u32)capType <= 35) {
        const CapTypeData* capInfo;
        if (al::isEqualString(sCapTypeTable[capType].name, "CowboyCap"))
            capInfo = &sShibakenCapData;
        else
            capInfo = &sCapTypeTable[capType];
        cap = new TalkNpcCap(capInfo);
        al::initCreateActorNoPlacementInfo(cap, initInfo);
        cap->initAttach(parentActor);
    }
    return cap;
}

TalkNpcCap* TalkNpcCap::createForShoppingNpc(const al::LiveActor* parentActor,
                                             const al::ActorInitInfo& initInfo) {
    TalkNpcCap* cap = new TalkNpcCap(&sShoppingCapData);
    al::initCreateActorNoPlacementInfo(cap, initInfo);
    al::initActorActionKeeper(cap, initInfo, "ObjectData/ShoppingCap", nullptr);
    cap->initAttach(parentActor);
    return cap;
}

TalkNpcCap* TalkNpcCap::createForShoppingNpcChromakey(const al::LiveActor* parentActor,
                                                      const al::ActorInitInfo& initInfo) {
    TalkNpcCap* cap = new TalkNpcCap(&sShoppingCapData);
    cap->mIsChromakey = true;
    al::initCreateActorNoPlacementInfo(cap, initInfo);
    al::initActorActionKeeper(cap, initInfo, "ObjectData/ShoppingCap", nullptr);
    cap->initAttach(parentActor);
    return cap;
}

TalkNpcCap* TalkNpcCap::createForVolleyballNpc(const al::LiveActor* parentActor,
                                               const al::ActorInitInfo& initInfo) {
    TalkNpcCap* cap = new TalkNpcCap(&sCapTypeTable[2]);
    al::initCreateActorNoPlacementInfo(cap, initInfo);
    cap->initAttach(parentActor);
    return cap;
}

__attribute__((noinline)) static void updateCapTransform(TalkNpcCap* cap,
                                                         const sead::Matrix34f* baseMtx,
                                                         const sead::Vector3f* localTrans,
                                                         const sead::Vector3f* localRotate,
                                                         bool isShoppingCap, f32 scale) {
    sead::Quatf baseQuat = sead::Quatf::unit;
    baseMtx->toQuat(baseQuat);

    sead::Quatf localQuat;
    localQuat.setRPY(sead::Mathf::deg2rad(localRotate->x), sead::Mathf::deg2rad(localRotate->y),
                     sead::Mathf::deg2rad(localRotate->z));

    sead::Quatf combinedQuat = baseQuat * localQuat;

    sead::Vector3f rotatedTrans;
    rotatedTrans.setRotated(combinedQuat, *localTrans);
    rotatedTrans.x += baseMtx->m[0][3];
    rotatedTrans.y += baseMtx->m[1][3];
    rotatedTrans.z += baseMtx->m[2][3];

    sead::Vector3f mtxScale = {1.0f, 1.0f, 1.0f};
    al::calcMtxScale(&mtxScale, *baseMtx);

    if (isShoppingCap) {
        mtxScale.x *= mtxScale.x;
        mtxScale.y *= mtxScale.y;
        mtxScale.z *= mtxScale.z;
    }

    sead::Vector3f minScale = {0.1f, 0.1f, 0.1f};
    al::clampV3f(&mtxScale, minScale, mtxScale);

    sead::Vector3f finalScale;
    finalScale.x = mtxScale.x * scale;
    finalScale.y = mtxScale.y * scale;
    finalScale.z = mtxScale.z * scale;

    al::setScale(cap, finalScale);
    al::resetQuatPosition(cap, combinedQuat, rotatedTrans);
}

// NON_MATCHING: compiler code-factoring merges shared body into makeActorAlive and control
// tail-calls into it; cannot reproduce this optimization in source
void TalkNpcCap::makeActorAlive() {
    al::LiveActor::makeActorAlive();
    updateCapTransform(this, mBaseMtx, &mLocalTrans, &mLocalRotate, mCapInfo == &sShoppingCapData,
                       mScale);
}

void TalkNpcCap::control() {
    updateCapTransform(this, mBaseMtx, &mLocalTrans, &mLocalRotate, mCapInfo == &sShoppingCapData,
                       mScale);
}

// NON_MATCHING: regswaps + different conditional codegen for boolean if-else chain
void TalkNpcCap::init(const al::ActorInitInfo& initInfo) {
    al::initActorSceneInfo(this, initInfo);
    al::initActorPoseTQSV(this);
    al::initActorSRT(this, initInfo);
    al::initActorModelKeeper(this, initInfo, mCapInfo->objectDataPath, 1, nullptr);
    al::initActorClipping(this, initInfo);
    al::invalidateClipping(this);

    bool isChromakey = mIsChromakey;
    al::initExecutorUpdate(this, initInfo, u8"ＮＰＣ装飾");

    if (isChromakey) {
        al::initExecutorDraw(this, initInfo, u8"ＮＰＣ[クロマキー]");
        al::initExecutorDraw(this, initInfo, u8"Ｚプリパス[ＮＰＣクロマキー]");
    } else {
        al::initExecutorDraw(this, initInfo, u8"Ｚプリパス[ディザ]");

        bool hasDeferred;
        bool hasSemiTransp;
        bool hasForward;
        bool hasIndirect;
        al::getModelDrawCategoryFromShaderAssign(&hasDeferred, &hasSemiTransp, &hasForward,
                                                 &hasIndirect, this);

        if (hasDeferred && !hasForward && !hasSemiTransp && !hasIndirect) {
            al::initExecutorDraw(this, initInfo, u8"ＮＰＣ");
        } else if (!hasDeferred && hasSemiTransp && !(hasIndirect | hasForward)) {
            al::initExecutorDraw(this, initInfo, u8"ＮＰＣ[ディファード半透明]");
        } else if (hasForward && !(hasIndirect | (hasSemiTransp | hasDeferred))) {
            al::initExecutorDraw(this, initInfo, u8"ＮＰＣ[フォワード]");
        } else if (!((hasSemiTransp | hasDeferred) | hasForward) && hasIndirect) {
            al::initExecutorDraw(this, initInfo, u8"ＮＰＣ[インダイレクト]");
        } else {
            if (hasDeferred)
                al::initExecutorDraw(this, initInfo, u8"ＮＰＣ[ディファードのみ]");
            if (hasSemiTransp)
                al::initExecutorDraw(this, initInfo, u8"ＮＰＣ[ディファード半透明のみ]");
            if (hasForward)
                al::initExecutorDraw(this, initInfo, u8"ＮＰＣ[フォワードのみ]");
            if (hasIndirect)
                al::initExecutorDraw(this, initInfo, u8"ＮＰＣ[インダイレクトのみ]");
        }
    }

    al::initExecutorModelUpdate(this, initInfo);
    al::initActorMaterialCategory(this, initInfo, "Obj");
}
