#include "Npc/WorldTravelingPeach.h"

#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <random/seadRandom.h>

#include "Library/Collision/PartsConnectorUtil.h"
#include "Library/Demo/DemoFunction.h"
#include "Library/Event/EventFlowExecutor.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/Joint/JointSpringControllerHolder.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Obj/PartsFunction.h"
#include "Library/Obj/PartsModel.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Stage/StageSwitchUtil.h"
#include "Library/Thread/FunctorV0M.h"

#include "Npc/FukankunZoomCapMessage.h"
#include "Npc/PeachTrunk.h"
#include "Npc/TalkNpcActionAnimInfo.h"
#include "Npc/TalkNpcStateEvent.h"
#include "Npc/Tiara.h"
#include "Npc/WorldTravelingPeachWaitState.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/DemoUtil.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerDemoUtil.h"
#include "Util/PlayerUtil.h"

namespace {
NERVE_IMPL(WorldTravelingPeach, Event);
NERVES_MAKE_STRUCT(WorldTravelingPeach, Event);
}  // namespace

static const char* sModelNames[] = {"Peach",         "PeachSummer", "PeachWinter",
                                    "PeachExplorer", "PeachFarmer", "PeachYukata"};

// NON_MATCHING: WFixedSafeString<32> inline expansion mismatch
WorldTravelingPeach::WorldTravelingPeach(const char* name) : al::LiveActor(name) {}

// NON_MATCHING: stack layout, Vector3f rodata vs inline, csel vs branch, SafeString scheduling
void WorldTravelingPeach::init(const al::ActorInitInfo& info) {
    const char* eventName = al::getStringArg(info, "EventName");

    bool isRandomModel = false;
    if (al::isEqualString(eventName, "TravelPeachCastle") &&
        GameDataFunction::isGotLinkedShineBeforeInitActor(info, "ShineActor")) {
        sead::Random rng;
        u32 index = (u32)((6 * (u64)rng.getU32()) >> 32);
        if (index > 5)
            index = 5;
        sead::SafeString archiveStr(sModelNames[index]);

        const char* modelName = nullptr;
        alPlacementFunction::getModelName(&modelName, info);
        const char* suffix = nullptr;
        if (modelName && al::isEqualString(modelName, "Peach"))
            suffix = "WorldTraveling";
        al::initActorWithArchiveName(this, info, archiveStr, suffix);
        isRandomModel = true;
    } else {
        const char* modelName = nullptr;
        alPlacementFunction::getModelName(&modelName, info);
        const char* suffix = nullptr;
        if (modelName && al::isEqualString(modelName, "Peach"))
            suffix = "WorldTraveling";
        al::initActorChangeModelSuffix(this, info, suffix);
    }

    al::initNerve(this, &NrvWorldTravelingPeach.Event, 1);
    al::initSubActorKeeperNoFile(this, info, 3);

    mTalkNpcParam = rs::initTalkNpcParam(this, nullptr);

    mAnimInfo = new TalkNpcActionAnimInfo();
    mAnimInfo->initWaitActionNameFromPlacementInfo(this, info, 0);
    mAnimInfo->init(this, info, mTalkNpcParam, nullptr);

    al::startVisAnim(this, "TiaraOff");

    al::EventFlowExecutor* executor =
        rs::initEventFlow(this, info, "WorldTravelingPeach", eventName);
    rs::initEventActionNameConverter(executor, mAnimInfo);
    rs::initEventCharacterName(executor, info, "Peach");
    al::initEventReceiver(executor, static_cast<al::IEventFlowEventReceiver*>(this));
    rs::initEventParam(executor, mTalkNpcParam, nullptr);

    if (rs::isDefinedEventCamera(executor, "Default"))
        rs::initEventCameraObject(executor, info, "Default");

    mTiara = new Tiara(u8"ティアラ");
    mTiara->mHostActor = this;
    al::initCreateActorWithPlacementInfo(mTiara, info);
    sead::Vector3f tiaraOffset = {100.0f, 160.0f, -100.0f};
    al::calcTransLocalOffset(al::getTransPtr(mTiara), this, tiaraOffset);
    al::invalidateClipping(mTiara);
    al::registerSubActorSyncClipping(this, mTiara);

    rs::makeEventCharacterName(&mCharName, info, "Tiara");

    const char* capName = nullptr;
    if (isRandomModel) {
        const char* myModelName = al::getModelName(this);
        if (al::isEqualString(myModelName, "PeachExplorer")) {
            capName = "PeachExplorerCap";
        } else {
            myModelName = al::getModelName(this);
            if (al::isEqualString(myModelName, "PeachSummer")) {
                capName = "PeachSummerCap";
            } else {
                myModelName = al::getModelName(this);
                if (al::isEqualString(myModelName, "PeachWinter"))
                    capName = "PeachWinterCap";
            }
        }
        if (!capName)
            goto checkCapType;
    } else {
    checkCapType:
        const char* capType = nullptr;
        if (!al::tryGetStringArg(&capType, info, "CapType") || !capType ||
            al::isEqualString(capType, "None"))
            goto noCap;
        capName = capType;
    }
    {
        const sead::Matrix34f* jointMtx = al::getJointMtxPtr(this, "CapTarget");
        al::PartsModel* cap =
            al::createPartsModel(this, info, u8"世界旅行ピーチの帽子", capName, jointMtx);
        al::onSyncAlphaMaskSubActor(this, u8"世界旅行ピーチの帽子");
        al::JointSpringControllerHolder::tryCreateAndInitJointControllerKeeper(
            cap, "InitJointSpringCtrl");
    }
noCap:
    PeachTrunk* trunk = new PeachTrunk(u8"世界旅行ピーチのトランク");
    al::initCreateActorNoPlacementInfo(trunk, info);
    sead::Vector3f* trunkTrans = al::getTransPtr(trunk);
    const char* myModelName2 = al::getModelName(this);
    if (al::isEqualString(myModelName2, "Peach")) {
        sead::Vector3f trunkOffset = {90.0f, 0.0f, -35.0f};
        al::calcTransLocalOffset(trunkTrans, this, trunkOffset);
    } else {
        sead::Vector3f trunkOffset = {60.0f, 0.0f, -20.0f};
        al::calcTransLocalOffset(trunkTrans, this, trunkOffset);
    }
    al::addRotateAndRepeatY(trunk, 70.0f);
    al::registerSubActorSyncClipping(this, trunk);
    al::onSyncAlphaMaskSubActor(this, trunk);

    mWaitState = new WorldTravelingPeachWaitState(this, mTiara, mTalkNpcParam, mAnimInfo);
    rs::initEventMovement(executor, mWaitState, info);

    s32 lookAtJointNum = rs::getNpcJointLookAtControlJointNum(mTalkNpcParam);
    s32 springJointNum = al::JointSpringControllerHolder::calcInitFileSpringControlJointNum(
        this, "InitJointSpringCtrl");
    al::initJointControllerKeeper(this, springJointNum + lookAtJointNum);

    if (lookAtJointNum >= 1)
        mJointLookAtController =
            rs::tryCreateAndAppendNpcJointLookAtController(this, mTalkNpcParam);

    if (springJointNum >= 1) {
        mSpringController = new al::JointSpringControllerHolder();
        mSpringController->init(this, "InitJointSpringCtrl");
    }

    mStateEvent = new TalkNpcStateEvent(u8"イベント", this, mTalkNpcParam, executor, -1);
    al::initNerveState(this, mStateEvent, &NrvWorldTravelingPeach.Event, u8"イベント");

    ActorStateReactionBase* reaction = rs::createNpcStateReaction(
        this, mTalkNpcParam, (const NpcStateReactionParam*)mAnimInfo->_18);
    mStateEvent->initStateReaction(reaction);

    GameDataHolderAccessor accessor(this);
    const char* worldName = GameDataFunction::getWorldDevelopNameCurrent(accessor);
    if (!rs::checkGetShineForWorldTravelingPeach(accessor, worldName)) {
        mCapMessage = new FukankunZoomCapMessage(this);
        mCapMessage->init(info, "CapMessage", "FukankunZoomWorldTravelingPeach");
    }

    sead::Quatf quat = sead::Quatf::unit;
    const sead::Vector3f& front = al::getFront(this);
    const sead::Vector3f& gravity = al::getGravity(this);
    sead::Vector3f up = {-gravity.x, -gravity.y, -gravity.z};
    al::makeQuatFrontUp(&quat, front, up);

    mMtxConnector = al::tryCreateMtxConnector(this, info, quat);

    if (al::isEqualString(eventName, "TravelPeachCastle")) {
        if (!rs::checkGetShineForWorldTravelingPeach(GameDataHolderAccessor(this), "Moon"))
            goto disableAndReturn;
    } else if (al::isEqualString(eventName, "TravelMoon")) {
        if (!rs::checkEnableAppearMoonWorldTravelingPeach(this))
            goto disableAndReturn;
    } else if ((al::isEqualString(eventName, "Travel") ||
                al::isEqualString(eventName, "TravelWithTiara")) &&
               !rs::isStartWorldTravelingPeach(this)) {
        goto disableAndReturn;
    }

    rs::startEventFlow(executor, "Init");

    {
        s32 shineIndex = GameDataFunction::tryFindLinkedShineIndex(this, info);
        GameDataFunction::enableHintByShineIndex(this, shineIndex);
    }

    if (al::tryGetArg(&mIsInvalidTrample, info, "IsInvalidTrample")) {
        if (mIsInvalidTrample)
            *(reinterpret_cast<u8*>(mTiara) + 317) = 1;
    }

    al::listenStageSwitchOnOff(
        this, "SwitchPeachWorldHomeCastleCapDemo",
        al::FunctorV0M<WorldTravelingPeach*, void (WorldTravelingPeach::*)()>(
            this, &WorldTravelingPeach::startPeachWorldHomeCastleCapDemo),
        al::FunctorV0M<WorldTravelingPeach*, void (WorldTravelingPeach::*)()>(
            this, &WorldTravelingPeach::endPeachWorldHomeCastleCapDemo));

    al::registActorToDemoInfo(this, info);
    al::trySyncStageSwitchAppearAndKill(this);
    return;

disableAndReturn:
    s32 shineIndex2 = GameDataFunction::tryFindLinkedShineIndex(this, info);
    GameDataFunction::disableHintByShineIndex(this, shineIndex2);
    makeActorDead();
}

void WorldTravelingPeach::startPeachWorldHomeCastleCapDemo() {
    if (al::isDead(this))
        return;
    mWaitState->startPeachWorldHomeCastleCapDemo();
    if (!al::isNerve(this, &NrvWorldTravelingPeach.Event))
        al::setNerve(this, &NrvWorldTravelingPeach.Event);
    rs::addDemoActor(this, true);
}

void WorldTravelingPeach::endPeachWorldHomeCastleCapDemo() {
    if (al::isDead(this))
        return;
    mWaitState->endPeachWorldHomeCastleCapDemo();
}

void WorldTravelingPeach::initAfterPlacement() {
    rs::tryAttachConnectorToCollisionTFSV(this, mMtxConnector, &mConnectorQuat);
    if (mCapMessage)
        mCapMessage->initAfterPlacement();
    rs::tryUpdateMaterialCodeByFloorCollisionOnArrow(this);
}

void WorldTravelingPeach::makeActorDead() {
    al::LiveActor::makeActorDead();
    al::makeActorDeadSubActorAll(this);
}

// NON_MATCHING: Quatf copy pattern, stack layout, instruction scheduling
void WorldTravelingPeach::control() {
    rs::trySwitchDepthToSelfShadow(this);
    if (mJointLookAtController)
        rs::updateNpcJointLookAtController(mJointLookAtController);

    if (mMtxConnector) {
        sead::Vector3f connectorTrans = {0.0f, 0.0f, 0.0f};
        sead::Quatf connectorQuat = sead::Quatf::unit;

        const sead::Vector3f& oldTrans = al::getTrans(this);
        f32 oldX = oldTrans.x;
        f32 oldY = oldTrans.y;
        f32 oldZ = oldTrans.z;

        al::calcConnectQT(&connectorQuat, &connectorTrans, mMtxConnector);

        sead::Vector3f oldFront = {0.0f, 0.0f, 0.0f};
        sead::Vector3f newFront = {0.0f, 0.0f, 0.0f};
        al::calcQuatFront(&oldFront, mConnectorQuat);
        al::calcQuatFront(&newFront, connectorQuat);

        const sead::Vector3f& gravity = al::getGravity(this);
        sead::Vector3f up = {-gravity.x, -gravity.y, -gravity.z};
        f32 angle = al::calcAngleOnPlaneDegree(oldFront, newFront, up);

        al::rotateVectorDegreeY(al::getFrontPtr(this), angle);
        al::setTrans(this, connectorTrans);

        Tiara* tiara = mTiara;
        mConnectorQuat = connectorQuat;

        const sead::Vector3f& tiaraTrans = al::getTrans(tiara);
        sead::Vector3f newTiaraTrans = {(connectorTrans.x - oldX) + tiaraTrans.x,
                                        (connectorTrans.y - oldY) + tiaraTrans.y,
                                        (connectorTrans.z - oldZ) + tiaraTrans.z};
        al::setTrans(tiara, newTiaraTrans);

        al::LiveActor* trunkActor = al::getSubActor(this, u8"世界旅行ピーチのトランク");
        const sead::Vector3f& trunkTrans = al::getTrans(trunkActor);
        sead::Vector3f trunkRelative = {trunkTrans.x - oldX, trunkTrans.y - oldY,
                                        trunkTrans.z - oldZ};
        al::rotateVectorDegreeY(&trunkRelative, angle);

        al::LiveActor* trunkActor2 = al::getSubActor(this, u8"世界旅行ピーチのトランク");
        const sead::Vector3f& myTrans = al::getTrans(this);
        sead::Vector3f newTrunkTrans = {myTrans.x + trunkRelative.x, myTrans.y + trunkRelative.y,
                                        myTrans.z + trunkRelative.z};
        al::setTrans(trunkActor2, newTrunkTrans);

        al::addRotateAndRepeatY(al::getSubActor(this, u8"世界旅行ピーチのトランク"), angle);
    }

    GameDataHolderAccessor accessor(this);
    const char* worldName = GameDataFunction::getWorldDevelopNameCurrent(accessor);
    if (!rs::checkGetShineForWorldTravelingPeach(accessor, worldName)) {
        if (mCapMessage)
            mCapMessage->update();
    }
}

void WorldTravelingPeach::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (!al::isSensorEye(self)) {
        rs::attackSensorNpcCommon(self, other);
        return;
    }
    if (al::isNerve(this, &NrvWorldTravelingPeach.Event))
        rs::sendMsgEventFlowScareCheck(other, self, mStateEvent->mEventFlowExecutor);
}

bool WorldTravelingPeach::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                     al::HitSensor* self) {
    if (rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;
    if (rs::tryReceiveMsgPlayerDisregard(msg, self, mTalkNpcParam))
        return true;

    if (rs::isInvalidTrampleSensor(self, mTalkNpcParam) || mIsInvalidTrample) {
        if (mStateEvent->tryStartReactionWithoutTrample(msg, other, self))
            return true;
    } else {
        if (mStateEvent->tryStartReaction(msg, other, self))
            return true;
    }

    return false;
}

// NON_MATCHING: null check on IUseSceneObjHolder cast
bool WorldTravelingPeach::receiveEvent(const al::EventFlowEventData* data) {
    if (al::isEventName(data, "TiaraWait")) {
        al::EventFlowExecutor* executor = mStateEvent->mEventFlowExecutor;
        mTiara->startWait();
        rs::resetEventCharacterName(executor);
        al::resetTalkSubActorName(executor);
        sead::Vector3f watchTrans = {0.0f, 0.0f, 0.0f};
        rs::calcPlayerWatchTrans(&watchTrans, this, mTalkNpcParam);
        rs::validateWatchTarget(this, watchTrans);
        return true;
    }

    if (al::isEventName(data, "TiaraTalk")) {
        al::EventFlowExecutor* executor = mStateEvent->mEventFlowExecutor;
        mTiara->startTalk();
        rs::swapEventCharacterName(executor, &mCharName);
        al::setTalkSubActorName(executor, u8"ティアラ");
        rs::validateWatchTarget(this, al::getTrans(mTiara));
        return true;
    }

    if (al::isEventName(data, "TiaraStop")) {
        mTiara->startStop();
        return true;
    }

    if (al::isEventName(data, "WorldTravelingPeachActionLoop")) {
        const char* actionName = al::getEventDataParamString(data, "ActionName");
        rs::startNpcAction(this, actionName);
        return true;
    }

    if (al::isEventName(data, "WorldTravelingPeachActionOneTime")) {
        const char* actionName = al::getEventDataParamString(data, "ActionName");
        if (rs::tryStartNpcActionIfNotPlaying(this, actionName))
            return false;
        return al::isActionEnd(this);
    }

    if (al::isEventName(data, "CheckFirstTalk"))
        return !rs::isTalkWorldTravelingPeach(this);

    if (al::isEventName(data, "SaveAfterFirstTalk")) {
        rs::talkWorldTravelingPeach(this);
        return true;
    }

    if (al::isEventName(data, "CheckCompletePeachCastle")) {
        GameDataHolderAccessor accessor(this);
        return rs::checkGetShineForWorldTravelingPeach(accessor, "Peach");
    }

    return false;
}

void WorldTravelingPeach::exeEvent() {
    al::updateNerveState(this);

    bool scareTiming = rs::isNpcScareTiming(mStateEvent->mEventFlowExecutor);
    Tiara* tiara = mTiara;
    if (!scareTiming) {
        tiara->tryEndScare();
        return;
    }

    tiara->tryStartScare();

    if (rs::isPlayerHack(this) && rs::isPlayerHackGroupTalkScare(this)) {
        const TalkNpcParam* param = mTalkNpcParam;
        const sead::Vector3f& playerPos = rs::getPlayerPos(this);
        rs::tryUpdateNpcEyeLineAnimToTarget(this, param, playerPos, true);
        return;
    }

    sead::Vector3f scarePos = {0.0f, 0.0f, 0.0f};
    if (al::tryGetScareEnemyPos(&scarePos, mStateEvent->mEventFlowExecutor))
        rs::tryUpdateNpcEyeLineAnimToTarget(this, mTalkNpcParam, scarePos, true);
    else
        rs::resetNpcEyeLineAnim(this);
}
