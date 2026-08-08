#include "Player/PlayerFunction.h"

#include "Library/Base/StringUtil.h"
#include "Library/Controller/InputFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Obj/ActorDitherAnimator.h"
#include "Library/Shadow/ActorShadowUtil.h"
#include "Library/Player/PlayerUtil.h"
#include "Library/Resource/ResourceFunction.h"

#include "Player/HackCap.h"
#include "Player/PlayerActorBase.h"
#include "Player/PlayerDamageKeeper.h"
#include "Player/PlayerHackKeeper.h"
#include "Player/PlayerInfo.h"
#include "Player/PlayerJudgeDead.h"
#include "Player/PlayerJudgeDeadWipeStart.h"
#include "Player/PlayerJudgeDrawForward.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/JudgeUtil.h"

namespace PlayerFunction {


void setupMarioFaceEarringVisibility(al::LiveActor* actor, const PlayerCostumeInfo* costumeInfo) {
    if (al::isExistJoint(actor, "Earrings"))
        al::setJointVisibility(actor, "Earrings", costumeInfo->isEnableEarring());
}

void setupMarioHeadStrapVisibility(al::LiveActor* actor, const PlayerCostumeInfo* costumeInfo) {
    if (costumeInfo->getHeadInfo()->isUseStrap)
        al::setJointVisibility(actor, "Strap", !costumeInfo->getBodyInfo()->isUseBeard);
}

void createCapModelName(sead::BufferedSafeStringBase<char>* modelName,
                        const char* playerModelName) {
    if (al::isEqualString(playerModelName, "MarioInvisible"))
        modelName->format("MarioCap");
    else
        modelName->format("%sCap", playerModelName);
}

void initYoshiTongueParamHolder(al::LiveActor* actor) {
    al::initActorParamHolder(
        actor, al::findOrCreateResource(sead::SafeString("ObjectData/MarioCapCommonInfo"), nullptr),
        nullptr);
}

bool isNeedHairControl(const PlayerBodyCostumeInfo* bodyInfo, const char* headName) {
    bool result = bodyInfo->isHideHeadHair;
    if (!result)
        return result;
    if (bodyInfo->isMario64) {
        result = !al::isEqualSubString(headName, "Mario64");
        return result;
    }
    return true;
}

bool isInvisibleCap(const PlayerCostumeInfo* costumeInfo) {
    return costumeInfo->getHeadInfo()->isInvisibleHead;
}

void showHairVisibility(al::LiveActor* actor) {
    if (al::isExistJoint(actor, "CapHair"))
        al::setJointVisibility(actor, "CapHair", true);
    al::setJointVisibility(actor, "Hair", true);
}

void hideHairVisibility(al::LiveActor* actor) {
    if (al::isExistJoint(actor, "CapHair"))
        al::setJointVisibility(actor, "CapHair", false);
    al::setJointVisibility(actor, "Hair", false);
}

void syncBodyHairVisibility(al::LiveActor* actor, al::LiveActor* bodyActor) {
    if (al::isJointVisibility(bodyActor, "Cap") && !al::isJointVisibility(bodyActor, "Hair")) {
        al::setJointVisibility(actor, "Hair", false);
        al::setJointVisibility(actor, "CapHair", true);
    } else {
        al::setJointVisibility(actor, "Hair", true);
        al::setJointVisibility(actor, "CapHair", false);
    }
}

void getMarioFaceNoseShrinkScale(sead::Vector3f* scale) {
    scale->set(0.75f, 1.0f, 1.0f);
}

void getMarioFaceBigEarScale(sead::Vector3f* scale) {
    scale->set(1.5f, 1.2f, 1.0f);
}

void syncMarioFaceBeardVisibility(al::LiveActor* actor, al::LiveActor* headActor) {
    al::setJointVisibility(actor, "Beard", !al::isJointVisibility(headActor, "Cap"));
}

void setupMarioFaceBeardVisibility(al::LiveActor* actor, const PlayerCostumeInfo* costumeInfo) {
    if (al::isExistJoint(actor, "Beard"))
        al::setJointVisibility(actor, "Beard", !costumeInfo->isSyncFaceBeard());
}

void syncMarioHeadStrapVisibility(al::LiveActor* actor) {
    sead::Vector3f capTrans = al::getJointMtxPtr(actor, "Cap")->getTranslation();
    sead::Vector3f jawTrans = al::getJointMtxPtr(actor, "Jaw")->getTranslation();
    bool isNear = (jawTrans - capTrans).length() < 60.0f;
    al::setJointVisibility(actor, "Strap", al::isJointVisibility(actor, "Cap") & isNear);
}

al::ActorDitherAnimator* createPlayerDitherAnimator(al::LiveActor* actor, f32 distance) {
    auto* animator = new al::ActorDitherAnimator(actor);
    animator->initSphereByProgram(distance, true);
    animator->initSubJudgeTableByProgram(true);
    animator->initSubJudgeBoundingBoxByProgram("SnapShotMode", sead::Vector3f(65.0f, 150.0f, 65.0f),
                                               sead::Vector3f(0.0f, 75.0f, 5.0f));
    return animator;
}

bool isPlayerHitPointOne(const al::LiveActor* actor) {
    return GameDataFunction::getPlayerHitPoint(GameDataHolderAccessor(actor)) == 1;
}

bool isPlayerDeadStatus(const al::LiveActor* actor) {
    auto* player = static_cast<PlayerActorBase*>(al::getPlayerActor(actor, 0));
    PlayerInfo* info = player->getPlayerInfo();
    if (info)
        return rs::isJudge(info->getJudgeDead());

    player = static_cast<PlayerActorBase*>(al::getPlayerActor(actor, 0));
    return al::isDead(player);
}

bool isPlayerDeadWipeStart(const al::LiveActor* actor) {
    auto* player = static_cast<PlayerActorBase*>(al::getPlayerActor(actor, 0));
    PlayerInfo* info = player->getPlayerInfo();
    if (info)
        return rs::isJudge(info->getJudgeDeadWipeStart());

    player = static_cast<PlayerActorBase*>(al::getPlayerActor(actor, 0));
    return al::isDead(player);
}

bool isPlayerDeadEnableCoinAppear(const al::LiveActor* actor) {
    auto* player = static_cast<PlayerActorBase*>(al::getPlayerActor(actor, 0));
    PlayerInfo* info = player->getPlayerInfo();
    return !info || info->getJudgeDeadWipeStart()->isDeadEnableCoinAppear();
}

void getPlayerDeadWipeInfo(const al::LiveActor* actor, const char** name, s32* wait) {
    auto* player = static_cast<PlayerActorBase*>(al::getPlayerActor(actor, 0));
    PlayerInfo* info = player->getPlayerInfo();
    if (info) {
        info->getJudgeDeadWipeStart()->getWipeInfo(name, wait);
        return;
    }
    *name = "WipeMiss";
    *wait = 0;
}

bool isPlayerDeadDrawForward(const al::LiveActor* actor) {
    auto* player = static_cast<PlayerActorBase*>(al::getPlayerActor(actor, 0));
    PlayerInfo* info = player->getPlayerInfo();
    if (info)
        return rs::isJudge(info->getJudgeDrawForward());

    player = static_cast<PlayerActorBase*>(al::getPlayerActor(actor, 0));
    return al::isDead(player);
}

u32 getPlayerInputPort(const al::LiveActor* actor) {
    return static_cast<const PlayerActorBase*>(actor)->getPortNo();
}

const sead::Matrix34f* getPlayerViewMtx(const al::LiveActor* actor) {
    return static_cast<const PlayerActorBase*>(actor)->getViewMtx();
}

void calcPlayerInputVec(sead::Vector3f* inputVec, const al::LiveActor* actor) {
    inputVec->set(sead::Vector3f::zero);
    const sead::Vector2f& input = al::getLeftStick(getPlayerInputPort(actor));
    al::calcVecViewInput(inputVec, input, -al::getGravity(actor), getPlayerViewMtx(actor));
}

bool tryActivateAmiiboPreventDamage(const al::LiveActor* actor) {
    auto* player = static_cast<PlayerActorBase*>(al::getPlayerActor(actor, 0));
    PlayerInfo* info = player->getPlayerInfo();
    if (!info)
        return false;

    PlayerDamageKeeper* damageKeeper = info->getDamageKeeper();
    if (!damageKeeper || damageKeeper->isPreventDamage())
        return false;

    damageKeeper->activatePreventDamage();
    if (info->getHackKeeper()->getHackSensor()) {
        HackCap* cap = info->getHackCap();
        if (cap)
            cap->activateInvincibleEffect();
    }
    return true;
}

const char* getPlayerDepthGroundShadowName() {
    return "DepthGround";
}

void changeDepthShadowMapSizeHigh(al::LiveActor* actor) {
    al::setDepthShadowMapSize(actor, 0x100, 0x100, "DepthDirectional");
}

void changeDepthShadowMapSizeNormal(al::LiveActor* actor) {
    al::setDepthShadowMapSize(actor, 0x100, 0x100, "DepthDirectional");
}

}  // namespace PlayerFunction
