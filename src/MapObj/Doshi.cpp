#include "MapObj/Doshi.h"
#include "Library/Base/StringUtil.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Fluid/JointRippleGenerator.h"
#include "Library/Item/ItemUtil.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/Joint/JointLookAtController.h"
#include "Library/Joint/JointSpringControllerHolder.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/SubActorKeeper.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Obj/BgmPlayObj.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementInfo.h"
#include "Library/Player/PlayerUtil.h"
#include "Library/Rail/RailUtil.h"
#include "Library/Stage/StageSwitchKeeper.h"
#include "Library/Thread/FunctorV0M.h"
#include "MapObj/DoshiStateWanderBossBattle.h"
#include "MapObj/ShoppingWatcher.h"
#include "Npc/TalkNpcCap.h"
#include "System/GameDataFunction.h"
#include "System/GameDataUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/SensorMsgFunction.h"
#include "math/seadMatrix.h"

namespace {
NERVE_IMPL(Doshi, Run);
NERVE_IMPL(Doshi, Wait);
NERVE_IMPL(Doshi, BossBattle);
NERVE_IMPL(Doshi, Reaction);

NERVES_MAKE_NOSTRUCT(Doshi, Reaction);
NERVES_MAKE_STRUCT(Doshi, Run, Wait, BossBattle);

}  // namespace

static const char* sColorNames[] = {"Blue", "Yellow", "Purple"};

Doshi::Doshi(const char* name) : al::LiveActor(name) {}

void Doshi::init(const al::ActorInitInfo& info) {
    using DoshiFunctor = al::FunctorV0M<Doshi*, void (Doshi::*)()>;

    if (al::isExistLinkChild(info, "ShopNpc", 0))
        al::initActorSuffix(this, info, "Shop");
    else
        al::initActor(this, info);

    mActiveNerve = &NrvDoshi.Run;
    if (al::isExistRail(this)) {
        al::setSyncRailToStart(this);
        al::turnToRailDir(this, 360.0f);
        movement();
    } else {
        mActiveNerve = &NrvDoshi.Wait;
    }

    al::initNerve(this, mActiveNerve, 1);
    mWanderBossBattle = new DoshiStateWanderBossBattle(this);

    al::initNerveState(this, mWanderBossBattle, &NrvDoshi.BossBattle, "ボスバトル");
    al::setClippingInfo(this, mClippingRadius, &mTrans);

    al::calcTransLocalOffset(&mTrans, this, sead::Vector3f(0.0f, -mLocalOffsetY, 0.0f));

    s32 color = 0;
    al::tryGetArg(&color, info, "Color");
    const char* colorName;
    if ((u32)color <= 2)
        colorName = sColorNames[color];
    else
        colorName = "Blue";

    al::startMtpAnim(this, colorName);
    al::startVisAnim(this, colorName);

    Camera* camera = new Camera;
    camera->isActive = true;
    camera->ticket = al::initObjectCamera(this, info, "Doshi", nullptr);
    mCamera = camera;

    if (al::isExistLinkChild(info, "ShopNpc", 0)) {
        mShoppingWatcher = new ShoppingWatcher("ショップNPC(ドッシー)", "Doshi", this);
        al::PlacementInfo placementInfo;
        al::getLinksInfoByIndex(&placementInfo, info, "ShopNpc", 0);
        al::initCreateActorWithPlacementInfo(mShoppingWatcher, info, placementInfo);
        al::registerSubActorSyncClipping(this, mShoppingWatcher);
        mTalkNpcCap = TalkNpcCap::createForShoppingNpc(this, info);
        al::registerSubActor(this, mTalkNpcCap);
        al::onSyncClippingSubActor(this, mTalkNpcCap);
        al::onSyncAppearSubActor(this, mTalkNpcCap);
        al::onSyncHideSubActor(this, mTalkNpcCap);
        al::onSyncAlphaMaskSubActor(this, mTalkNpcCap);
        al::JointSpringControllerHolder::tryCreateAndInitJointControllerKeeper(
            mTalkNpcCap, "InitJointSpringCtrlDoshi");

        al::startMtpAnimAndSetFrameAndStop(mTalkNpcCap, "Color",
                                           mShoppingWatcher->get_1C0() ? 0.0f : 1.0f);
        mShoppingWatcher->makeActorAlive();
        al::onSyncAlphaMaskSubActor(this, mShoppingWatcher);
        if (mTalkNpcCap)
            mTalkNpcCap->makeActorAlive();
    }

    if (al::isExistLinkChild(info, "BgmPlayObj", 0)) {
        mBgmPlayObj = new al::BgmPlayObj("BGM再生オブジェ", 0);
        al::PlacementInfo placementInfo;
        al::getLinksInfoByIndex(&placementInfo, info, "BgmPlayObj", 0);
        al::initCreateActorWithPlacementInfo(mBgmPlayObj, info, placementInfo);
        al::registerSubActorSyncClipping(this, mBgmPlayObj);
        mBgmPlayObj->appear();
    }
    al::initJointControllerKeeper(this, 1);
    mJointLookAtController = al::initJointLookAtController(this, 1);
    mJointLookAtController->set_52(true);
    al::appendJointLookAtController(mJointLookAtController, this, "Head", 0.03f, {-45.0f, 45.0f},
                                    {-30.0f, 30.0f}, sead::Vector3f::ex, sead::Vector3f::ey);

    if (!rs::isSequenceTimeBalloonOrRace(this)) {
        mEventFlowExecutor = rs::initEventFlow(this, info, nullptr, nullptr);
        rs::startEventFlow(mEventFlowExecutor, "talk");
    }

    mJointRippleGeneratorNeck = new al::JointRippleGenerator(this);
    mJointRippleGeneratorSpine = new al::JointRippleGenerator(this);
    mJointRippleGeneratorTail = new al::JointRippleGenerator(this);

    mJointRippleGeneratorNeck->setData("Neck3", {0.0f, -80.0f, 0.0f}, 0.0f, 100.0f, 2.0f, 10.0f);
    mJointRippleGeneratorSpine->setData("Spine", {370.0f, 100.0f, 0.0f}, 0.12f, 150.0f, 1.3f,
                                        10.0f);
    mJointRippleGeneratorTail->setData("Tail3", {50.0f, 0.0f, 0.0f}, 0.08f, 100.0f, 1.5f, 8.0f);

    if (al::listenStageSwitchOn(this, "SwitchBattle", DoshiFunctor(this, &Doshi::onBattleStart))) {
        sead::Vector3f position;
        sead::Vector3f front;
        sead::Matrix34f matrix;
        if (al::tryGetLinksMatrixTR(&matrix, info, "WanderBattlePos")) {
            matrix.getBase(position, 3);
            matrix.getBase(front, 2);
        }
        mWanderBossBattle->setting(position, front);
    }

    makeActorAlive();

    if (mTalkNpcCap)
        al::startAction(mTalkNpcCap, "Doshi");
};

void Doshi::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (mTalkNpcCap) {
        if (al::isSensorName(self, "ShopCap1"))
            rs::sendMsgPushToPlayer(other, self);
        else if (al::isSensorName(self, "ShopCap2"))
            rs::sendMsgPushToPlayer(other, self);
    }
}

bool Doshi::receiveMsg(const al::SensorMsg* message, al::HitSensor* other, al::HitSensor* self) {
    if (al::isMsgPlayerDisregard(message))
        return false;

    if (rs::isMsgPlayerDisregardHomingAttack(message) ||
        rs::isMsgPlayerDisregardTargetMarker(message))
        return true;

    if (al::isMsgString(message)) {
        const char* msgString = al::getMsgString(message);
        return al::isEqualString(msgString, "HosuiTrailOff");
    }

    if (mWanderBossBattle->receiveMsg(message, other, self))
        return true;

    if (al::isSensorName(self, "Collision")) {
        if (rs::isMsgPlayerAndCapHipDropAll(message) &&
            (!al::isNerve(this, &Reaction) || al::isGreaterEqualStep(this, 70))) {
            al::setNerve(this, &Reaction);
            return false;
        }

        if (rs::isMsgCapAttackCollide(message) &&
            (!al::isNerve(this, &Reaction) || al::isGreaterEqualStep(this, 70))) {
            sead::Vector3f pos = al::getSensorPos(other);
            rs::requestHitReactionToAttacker(message, other, pos);

            if (mRemainingCoins >= 1) {
                al::appearItemTiming(this, "直接攻撃(コイン)", pos, sead::Vector3<float>::ez,
                                     nullptr);
                --mRemainingCoins;
            }

            al::setNerve(this, &Reaction);
            return true;
        }
    }

    if ((rs::isMsgGiantWanderBossBulletAttack(message) ||
         rs::isMsgPukupukuRollingAttack(message)) &&
        (!al::isNerve(this, &Reaction) || al::isGreaterEqualStep(this, 70))) {
        al::setNerve(this, &Reaction);
        return true;
    }

    return false;
}

// NON_MATCHING: There's a regswap in lookAtOffset.length()
void Doshi::control() {
    Camera* camera = getCamera();

    if (al::isActiveCamera(camera->ticket)) {
        al::LiveActor* player = al::getPlayerActor(this, 0);
        if (!al::isInWater(player)) {
            if (!((al::getTrans(this) - al::getPlayerPos(this, 0)).length() <= 1500.0f) &&
                al::isVelocitySlow(player, 3.0f))
                al::endCamera(this, camera->ticket, -1, 0);
        }
    }

    sead::Vector3f up;
    sead::Vector3f front;
    if (camera->isActive) {
        const sead::Vector3f& playerPos = al::getPlayerPos(this, 0);
        al::calcUpDir(&up, this);
        al::calcFrontDir(&front, this);
        const sead::Vector3f& thisPos = al::getTrans(this);

        bool length = (((up * 0.0f) + thisPos) + (front * 260.0f) - playerPos).length() <= 1500.0f;

        if (!al::isActiveCamera(camera->ticket) && length &&
            al::isInWater(al::getPlayerActor(this, 0)))
            al::startCamera(this, camera->ticket, -1);
        else if (al::isActiveCamera(camera->ticket) && !length)
            al::endCamera(this, camera->ticket, -1, false);
    }
    if (mTalkNpcCap)
        mTalkNpcCap->control();

    up = {0.0f, -mLocalOffsetY, 0.0f};
    al::calcTransLocalOffset(&mTrans, this, up);

    if (mShoppingWatcher) {
        al::calcJointPos(&up, this, "Shop");
        if (!mShoppingWatcher->isInsideTerritoryPlayer()) {
            al::calcFrontDir(&front, this);
            al::turnQuatFrontToDirDegreeH(mShoppingWatcher, front, 1.0f);
        }
        al::updatePoseTrans(mShoppingWatcher, up);
    }

    if (mBgmPlayObj)
        mBgmPlayObj->setMatrix(al::getJointMtxPtr(this, "Shop"));

    sead::Vector3f playerHeadPos = al::getPlayerPos(this, 0) - (sead::Vector3f::ey * 100.0f);
    sead::Matrix34f* headMtx = al::getJointMtxPtr(this, "Head");

    sead::Vector3f headPos;
    headMtx->getTranslation(headPos);

    sead::Vector3f lookAtOffset = sead::Vector3f(playerHeadPos - headPos);

    f32 lookAtDistance = lookAtOffset.length();

    al::calcFrontDir(&up, this);

    front = lookAtOffset;
    front.y = 0;
    al::tryNormalizeOrDirZ(&front);
    f32 angle = al::calcAngleDegree(front, up);
    lookAtOffset.y = playerHeadPos.y - (headPos.y + 200.0f);
    f32 length = lookAtOffset.length();
    if (lookAtDistance <= 1500.0f && angle <= 90.0f && !(length <= 500.0f))
        mJointLookAtController->requestJointLookAt(playerHeadPos);
    mJointLookAtController->set_50_51();

    if (mEventFlowExecutor)
        rs::updateEventFlow(mEventFlowExecutor);
    if (mShoppingWatcher) {
        s32 shopNpcSlot = mShoppingWatcher->get_1C0() ? 1 : 2;
        GameDataFunction::setShopNpcTrans(this, mShoppingWatcher->getShopName(), shopNpcSlot);
    }
}

void Doshi::onBattleStart() {
    al::invalidateClipping(this);
    mActiveNerve = &NrvDoshi.BossBattle;
    al::setNerve(this, &NrvDoshi.BossBattle);
}

void Doshi::doReaction(const al::SensorMsg* message, al::HitSensor* other, al::HitSensor* self) {
    sead::Vector3f sensorPos = al::getSensorPos(other);
    rs::requestHitReactionToAttacker(message, other, sensorPos);
    if (mRemainingCoins >= 1) {
        al::appearItemTiming(this, "直接攻撃(コイン)", sensorPos, sead::Vector3f::ez, nullptr);
        --mRemainingCoins;
    }
}

void Doshi::requestUpdateAction(f32 a2, f32 a3) {
    if (mActiveNerve == &NrvDoshi.Wait) {
        al::startAction(this, "Wait");
        al::setActionFrame(this, a2);
    } else if (mActiveNerve == &NrvDoshi.Run) {
        al::startAction(this, isInWater() ? "Swim" : "Run");
        al::setActionFrame(this, a3);
    }

    al::resetPosition(this);
    al::LiveActor* head = al::getSubActor(this, "ドッシー(頭)");
    al::resetPosition(head);
    al::LiveActor* neck1 = al::getSubActor(this, "ドッシー(首１)");
    al::resetPosition(neck1);
    al::LiveActor* neck2 = al::getSubActor(this, "ドッシー(首２)");
    al::resetPosition(neck2);
    al::LiveActor* body = al::getSubActor(this, "ドッシー(身体)");
    al::resetPosition(body);
    al::LiveActor* waist = al::getSubActor(this, "ドッシー(腰)");
    al::resetPosition(waist);
    al::LiveActor* back = al::getSubActor(this, "ドッシー(背中)");
    al::resetPosition(back);
    al::LiveActor* tail = al::getSubActor(this, "ドッシー(尻尾)");
    al::resetPosition(tail);
}

bool Doshi::isInWater() const {
    const sead::Vector3f& trans = al::getTrans(this);
    return al::isInWaterPos(this, {(sead::Vector3f::ey * 800.0f) + trans});
}

void Doshi::requestStartCamera() {
    Camera* camera = mCamera;
    if (!al::isActiveCamera(camera->ticket))
        al::startCamera(this, camera->ticket, -1);
    camera->isActive = true;
}

void Doshi::requestEndCamera() {
    Camera* camera = getCamera();
    if (al::isActiveCamera(camera->ticket))
        al::endCamera(this, camera->ticket, -1, false);
    camera->isActive = false;
}

void Doshi::exeWait() {
    if (al::isFirstStep(this)) {
        if (mActionWaitFrame >= 0.0f) {
            requestUpdateAction(mActionWaitFrame, mActionMoveFrame);
            mActionWaitFrame = -1.0f;
            mActionMoveFrame = -1.0f;
        } else
            al::startAction(this, "Wait");
    }
    mJointRippleGeneratorNeck->updateAndGenerate();
    mJointRippleGeneratorSpine->updateAndGenerate();
    mJointRippleGeneratorTail->updateAndGenerate();
}

void Doshi::exeRun() {
    if (al::isFirstStep(this)) {
        if (mActionMoveFrame >= 0.0f) {
            requestUpdateAction(mActionWaitFrame, mActionMoveFrame);
            mActionWaitFrame = -1.0f;
            mActionMoveFrame = -1.0f;
        } else
            al::startAction(this, isInWater() ? "Swim" : "Run");
    }
    mJointRippleGeneratorNeck->updateAndGenerate();
    mJointRippleGeneratorSpine->updateAndGenerate();
    mJointRippleGeneratorTail->updateAndGenerate();

    al::moveSyncRailTurn(this, al::calcNerveValue(this, 60, 0.0f, 4.0f));
    al::turnToRailDir(this, 0.8f);
}

void Doshi::exeReaction() {
    if (al::isFirstStep(this))
        al::startAction(this, isInWater() ? "ReactionWater" : "ReactionSurface");

    mJointRippleGeneratorNeck->updateAndGenerate();
    mJointRippleGeneratorSpine->updateAndGenerate();
    mJointRippleGeneratorTail->updateAndGenerate();

    if (al::isActionEnd(this))
        al::setNerve(this, mActiveNerve);
}

void Doshi::exeBossBattle() {
    al::updateNerveState(this);
}
