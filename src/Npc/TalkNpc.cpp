#include "Npc/TalkNpc.h"

#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Area/AreaObjGroup.h"
#include "Library/Area/AreaObjUtil.h"
#include "Library/Collision/Collider.h"
#include "Library/Collision/PartsConnectorUtil.h"
#include "Library/Demo/DemoFunction.h"
#include "Library/Event/EventFlowExecutor.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowMovementRailBlend.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/Joint/JointControllerBase.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/Joint/JointSpringControllerHolder.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorResourceFunction.h"
#include "Library/LiveActor/ActorSensorFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Model/ModelShapeUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Rail/RailUtil.h"
#include "Library/Shadow/ActorShadowUtil.h"
#include "Library/Stage/StageSwitchUtil.h"
#include "Library/Thread/FunctorV0M.h"
#include "Library/Yaml/ByamlIter.h"

#include "Npc/BgmAnimeSynchronizer.h"
#include "Npc/BirdMtxGlideCtrl.h"
#include "Npc/EventActorMovementRailTraffic.h"
#include "Npc/EventActorMovementTurn.h"
#include "Npc/HideAndSeekCapMan.h"
#include "Npc/RandomActionUpdater.h"
#include "Npc/TalkNpcActionAnimInfo.h"
#include "Npc/TalkNpcCap.h"
#include "Npc/TalkNpcParam.h"
#include "Npc/TalkNpcPartialAnimCtrl.h"
#include "Npc/TalkNpcStateEvent.h"
#include "Npc/TalkNpcSwitchActionPlayerHolder.h"
#include "Npc/TalkNpcSwitchWaitActionObj.h"
#include "Player/PlayerEyeSensorHitHolder.h"
#include "System/GameDataFunction.h"
#include "System/GameDataUtil.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {

NERVE_IMPL(TalkNpc, DemoKoopa);
NERVES_MAKE_NOSTRUCT(TalkNpc, DemoKoopa);

// GHOST: Move these to their proper files according to file_list.yml.
class TrampleReactionJointCtrl : public al::JointControllerBase {
public:
    TrampleReactionJointCtrl(s32 numJoints) : al::JointControllerBase(numJoints) {}

    void calcJointCallback(s32 jointId, sead::Matrix34f* mtx) override;
    const char* getCtrlTypeName() const override;

    s32 mAnimFrame = -1;
    s32 mJointRootIndex = 0;
    s32 mNumChildren = 0;
    bool mIsCapReaction = false;
};

// GHOST: Move these to their proper files according to file_list.yml.
class CapReactionJointCtrl : public al::JointControllerBase {
public:
    CapReactionJointCtrl(s32 numJoints) : al::JointControllerBase(numJoints) {}

    void calcJointCallback(s32 jointId, sead::Matrix34f* mtx) override;
    const char* getCtrlTypeName() const override;

    const sead::Matrix34f* mBaseMtx = nullptr;
    s32 mJointRootIndex = 0;
    s32 mAnimFrame = -1;
    sead::Vector3f mAxis = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mRotatedAxis = {0.0f, 0.0f, 0.0f};
};

// GHOST: Move these to their proper files according to file_list.yml.
struct SubActorAnimCache {
    const char* mSklAnimName = nullptr;
    const char* mMtsAnimName = nullptr;
};

// GHOST: Move these to their proper files according to file_list.yml.
struct SubActorSyncData {
    al::LiveActor* mActor = nullptr;
    SubActorAnimCache* mAnimCache = nullptr;
    al::LiveActor** mSubActors = nullptr;
    s32 mSubActorCount = 0;
    bool mHasCityMayorFace = false;
};

// GHOST: Move these to their proper files according to file_list.yml.
struct ClippingNearClipInfo {
    void* mClippingActorInfo = nullptr;
    f32 mSavedNearClip = 0.0f;
};

NERVE_IMPL(TalkNpc, Event);
NERVE_IMPL(TalkNpc, HackEvent);
NERVE_IMPL(TalkNpc, KinopioBrigadeFirstEvent);

NERVES_MAKE_STRUCT(TalkNpc, Event, KinopioBrigadeFirstEvent, HackEvent);

// NON_MATCHING: translation sinking for trample reaction
void TrampleReactionJointCtrl::calcJointCallback(s32 jointId, sead::Matrix34f* mtx) {
    if (mAnimFrame < 0)
        return;

    s32 maxFrame = mIsCapReaction ? 70 : 10;
    f32 amplitude = mIsCapReaction ? 15.0f : 30.0f;

    f32 ratio = al::normalize(mAnimFrame, 0, maxFrame);
    f32 angle = al::lerpValue(-45.0f, -180.0f, ratio);
    f32 offset = amplitude * sead::Mathf::sin(angle * 0.017453f);

    if (mJointRootIndex == jointId) {
        (*mtx)(0, 3) += 0.0f;
        (*mtx)(1, 3) += offset;
        (*mtx)(2, 3) += 0.0f;
    } else {
        f32 depthRatio = al::normalize(1, 0, mNumChildren);
        (*mtx)(1, 3) -= offset * depthRatio;
    }
}

const char* TrampleReactionJointCtrl::getCtrlTypeName() const {
    return u8"踏みリアクション中の沈み挙動";
}

// NON_MATCHING: complex quaternion rotation for cap reaction recoil
void CapReactionJointCtrl::calcJointCallback(s32 jointId, sead::Matrix34f* mtx) {
    if (mAnimFrame < 0)
        return;

    f32 sinkAngle = mJointRootIndex == jointId ? 10.0f : -10.0f;
    f32 ratio = al::normalize(mAnimFrame, 0, 70);
    f32 decaying = (1.0f - ratio) * sinkAngle;

    f32 ratio2 = al::normalize(mAnimFrame, 0, 70);
    f32 eased = al::easeOut(ratio2);
    f32 angle = al::lerpValue(50.0f, 360.0f, eased);
    f32 offset = decaying * sead::Mathf::sin(angle * 0.017453f);

    sead::Quatf baseQuat = sead::Quatf::unit;
    mBaseMtx->toQuat(baseQuat);

    sead::Quatf rotQuat = sead::Quatf::unit;
    al::makeQuatRotateDegree(&rotQuat, mRotatedAxis, offset);

    sead::Quatf mtxQuat = sead::Quatf::unit;
    mtx->toQuat(mtxQuat);

    sead::Quatf result;
    result.x = mtxQuat.w * rotQuat.x + mtxQuat.x * rotQuat.w + mtxQuat.y * rotQuat.z -
               mtxQuat.z * rotQuat.y;
    result.y = mtxQuat.w * rotQuat.y - mtxQuat.x * rotQuat.z + mtxQuat.y * rotQuat.w +
               mtxQuat.z * rotQuat.x;
    result.z = mtxQuat.w * rotQuat.z + mtxQuat.x * rotQuat.y - mtxQuat.y * rotQuat.x +
               mtxQuat.z * rotQuat.w;
    result.w = mtxQuat.w * rotQuat.w - mtxQuat.x * rotQuat.x - mtxQuat.y * rotQuat.y -
               mtxQuat.z * rotQuat.z;

    f32 savedTx = (*mtx)(0, 3);
    f32 savedTy = (*mtx)(1, 3);
    f32 savedTz = (*mtx)(2, 3);
    mtx->fromQuat(result);

    if (mJointRootIndex == jointId) {
        f32 ax = mAxis.x, ay = mAxis.y, az = mAxis.z;
        f32 bx = baseQuat.x, by = baseQuat.y, bz = baseQuat.z, bw = baseQuat.w;

        f32 cx = ay * bz - az * by + bw * ax;
        f32 cy = az * bx - ax * bz + bw * ay;
        f32 cz = ax * by - ay * bx + bw * az;
        f32 cw = -(ax * bx + ay * by + az * bz);

        f32 rx = bw * cx - bx * cw + by * cz - bz * cy;
        f32 ry = bw * cy - by * cw + bz * cx - bx * cz;
        f32 rz = bw * cz - bz * cw + bx * cy - by * cx;

        f32 baseTx = (*mBaseMtx)(0, 3);
        f32 baseTy = (*mBaseMtx)(1, 3);
        f32 baseTz = (*mBaseMtx)(2, 3);

        f32 dist = sead::Mathf::sqrt((baseTx - savedTx) * (baseTx - savedTx) +
                                     (baseTy - savedTy) * (baseTy - savedTy) +
                                     (baseTz - savedTz) * (baseTz - savedTz));

        f32 tanVal = sead::Mathf::tan(offset * 0.017453f);

        (*mtx)(0, 3) = savedTx + dist * tanVal * rx;
        (*mtx)(1, 3) = savedTy + dist * tanVal * ry;
        (*mtx)(2, 3) = savedTz + dist * tanVal * rz;
    }
}

const char* CapReactionJointCtrl::getCtrlTypeName() const {
    return u8"帽子リアクション中ののけぞり挙動";
}

void countJointDescendants(s32* outCount, s32* outMaxDepth, al::LiveActor* actor,
                           const char* jointName) {
    const al::ModelKeeper* keeper = actor->getModelKeeper();
    *outCount = 1;
    *outMaxDepth = 0;

    s32 totalJoints = al::getJointNum(keeper);
    s32 targetIndex = al::getJointIndex(keeper, jointName);

    for (s32 i = 0; i < totalJoints; i++) {
        const char* childName = al::getJointName(keeper, i);
        if (i == targetIndex)
            continue;

        s32 parentIndex = al::getParentJointIndex(keeper, i);
        if (targetIndex != parentIndex)
            continue;

        s32 childCount = 0;
        s32 childMaxDepth = 1;
        countJointDescendants(&childCount, &childMaxDepth, actor, childName);

        *outCount += childCount;
        if (*outMaxDepth < childMaxDepth)
            *outMaxDepth = childMaxDepth;
    }

    *outMaxDepth += 1;
}

void recursiveAppendJointId(al::JointControllerBase* controller, al::LiveActor* actor,
                            s32 jointIndex) {
    const al::ModelKeeper* keeper = actor->getModelKeeper();
    s32 totalJoints = al::getJointNum(keeper);

    for (s32 i = 0; i < totalJoints; i++) {
        if (i == jointIndex)
            continue;

        s32 parentIndex = al::getParentJointIndex(keeper, i);
        if (parentIndex != jointIndex)
            continue;

        recursiveAppendJointId(controller, actor, i);
    }

    controller->appendJointId(jointIndex);
}

s32 initMovementType(al::EventFlowExecutor* eventExec, al::LiveActor* actor,
                     const al::ActorInitInfo& info, const TalkNpcParam* param,
                     const TalkNpcActionAnimInfo* animInfo) {
    if (al::isExistRail(actor)) {
        if (rs::isExistTrafficAreaDirector(actor)) {
            auto* movement = new EventActorMovementRailTraffic(u8"交通レール移動", actor);
            rs::initEventMovement(eventExec, movement, info);
            return 6;
        }

        bool isKeepMove = false;
        if (al::tryGetArg(&isKeepMove, info, "IsKeepMoveOnRail") && isKeepMove) {
            auto* movement = new al::EventFlowMovementRailBlend(actor);
            rs::initEventMovement(eventExec, movement, info);
            return 5;
        }

        rs::initEventMovementRail(eventExec, info);
        return 4;
    }

    if (al::tryGetBoolArgOrFalse(info, "IsTurn")) {
        auto* movement = new EventActorMovementTurn(actor, param, animInfo);
        rs::initEventMovement(eventExec, movement, info);
        return 1;
    }

    if (al::tryGetBoolArgOrFalse(info, "IsWander")) {
        if (al::tryGetBoolArgOrFalse(info, "IsWanderWithoutCollider")) {
            rs::initEventMovementWander(eventExec, info);
            return 3;
        }
        al::onCollide(actor);
        rs::initEventMovementWander(eventExec, info);
        return 2;
    }

    rs::initEventMovementWait(eventExec, info);
    return 0;
}

}  // namespace

TalkNpc::TalkNpc(const char* name) : al::LiveActor(name) {}

// NON_MATCHING: massive init function (4840 bytes)
void TalkNpc::init(const al::ActorInitInfo& info) {
    const char* argWaitAction = TalkNpcActionAnimInfo::getArgWaitActionName(info);
    const char* modelSuffix = nullptr;
    const char* modelName = nullptr;
    if (alPlacementFunction::tryGetModelName(&modelName, info) && modelName &&
        al::isEqualString(modelName, "Rabbit")) {
        modelSuffix = "TalkNpc";
    } else {
        bool isInRoom = false;
        al::tryGetArg(&isInRoom, info, "IsInRoom");
        if (al::isEqualSubString(argWaitAction, "SitBench")) {
            modelSuffix = "SitBench";
        } else if (al::isEqualString(argWaitAction, "UmbrellaWait")) {
            modelSuffix = "Umbrella";
        } else {
            bool isInvalidDither = false;
            if (al::tryGetArg(&isInvalidDither, info, "IsInvalidDitherAnim") && isInvalidDither)
                modelSuffix = "NoAwayNearClip";
            else if (isInRoom)
                modelSuffix = "InRoom";
        }
    }

    al::initActorChangeModelSuffix(this, info, modelSuffix);
    al::initNerve(this, &NrvTalkNpc.Event, 3);

    if (!al::isExistSubActorKeeper(this))
        al::initSubActorKeeperNoFile(this, info, 2);

    al::offCollide(this);
    mInitTrans = al::getTrans(this);
    mInitFront = al::getFront(this);

    bool isTimeBalloonOrRace = rs::isSequenceTimeBalloonOrRace(this);

    mActionAnimInfo = new TalkNpcActionAnimInfo();
    mActionAnimInfo->initWaitActionNameFromPlacementInfo(this, info, isTimeBalloonOrRace);

    auto* syncHolder = new SubActorSyncData();
    syncHolder->mActor = this;
    auto* animCache = new SubActorAnimCache();
    syncHolder->mAnimCache = animCache;

    al::LiveActor* cityMayorFace = rs::tryGetSubActorCityMayorFace(this);
    syncHolder->mHasCityMayorFace = cityMayorFace != nullptr;
    s32 subActorNum = al::getSubActorNum(this);
    s32 syncCount = subActorNum - (s32)syncHolder->mHasCityMayorFace;

    auto** subActors = new al::LiveActor*[syncCount];
    for (s32 i = 0; i < syncCount; i++)
        subActors[i] = nullptr;
    syncHolder->mSubActors = subActors;

    for (s32 i = 0; i < subActorNum; i++) {
        al::LiveActor* subActor = al::getSubActor(this, i);
        if (subActor != cityMayorFace && subActor->getModelKeeper() &&
            subActor->getModelKeeper() != getModelKeeper()) {
            syncHolder->mSubActors[syncHolder->mSubActorCount++] = subActor;
        }
    }
    mSubActorSyncHolder = (TalkNpcSubActorSyncHolder*)syncHolder;

    mTalkNpcParam = rs::initTalkNpcParam(this, nullptr);
    mActionAnimInfo->init(this, info, mTalkNpcParam, modelSuffix);

    al::tryGetArg(&mIsInvalidTrample, info, "IsInvalidTrample");

    mTalkNpcCap = TalkNpcCap::tryCreate(this, info);
    if (mTalkNpcCap) {
        al::registerSubActor(this, (al::LiveActor*)mTalkNpcCap);
        al::onSyncClippingSubActor(this, (al::LiveActor*)mTalkNpcCap);
        al::onSyncAppearSubActor(this, (al::LiveActor*)mTalkNpcCap);
        al::onSyncHideSubActor(this, (al::LiveActor*)mTalkNpcCap);
        al::onSyncAlphaMaskSubActor(this, (al::LiveActor*)mTalkNpcCap);
    }

    mHideAndSeekCapMan = (al::LiveActor*)HideAndSeekCapMan::tryCreateLinkNpcIfClearGame(
        this, info, modelSuffix, "HideAndSeekCapMan");
    if (mHideAndSeekCapMan) {
        al::onSyncClippingSubActor(this, mHideAndSeekCapMan);
        al::onSyncAlphaMaskSubActor(this, mHideAndSeekCapMan);
    }

    mRandomActionUpdater = new RandomActionUpdater(this, mActionAnimInfo);
    mSwitchActionPlayerHolder =
        TalkNpcSwitchActionPlayerHolder::tryCreate(mRandomActionUpdater, info);
    TalkNpcSwitchWaitActionObj::tryCreate(mTalkNpcParam, mActionAnimInfo, mRandomActionUpdater,
                                          info);

    sead::Quatf frontQuat = sead::Quatf::unit;
    const sead::Vector3f& front = al::getFront(this);
    const sead::Vector3f& gravity = al::getGravity(this);
    sead::Vector3f negGravity = {-gravity.x, -gravity.y, -gravity.z};
    al::makeQuatFrontUp(&frontQuat, front, negGravity);

    mMtxConnector = al::tryCreateMtxConnector(this, info, frontQuat);

    if (mTalkNpcParam->mIsValidVariableFrameRateSkeletalAnim)
        mVariableFrameRateAnimator = nullptr;  // TODO: proper initialization

    rs::tryApplyNpcMaterialAnimPresetFromPlacementInfo(this, info, mTalkNpcParam);
    rs::setNpcMaterialAnimFromPlacementInfo(this, info);

    if (!mActionAnimInfo->tryApplyVisAnim(this))
        rs::tryStartForestManFlowerAnim(this);

    if (al::isExistDitherAnimator(this)) {
        bool isInvalidDither = false;
        if (al::tryGetArg(&isInvalidDither, info, "IsInvalidDitherAnim") && isInvalidDither)
            al::stopDitherAnimAutoCtrl(this);
    }

    bool isReduceEyeSensor = false;
    if (al::tryGetArg(&isReduceEyeSensor, info, "IsReduceEyeSensor") && isReduceEyeSensor)
        al::setSensorRadius(this, "Eye", 500.0f);

    al::FunctorV0M<TalkNpc*, void (TalkNpc::*)()> onFunctor(
        this, &TalkNpc::onFukankunCameraNearClipSetting);
    al::FunctorV0M<TalkNpc*, void (TalkNpc::*)()> offFunctor(
        this, &TalkNpc::offFukankunCameraNearClipSetting);
    al::listenStageSwitchOnOff(this, "SwitchFukankunCameraNearClipCtrl", onFunctor, offFunctor);

    bool isCapModel = al::isModelName(this, "CapMan") || al::isModelName(this, "CapWoman");

    s32 springNum = al::JointSpringControllerHolder::calcInitFileSpringControlJointNum(
        this, "InitJointSpringCtrl");
    al::initJointControllerKeeper(this, springNum + (s32)isCapModel + 10);

    mJointLookAtController = rs::tryCreateAndAppendNpcJointLookAtController(this, mTalkNpcParam);

    if (al::isExistModelResourceYaml(this, "InitJointSpringCtrl", nullptr)) {
        auto* springHolder = new al::JointSpringControllerHolder();
        springHolder->init(this, "InitJointSpringCtrl");
        mJointSpringControllerHolder = springHolder;
    }

    if (mJointLookAtController) {
        auto* eyeSensor = new PlayerEyeSensorHitHolder(1);
        mPlayerEyeSensorHitHolder = eyeSensor;
        rs::setPlayerEyeSensorHitHolder(mJointLookAtController, eyeSensor);
    }

    if (al::tryGetBoolArgOrFalse(info, "IsUseGroundSmooth"))
        mTalkNpcParam->createJointGroundSmoothController(this);

    if (mTalkNpcParam->mTrampleSensorName) {
        s32 hipDescCount = 0;
        s32 hipMaxDepth = 0;
        countJointDescendants(&hipDescCount, &hipMaxDepth, this, "Hip");

        auto* hipCtrl = new TrampleReactionJointCtrl(hipDescCount + 1);
        s32 rootJointIdx = al::getJointIndex(this, "JointRoot");
        s32 hipJointIdx = al::getJointIndex(this, "Hip");
        hipCtrl->mNumChildren = hipMaxDepth;
        hipCtrl->mIsCapReaction = false;
        hipCtrl->mAnimFrame = -1;
        hipCtrl->mJointRootIndex = rootJointIdx;
        hipCtrl->appendJointId(rootJointIdx);
        recursiveAppendJointId(hipCtrl, this, hipJointIdx);
        al::registerJointController(this, hipCtrl);
        mHipJointController = hipCtrl;

        auto* spineCtrl = new CapReactionJointCtrl(3);
        s32 spineRootIdx = al::getJointIndex(this, "JointRoot");
        s32 spine1Idx = al::getJointIndex(this, "Spine1");
        s32 spine2Idx = al::getJointIndex(this, "Spine2");
        spineCtrl->mBaseMtx = getBaseMtx();
        spineCtrl->mJointRootIndex = spineRootIdx;
        spineCtrl->mAnimFrame = -1;
        spineCtrl->mAxis = {0.0f, 0.0f, 0.0f};
        spineCtrl->mRotatedAxis = {0.0f, 0.0f, 0.0f};
        spineCtrl->appendJointId(spineRootIdx);
        spineCtrl->appendJointId(spine1Idx);
        spineCtrl->appendJointId(spine2Idx);
        al::registerJointController(this, spineCtrl);
        mSpineJointController = spineCtrl;
    }

    if (isCapModel)
        rs::initCapWorldNpcTail(this);

    mBirdMtxGlideCtrl =
        BirdMtxGlideCtrl::tryCreateAliveWaitByLinksBird(&mBirdGlideMtx, this, info, "Bird");
    if (mBirdMtxGlideCtrl)
        mTalkNpcParam->calcBirdGlideMtx(&mBirdGlideMtx, this);

    s32 shadowLength = -1;
    if (al::tryGetArg(&shadowLength, info, "ShadowLength") && shadowLength >= 1)
        al::setShadowMaskDropLength(this, (f32)shadowLength);

    al::ByamlIter bgmIter;
    if (al::tryGetActorInitFileIter(&bgmIter, this, "BgmRhythmSyncInfo", modelSuffix)) {
        mBgmAnimeSynchronizer = BgmAnimeSynchronizer::tryCreate(this, bgmIter);
        if (mBgmAnimeSynchronizer && !al::isInitializedBgmKeeper(this))
            al::initActorBgmKeeper(this, info, nullptr);
    }

    ActorStateReactionBase* stateReaction =
        rs::createNpcStateReaction(this, mTalkNpcParam, mActionAnimInfo->_18);

    const char* actorParamSuffix = mActionAnimInfo->tryGetActorParamSuffix();
    const char* eventName = "None";

    if (isTimeBalloonOrRace) {
        al::EventFlowExecutor* exec;
        exec = rs::initEventFlowSuffix(this, info, modelSuffix, "TalkNpc", "None");
        if (mJointLookAtController)
            al::onExistLookAtJointCtrl(exec);
        rs::initEventParam(exec, mTalkNpcParam, actorParamSuffix);
        rs::initEventActionNameConverter(exec, mActionAnimInfo);
        al::initEventReceiver(exec, (al::IEventFlowEventReceiver*)this);
        rs::tryInitItemKeeperByEvent(this, info, exec);

        s32 movType;
        if (al::isModelName(this, "ForestManScrap")) {
            rs::initEventMovementWait(exec, info);
            movType = 0;
        } else {
            auto* movement = new EventActorMovementTurn(this, mTalkNpcParam, mActionAnimInfo);
            rs::initEventMovement(exec, movement, info);
            movType = 1;
        }
        mMovementType = movType;

        rs::startNpcAction(this, mActionAnimInfo->getWaitActionName());

        auto* stateEvent = new TalkNpcStateEvent(u8"通常イベント", this, mTalkNpcParam, exec, -1);
        mStateEvent = stateEvent;
        stateEvent->initStateReaction(stateReaction);
        mStateEvent->setAnimInfo(mActionAnimInfo);
        mStateEvent->setRandomActionUpdater(mRandomActionUpdater);
        if (mBirdMtxGlideCtrl)
            mStateEvent->setGlideCtrl(mBirdMtxGlideCtrl);

        al::initNerveState(this, mStateEvent, &NrvTalkNpc.Event, u8"通常イベント");
    } else {
        al::getStringArg(&eventName, info, "EventName");

        al::EventFlowExecutor* exec;
        exec = rs::initEventFlowSuffix(this, info, modelSuffix, "TalkNpc", eventName);
        if (mJointLookAtController)
            al::onExistLookAtJointCtrl(exec);
        rs::initEventParam(exec, mTalkNpcParam, actorParamSuffix);
        rs::initEventActionNameConverter(exec, mActionAnimInfo);
        al::initEventReceiver(exec, (al::IEventFlowEventReceiver*)this);

        if (rs::isDefinedEventCamera(exec, "Default"))
            rs::initEventCameraObject(exec, info, "Default");
        if (rs::isDefinedEventCamera(exec, "Focus"))
            rs::initEventCameraObject(exec, info, "Focus");
        if (rs::isDefinedEventCamera(exec, "AfterKeepPose"))
            rs::initEventCameraObjectAfterKeepPose(exec, info, "AfterKeepPose");

        const char* charLabel = nullptr;
        if (al::tryGetStringArg(&charLabel, info, "CharacterLabelName") && charLabel &&
            !al::isEqualString(charLabel, "None"))
            rs::initEventCharacterName(exec, info, charLabel);

        rs::tryInitItemKeeperByEvent(this, info, exec);

        mMovementType = initMovementType(exec, this, info, mTalkNpcParam, mActionAnimInfo);

        rs::startNpcAction(this, mActionAnimInfo->getWaitActionName());

        auto* stateEvent = new TalkNpcStateEvent(u8"通常イベント", this, mTalkNpcParam, exec, -1);
        mStateEvent = stateEvent;
        stateEvent->initStateReaction(stateReaction);
        mStateEvent->setAnimInfo(mActionAnimInfo);
        mStateEvent->setRandomActionUpdater(mRandomActionUpdater);
        if (mBirdMtxGlideCtrl)
            mStateEvent->setGlideCtrl(mBirdMtxGlideCtrl);
        if (mMovementType == 6)
            mStateEvent->setIsTrafficNpc(true);

        al::initNerveState(this, mStateEvent, &NrvTalkNpc.Event, u8"通常イベント");

        if (al::isModelName(this, "KinopioBrigade") &&
            !rs::isOnFlagKinopioBrigadeNpcFirstTalk(this)) {
            al::EventFlowExecutor* firstExec;
            firstExec = rs::initEventFlowSuffix(this, info, modelSuffix, "TalkNpc",
                                                "KinopioBrigadeFirstTalk");
            if (mJointLookAtController)
                al::onExistLookAtJointCtrl(firstExec);
            rs::initEventParam(firstExec, mTalkNpcParam, actorParamSuffix);
            s32 firstMovType =
                initMovementType(firstExec, this, info, mTalkNpcParam, mActionAnimInfo);
            rs::initEventActionNameConverter(firstExec, mActionAnimInfo);
            rs::initEventCameraObject(firstExec, info, "KinopioBrigadeFirstTalk");
            al::initEventReceiver(firstExec, (al::IEventFlowEventReceiver*)this);

            const char* charLabel2 = nullptr;
            if (al::tryGetStringArg(&charLabel2, info, "CharacterLabelName") && charLabel2 &&
                !al::isEqualString(charLabel2, "None"))
                rs::initEventCharacterName(firstExec, info, charLabel2);

            auto* firstState = new TalkNpcStateEvent(u8"キノピオ隊長初遭遇イベント", this,
                                                     mTalkNpcParam, firstExec, -1);
            mStateEventFirst = firstState;
            firstState->initStateReaction(stateReaction);
            mStateEventFirst->_5c = true;
            mStateEventFirst->setAnimInfo(mActionAnimInfo);
            mStateEventFirst->setRandomActionUpdater(mRandomActionUpdater);
            if (firstMovType == 6)
                mStateEventFirst->setIsTrafficNpc(true);
            if (mBirdMtxGlideCtrl)
                mStateEventFirst->setGlideCtrl(mBirdMtxGlideCtrl);

            al::initNerveState(this, mStateEventFirst, &NrvTalkNpc.KinopioBrigadeFirstEvent,
                               u8"キノピオ隊長初遭遇イベント");
            al::setNerve(this, &NrvTalkNpc.KinopioBrigadeFirstEvent);
        }

        if (TalkNpcFunction::tryGetHackingEventHackType(&mHackingEventHackType, info)) {
            const char* hackEventName = nullptr;
            al::getStringArg(&hackEventName, info, "HackingEventName");

            al::EventFlowExecutor* hackExec;
            hackExec =
                rs::initEventFlowSuffix(this, info, modelSuffix, "TalkNpcHacking", hackEventName);
            if (mJointLookAtController)
                al::onExistLookAtJointCtrl(hackExec);
            rs::initEventParam(hackExec, mTalkNpcParam, actorParamSuffix);
            s32 hackMovType =
                initMovementType(hackExec, this, info, mTalkNpcParam, mActionAnimInfo);
            rs::initEventActionNameConverter(hackExec, mActionAnimInfo);
            if (rs::isDefinedEventCamera(hackExec, "HackingEvent"))
                rs::initEventCameraObject(hackExec, info, "HackingEvent");
            al::initEventReceiver(hackExec, (al::IEventFlowEventReceiver*)this);

            const char* charLabel3 = nullptr;
            if (al::tryGetStringArg(&charLabel3, info, "CharacterLabelName") && charLabel3 &&
                !al::isEqualString(charLabel3, "None"))
                rs::initEventCharacterName(hackExec, info, charLabel3);

            auto* hackState = new TalkNpcStateEvent(u8"キャプチャイベント", this, mTalkNpcParam,
                                                    hackExec, mHackingEventHackType);
            mStateEventHacking = hackState;
            hackState->initStateReaction(stateReaction);
            mStateEventHacking->setAnimInfo(mActionAnimInfo);
            mStateEventHacking->setRandomActionUpdater(mRandomActionUpdater);
            if (mBirdMtxGlideCtrl)
                mStateEventHacking->setGlideCtrl(mBirdMtxGlideCtrl);
            if (hackMovType == 6)
                mStateEventHacking->setIsTrafficNpc(true);

            al::initNerveState(this, mStateEventHacking, &NrvTalkNpc.HackEvent,
                               u8"キャプチャイベント");

            mLinkAreaGroup =
                al::createLinkAreaGroup(this, info, "PlayerHackTypeCheckEventArea",
                                        u8"プレイヤー憑依タイプチェックイベントエリアグループ",
                                        u8"プレイヤー憑依タイプチェックイベントエリア");
        }
    }

    bool allWaitAtPoint;
    if (!mTalkNpcParam->mTrampleSensorName) {
        allWaitAtPoint = true;
    } else if (mStateEvent && !al::isWaitAtPointMovement(mStateEvent->mEventFlowExecutor)) {
        allWaitAtPoint = false;
    } else if (mStateEventHacking &&
               !al::isWaitAtPointMovement(mStateEventHacking->mEventFlowExecutor)) {
        allWaitAtPoint = false;
    } else if (mStateEventFirst &&
               !al::isWaitAtPointMovement(mStateEventFirst->mEventFlowExecutor)) {
        allWaitAtPoint = false;
    } else {
        allWaitAtPoint = true;
    }

    const char* balloonAction = nullptr;
    bool hasBalloon = false;
    if (al::tryGetStringArg(&balloonAction, info, "BalloonActionName") && balloonAction)
        hasBalloon = !al::isEqualString(balloonAction, "None");

    if (!allWaitAtPoint && mTalkNpcParam->tryInitPartialSklAnim(this)) {
        auto* partialAnim = new TalkNpcPartialAnimCtrl(this, mActionAnimInfo);
        mPartialAnimCtrl = partialAnim;
        partialAnim->mReactionParam = mActionAnimInfo->_18;

        if (!al::tryGetBoolArgOrFalse(info, "IsKeepMoveOnRail") &&
            mTalkNpcParam->tryGetByeByeBaseJointName(this)) {
            sead::Vector3f byeByeAxis;
            mTalkNpcParam->getByeByeLocalAxisFront(&byeByeAxis);
            mPartialAnimCtrl->mByeByeBaseJointName = mTalkNpcParam->tryGetByeByeBaseJointName(this);
            mPartialAnimCtrl->mByeByeLocalAxisFront = byeByeAxis;
        }

        mStateEvent->setPartialAnimCtrl(mPartialAnimCtrl);
        if (mStateEventHacking)
            mStateEventHacking->setPartialAnimCtrl(mPartialAnimCtrl);
        if (mStateEventFirst)
            mStateEventFirst->setPartialAnimCtrl(mPartialAnimCtrl);
    }

    if (hasBalloon)
        mRandomActionUpdater->initBalloonAction(balloonAction);

    al::registActorToDemoInfo(this, info);
    al::trySyncStageSwitchAppearAndKill(this);
}

// NON_MATCHING: material code logic structure differs from target
void TalkNpc::initAfterPlacement() {
    TalkNpcStateEvent* currentState = tryGetCurrentEventState();
    if (currentState) {
        currentState->initAfterPlacement();
        if (al::isDead(this))
            return;
    }

    if (al::isValidStageSwitch(this, "SwitchForceVanishAfterInit") &&
        al::isOnStageSwitch(this, "SwitchForceVanishAfterInit")) {
        makeActorDead();
        return;
    }

    rs::tryAttachConnectorToCollisionTFSV(this, mMtxConnector, &mConnectorQuat);

    if (mTalkNpcParam->mIsUpdateMaterialCode) {
        mNpcForceMaterialCodeAreaGroup = al::tryFindAreaObjGroup(this, "NpcForceMaterialCodeArea");
        if (al::isNoCollide(this) || !getCollider()) {
            if (mNpcForceMaterialCodeAreaGroup) {
                const al::AreaObj* areaObj =
                    al::tryGetAreaObj(mNpcForceMaterialCodeAreaGroup, al::getTrans(this));
                if (areaObj) {
                    const char* matCode = nullptr;
                    al::tryGetAreaObjStringArg(&matCode, areaObj, "MaterialCode");
                    if (matCode)
                        al::setMaterialCode(this, matCode);
                } else {
                    al::resetMaterialCode(this);
                }
            } else {
                rs::tryUpdateMaterialCodeByFloorCollisionOnArrow(this);
            }
        }
    }

    GameDataHolderAccessor accessor(this);
    const char* worldName = GameDataFunction::getWorldDevelopNameCurrent(accessor);
    if (rs::checkGetShineForWorldTravelingPeach(accessor, worldName))
        mStateEvent->trySwitchEventEntry("AfterGetWorldTravelingPeachShine");
}

TalkNpcStateEvent* TalkNpc::tryGetCurrentEventState() const {
    if (al::isNerve(this, &NrvTalkNpc.Event))
        return mStateEvent;
    if (al::isNerve(this, &NrvTalkNpc.HackEvent))
        return mStateEventHacking;
    if (al::isNerve(this, &NrvTalkNpc.KinopioBrigadeFirstEvent))
        return mStateEventFirst;
    return nullptr;
}

al::EventFlowExecutor* TalkNpc::tryGetCurrentEvent() const {
    TalkNpcStateEvent* state = tryGetCurrentEventState();
    if (state)
        return state->mEventFlowExecutor;
    return nullptr;
}

// NON_MATCHING: complex sub-actor anim sync logic
// NON_MATCHING: register allocation, sub-actor sync logic structure differs
void TalkNpc::movement() {
    if (mPlayerEyeSensorHitHolder)
        mPlayerEyeSensorHitHolder->clear();

    if (mJointLookAtController)
        rs::updateNpcJointLookAtController(mJointLookAtController);

    if (mPartialAnimCtrl)
        mPartialAnimCtrl->update();

    if (getHitSensorKeeper()) {
        const sead::Vector3f& playerPos = rs::getPlayerPos(this);
        const sead::Vector3f& trans = al::getTrans(this);
        f32 dx = playerPos.x - trans.x;
        f32 dy = playerPos.y - trans.y;
        f32 dz = playerPos.z - trans.z;
        f32 dist = sead::Mathf::sqrt(dx * dx + dy * dy + dz * dz);
        if (dist >= 3000.0f) {
            if (!mIsEyeSensorInvalid) {
                mIsEyeSensorInvalid = true;
                al::invalidateHitSensorEyeAll(this);
            }
        } else if (mIsEyeSensorInvalid) {
            mIsEyeSensorInvalid = false;
            al::validateHitSensorEyeAll(this);
        }
    }

    al::LiveActor::movement();

    if (mTalkNpcParam->mIsUpdateMaterialCode && (al::isNoCollide(this) || !getCollider())) {
        TalkNpcStateEvent* state = tryGetCurrentEventState();
        if (state && state->mEventFlowExecutor &&
            !al::isWaitAtPointMovement(state->mEventFlowExecutor)) {
            if (mNpcForceMaterialCodeAreaGroup) {
                const al::AreaObj* areaObj =
                    al::tryGetAreaObj(mNpcForceMaterialCodeAreaGroup, al::getTrans(this));
                if (areaObj) {
                    const char* matCode = nullptr;
                    al::tryGetAreaObjStringArg(&matCode, areaObj, "MaterialCode");
                    if (matCode)
                        al::setMaterialCode(this, matCode);
                } else {
                    al::resetMaterialCode(this);
                }
            } else {
                rs::tryUpdateMaterialCodeByFloorCollisionOnArrow(this);
            }
        }
    }

    rs::tryUpdateNpcEyeLineAnim(this, mTalkNpcParam);

    if (al::isExistSubActorKeeper(this)) {
        auto* syncHolder = (SubActorSyncData*)mSubActorSyncHolder;
        if (syncHolder->mHasCityMayorFace) {
            rs::syncActionCityMayorFace(syncHolder->mActor);
            rs::syncMtsAnimCityMayorFace(syncHolder->mActor);
        }

        if (al::isSklAnimPlaying(syncHolder->mActor, 0)) {
            const char* cachedSklAnim = syncHolder->mAnimCache->mSklAnimName;
            const char* currentSklAnim = al::getPlayingSklAnimName(syncHolder->mActor, 0);

            if (cachedSklAnim != currentSklAnim)
                syncHolder->mAnimCache->mSklAnimName = currentSklAnim;

            for (s32 i = 0; i < syncHolder->mSubActorCount; i++) {
                al::LiveActor* sub = syncHolder->mSubActors[i];
                const char* animName = al::getPlayingSklAnimName(syncHolder->mActor, 0);

                if (cachedSklAnim != currentSklAnim) {
                    if (al::isExistAction(sub, animName) && !al::isActionPlaying(sub, animName))
                        al::startAction(sub, animName);
                }

                if (al::isActionPlaying(sub, animName)) {
                    f32 frame = al::getSklAnimFrame(syncHolder->mActor, 0);
                    f32 maxFrame = al::getActionFrameMax(sub, animName);
                    if (frame <= maxFrame)
                        al::setActionFrame(sub, frame);
                }
            }
        }

        if (al::isMtsAnimPlaying(syncHolder->mActor)) {
            const char* cachedMtsAnim = syncHolder->mAnimCache->mMtsAnimName;
            const char* currentMtsAnim = al::getPlayingMtsAnimName(syncHolder->mActor);

            if (cachedMtsAnim != currentMtsAnim)
                syncHolder->mAnimCache->mMtsAnimName = currentMtsAnim;

            for (s32 i = 0; i < syncHolder->mSubActorCount; i++) {
                al::LiveActor* sub = syncHolder->mSubActors[i];
                const char* mtsName = al::getPlayingMtsAnimName(syncHolder->mActor);

                if (cachedMtsAnim != currentMtsAnim) {
                    if (al::isMtsAnimExist(sub, mtsName) && !al::isMtsAnimPlaying(sub, mtsName)) {
                        if (!al::isSklAnimPlaying(sub, 0) ||
                            !al::isMtsAnimExist(sub, al::getPlayingSklAnimName(sub, 0)))
                            al::startMtsAnim(sub, mtsName);
                    }
                }

                if (al::isMtsAnimExist(sub, mtsName) && al::isMtsAnimPlaying(sub, mtsName)) {
                    f32 frame = al::getMtsAnimFrame(syncHolder->mActor);
                    f32 maxFrame = al::getMtsAnimFrameMax(sub, mtsName);
                    if (frame < maxFrame)
                        al::setMtsAnimFrameAndStop(sub, frame);
                }
            }
        }
    }

    rs::tryUpdateNpcFacialAnim(this, mTalkNpcParam);

    if (mBirdMtxGlideCtrl)
        mBirdMtxGlideCtrl->update();

    rs::trySwitchDepthToSelfShadow(this);
}

// NON_MATCHING: register allocation in csel/csinc for frame counter
void TalkNpc::calcAnim() {
    al::LiveActor::calcAnim();

    if (mHipJointController) {
        auto* hip = (TrampleReactionJointCtrl*)mHipJointController;
        if (hip->mAnimFrame >= 0) {
            s32 maxFrame = hip->mIsCapReaction ? 70 : 10;
            if (maxFrame > hip->mAnimFrame + 1)
                hip->mAnimFrame++;
            else
                hip->mAnimFrame = -1;
        }
    }

    if (mSpineJointController) {
        auto* spine = (CapReactionJointCtrl*)mSpineJointController;
        if (spine->mAnimFrame >= 0)
            spine->mAnimFrame = spine->mAnimFrame > 68 ? -1 : spine->mAnimFrame + 1;
    }

    if (mBirdMtxGlideCtrl)
        mTalkNpcParam->calcBirdGlideMtx(&mBirdGlideMtx, this);
}

// NON_MATCHING: getTransPtr hoisting in collide path, register allocation
void TalkNpc::updateCollider() {
    if (al::isNoCollide(this) || !getCollider()) {
        sead::Vector3f* transPtr = al::getTransPtr(this);
        const sead::Vector3f& vel = al::getVelocity(this);
        transPtr->x += vel.x;
        transPtr->y += vel.y;
        transPtr->z += vel.z;
        if (mMovementType == 3)
            al::setTransY(this, mInitTrans.y);
        return;
    }

    sead::Vector3f* transPtr = al::getTransPtr(this);
    sead::Vector3f collideResult = getCollider()->collide(al::getVelocity(this));
    transPtr->x += collideResult.x;
    transPtr->y += collideResult.y;
    transPtr->z += collideResult.z;

    if (mTalkNpcParam->mIsUpdateMaterialCode) {
        if (al::isCollidedGround(this))
            al::setMaterialCode(this, al::getCollidedFloorMaterialCodeName(this));
        else
            al::resetMaterialCode(this);
    }
}

void TalkNpc::kill() {
    al::LiveActor::kill();
    al::tryOnSwitchDeadOn(this);
}

void TalkNpc::endClipped() {
    al::LiveActor::endClipped();
    if (mBgmAnimeSynchronizer)
        *(bool*)((char*)mBgmAnimeSynchronizer + 40) = true;
}

void TalkNpc::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isSensorEye(self)) {
        TalkNpcStateEvent* state = tryGetCurrentEventState();
        if (state && state->mEventFlowExecutor) {
            if (rs::sendMsgEventFlowScareCheck(other, self, state->mEventFlowExecutor))
                return;
        }

        if (!_236 && mPlayerEyeSensorHitHolder) {
            if (mPlayerEyeSensorHitHolder->isEnableRecordLookAt(other, self))
                mPlayerEyeSensorHitHolder->tryRecordLookAtTarget(other, self);
        }
        return;
    }

    if (rs::sendMsgPushToFish(other, self))
        return;
    rs::attackSensorNpcCommon(self, other);
    al::sendMsgNpcTouch(other, self);
}

// NON_MATCHING: quaternion math for cap/trample reactions
// NON_MATCHING: register allocation throughout, quaternion math inlines
bool TalkNpc::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) {
    if (rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;

    if (al::isMsgPlayerDisregard(msg))
        return mTalkNpcParam->isPlayerWatchDisregard(self);

    if (mPartialAnimCtrl && mPartialAnimCtrl->mReactionParam && mPartialAnimCtrl->isReactionCap()) {
        if (rs::checkMsgNpcTrampleReactionAll(msg, other, self, false)) {
            auto* hip = (TrampleReactionJointCtrl*)mHipJointController;
            hip->mAnimFrame = 0;
            hip->mIsCapReaction = false;
            mPartialAnimCtrl->startReaction();
            rs::requestHitReactionToAttacker(msg, other, self);
            return true;
        }

        if (rs::isMsgTankBulletNoReaction(msg) || rs::isMsgKillerAttackNoExplode(msg) ||
            rs::isMsgNpcCapReactionAll(msg)) {
            if (rs::isMsgPlayerObjectWallHit(msg) && mPartialAnimCtrl->isReactionCap())
                return false;

            mPartialAnimCtrl->startReactionCap();
            auto* hip = (TrampleReactionJointCtrl*)mHipJointController;
            hip->mAnimFrame = 0;
            hip->mIsCapReaction = true;

            sead::Vector3f dir = {0.0f, 0.0f, 0.0f};
            al::calcDirBetweenSensorsH(&dir, other, self);

            auto* spine = (CapReactionJointCtrl*)mSpineJointController;
            spine->mAnimFrame = 0;
            const sead::Vector3f& grav = al::getGravity(this);
            spine->mRotatedAxis.x = dir.y * grav.z - dir.z * grav.y;
            spine->mRotatedAxis.y = dir.z * grav.x - dir.x * grav.z;
            spine->mRotatedAxis.z = dir.x * grav.y - dir.y * grav.x;

            sead::Quatf baseQuat;
            spine->mBaseMtx->toQuat(baseQuat);

            sead::Quatf tmp;
            tmp.x = baseQuat.w * spine->mRotatedAxis.x + baseQuat.y * spine->mRotatedAxis.z -
                    baseQuat.z * spine->mRotatedAxis.y;
            tmp.y = baseQuat.w * spine->mRotatedAxis.y - baseQuat.x * spine->mRotatedAxis.z +
                    baseQuat.z * spine->mRotatedAxis.x;
            tmp.z = baseQuat.w * spine->mRotatedAxis.z + baseQuat.x * spine->mRotatedAxis.y -
                    baseQuat.y * spine->mRotatedAxis.x;
            tmp.w = -baseQuat.x * spine->mRotatedAxis.x - baseQuat.y * spine->mRotatedAxis.y -
                    baseQuat.z * spine->mRotatedAxis.z;
            spine->mRotatedAxis.x =
                tmp.w * baseQuat.x + tmp.x * baseQuat.w - tmp.y * baseQuat.z + tmp.z * baseQuat.y;
            spine->mRotatedAxis.y =
                tmp.w * baseQuat.y + tmp.x * baseQuat.z + tmp.y * baseQuat.w - tmp.z * baseQuat.x;
            spine->mRotatedAxis.z =
                tmp.w * baseQuat.z - tmp.x * baseQuat.y + tmp.y * baseQuat.x + tmp.z * baseQuat.w;
            al::normalize(&spine->mRotatedAxis);

            sead::Quatf dirQuat;
            dirQuat.x = baseQuat.w * dir.y - baseQuat.z * dir.z + dir.x * baseQuat.y;
            dirQuat.y = dir.y * baseQuat.x + dir.z * baseQuat.w - dir.x * baseQuat.z;
            dirQuat.z = dir.z * baseQuat.y + dir.x * baseQuat.w + dir.y * baseQuat.z;
            dirQuat.w = -dir.x * baseQuat.x - dir.y * baseQuat.y - dir.z * baseQuat.z;
            spine->mAxis.x = dirQuat.w * baseQuat.x + dirQuat.x * baseQuat.w -
                             dirQuat.y * baseQuat.z + dirQuat.z * baseQuat.y;
            spine->mAxis.y = dirQuat.w * baseQuat.y + dirQuat.x * baseQuat.z +
                             dirQuat.y * baseQuat.w - dirQuat.z * baseQuat.x;
            spine->mAxis.z = dirQuat.w * baseQuat.z - dirQuat.x * baseQuat.y +
                             dirQuat.y * baseQuat.x + dirQuat.z * baseQuat.w;

            rs::requestHitReactionToAttacker(msg, other, self);
            return true;
        }

        return rs::receiveMsgNpcCommonNoReaction(msg, other, self);
    }

    if (mIsInvalidTrample || rs::isInvalidTrampleSensor(self, mTalkNpcParam)) {
        TalkNpcStateEvent* state = tryGetCurrentEventState();
        if (state) {
            if (state->tryStartReactionWithoutTrample(msg, other, self)) {
                if (mSpineJointController && state->isCapReaction()) {
                    sead::Vector3f dir = {0.0f, 0.0f, 0.0f};
                    al::calcDirBetweenSensorsH(&dir, other, self);
                    auto* spine = (CapReactionJointCtrl*)mSpineJointController;
                    spine->mAnimFrame = 0;
                    const sead::Vector3f& grav = al::getGravity(this);
                    spine->mRotatedAxis.x = dir.y * grav.z - dir.z * grav.y;
                    spine->mRotatedAxis.y = dir.z * grav.x - dir.x * grav.z;
                    spine->mRotatedAxis.z = dir.x * grav.y - dir.y * grav.x;

                    sead::Quatf baseQuat;
                    spine->mBaseMtx->toQuat(baseQuat);

                    sead::Quatf tmp;
                    tmp.x = baseQuat.w * spine->mRotatedAxis.x +
                            baseQuat.y * spine->mRotatedAxis.z - baseQuat.z * spine->mRotatedAxis.y;
                    tmp.y = baseQuat.w * spine->mRotatedAxis.y -
                            baseQuat.x * spine->mRotatedAxis.z + baseQuat.z * spine->mRotatedAxis.x;
                    tmp.z = baseQuat.w * spine->mRotatedAxis.z +
                            baseQuat.x * spine->mRotatedAxis.y - baseQuat.y * spine->mRotatedAxis.x;
                    tmp.w = -baseQuat.x * spine->mRotatedAxis.x -
                            baseQuat.y * spine->mRotatedAxis.y - baseQuat.z * spine->mRotatedAxis.z;
                    spine->mRotatedAxis.x = tmp.w * baseQuat.x + tmp.x * baseQuat.w -
                                            tmp.y * baseQuat.z + tmp.z * baseQuat.y;
                    spine->mRotatedAxis.y = tmp.w * baseQuat.y + tmp.x * baseQuat.z +
                                            tmp.y * baseQuat.w - tmp.z * baseQuat.x;
                    spine->mRotatedAxis.z = tmp.w * baseQuat.z - tmp.x * baseQuat.y +
                                            tmp.y * baseQuat.x + tmp.z * baseQuat.w;
                    al::normalize(&spine->mRotatedAxis);

                    sead::Quatf dirQ;
                    dirQ.x = baseQuat.w * dir.y - baseQuat.z * dir.z + dir.x * baseQuat.y;
                    dirQ.y = dir.y * baseQuat.x + dir.z * baseQuat.w - dir.x * baseQuat.z;
                    dirQ.z = dir.z * baseQuat.y + dir.x * baseQuat.w + dir.y * baseQuat.z;
                    dirQ.w = -dir.x * baseQuat.x - dir.y * baseQuat.y - dir.z * baseQuat.z;
                    spine->mAxis.x = dirQ.w * baseQuat.x + dirQ.x * baseQuat.w -
                                     dirQ.y * baseQuat.z + dirQ.z * baseQuat.y;
                    spine->mAxis.y = dirQ.w * baseQuat.y + dirQ.x * baseQuat.z +
                                     dirQ.y * baseQuat.w - dirQ.z * baseQuat.x;
                    spine->mAxis.z = dirQ.w * baseQuat.z - dirQ.x * baseQuat.y +
                                     dirQ.y * baseQuat.x + dirQ.z * baseQuat.w;
                }
                return true;
            }
        }
        return rs::receiveMsgNpcCommonNoReaction(msg, other, self);
    }

    TalkNpcStateEvent* state = tryGetCurrentEventState();
    if (state) {
        if (state->tryStartReaction(msg, other, self)) {
            if (mSpineJointController && state->isCapReaction()) {
                sead::Vector3f dir = {0.0f, 0.0f, 0.0f};
                al::calcDirBetweenSensorsH(&dir, other, self);
                auto* spine = (CapReactionJointCtrl*)mSpineJointController;
                spine->mAnimFrame = 0;
                const sead::Vector3f& grav = al::getGravity(this);
                spine->mRotatedAxis.x = dir.y * grav.z - dir.z * grav.y;
                spine->mRotatedAxis.y = dir.z * grav.x - dir.x * grav.z;
                spine->mRotatedAxis.z = dir.x * grav.y - dir.y * grav.x;

                sead::Quatf baseQuat;
                spine->mBaseMtx->toQuat(baseQuat);

                sead::Quatf tmp;
                tmp.x = baseQuat.w * spine->mRotatedAxis.x + baseQuat.y * spine->mRotatedAxis.z -
                        baseQuat.z * spine->mRotatedAxis.y;
                tmp.y = baseQuat.w * spine->mRotatedAxis.y - baseQuat.x * spine->mRotatedAxis.z +
                        baseQuat.z * spine->mRotatedAxis.x;
                tmp.z = baseQuat.w * spine->mRotatedAxis.z + baseQuat.x * spine->mRotatedAxis.y -
                        baseQuat.y * spine->mRotatedAxis.x;
                tmp.w = -baseQuat.x * spine->mRotatedAxis.x - baseQuat.y * spine->mRotatedAxis.y -
                        baseQuat.z * spine->mRotatedAxis.z;
                spine->mRotatedAxis.x = tmp.w * baseQuat.x + tmp.x * baseQuat.w -
                                        tmp.y * baseQuat.z + tmp.z * baseQuat.y;
                spine->mRotatedAxis.y = tmp.w * baseQuat.y + tmp.x * baseQuat.z +
                                        tmp.y * baseQuat.w - tmp.z * baseQuat.x;
                spine->mRotatedAxis.z = tmp.w * baseQuat.z - tmp.x * baseQuat.y +
                                        tmp.y * baseQuat.x + tmp.z * baseQuat.w;
                al::normalize(&spine->mRotatedAxis);

                sead::Quatf dirQ;
                dirQ.x = baseQuat.w * dir.y - baseQuat.z * dir.z + dir.x * baseQuat.y;
                dirQ.y = dir.y * baseQuat.x + dir.z * baseQuat.w - dir.x * baseQuat.z;
                dirQ.z = dir.z * baseQuat.y + dir.x * baseQuat.w + dir.y * baseQuat.z;
                dirQ.w = -dir.x * baseQuat.x - dir.y * baseQuat.y - dir.z * baseQuat.z;
                spine->mAxis.x = dirQ.w * baseQuat.x + dirQ.x * baseQuat.w - dirQ.y * baseQuat.z +
                                 dirQ.z * baseQuat.y;
                spine->mAxis.y = dirQ.w * baseQuat.y + dirQ.x * baseQuat.z + dirQ.y * baseQuat.w -
                                 dirQ.z * baseQuat.x;
                spine->mAxis.z = dirQ.w * baseQuat.z - dirQ.x * baseQuat.y + dirQ.y * baseQuat.x +
                                 dirQ.z * baseQuat.w;
            }
            rs::requestHitReactionToAttacker(msg, other, self);
            return true;
        }
    }

    return rs::receiveMsgNpcCommonNoReaction(msg, other, self);
}

bool TalkNpc::receiveEvent(const al::EventFlowEventData* data) {
    if (TalkNpcFunction::receiveEventChangeWaitAction(mActionAnimInfo, data, mTalkNpcParam))
        return true;

    if (al::isEventName(data, "ChangeHackWaitAction")) {
        const char* actionName = al::getEventDataParamString(data, "ActionName");
        mActionAnimInfo->changeHackWaitActionName(actionName, mTalkNpcParam);
        return true;
    }

    if (al::isEventName(data, "PlayActionOneTime")) {
        const char* actionName = al::getEventDataParamString(data, "ActionName");
        mRandomActionUpdater->startActionOneTime(actionName);
        return true;
    }

    if (al::isEventName(data, "SyncEntry")) {
        const char* entryName = al::getEventDataParamString(data, "EntryName");
        if (mStateEvent && !mStateEvent->isDead())
            mStateEvent->trySwitchEventEntry(entryName);
        if (mStateEventHacking && !mStateEventHacking->isDead())
            mStateEventHacking->trySwitchEventEntry(entryName);
        if (mStateEventFirst && !mStateEventFirst->isDead())
            mStateEventFirst->trySwitchEventEntry(entryName);
        return true;
    }

    if (al::isEventName(data, "OnStartWorldTravelingPeach")) {
        rs::startWorldTravelingPeach(this);
        return true;
    }

    if (al::isEventName(data, "CheckPlayerHackTypeAndOnGround")) {
        if (!rs::isPlayerHack(this) || !rs::isPlayerHackType(this, mHackingEventHackType))
            return false;

        al::AreaObjGroup* areaGroup = (al::AreaObjGroup*)mLinkAreaGroup;
        if (!areaGroup || areaGroup->getSize() < 1)
            return false;

        for (s32 i = 0; i < areaGroup->getSize(); i++) {
            const al::AreaObj* areaObj = areaGroup->getAreaObj(i);
            const sead::Vector3f& playerPos = rs::getPlayerPos(this);
            if (al::isInAreaPos(areaObj, playerPos)) {
                s32 triggerType = 0;
                al::tryGetAreaObjArg(&triggerType, areaObj, "TriggerType");
                if (triggerType == 1) {
                    if (al::isInWaterPos(this, playerPos)) {
                        s32 count = mWaterCheckCount;
                        mWaterCheckCount = count + 1;
                        return count > 43;
                    }
                } else {
                    if (rs::isPlayerOnGround(this))
                        return true;
                }
            }
        }

        mWaterCheckCount = 0;
        return false;
    }

    if (al::isEventName(data, "CheckEnableConnectNormalEventToSeamless"))
        return mStateEvent->trySwitchEventEntryAndUpdate("AfterKinopioBrigadeFirstTalk");

    if (al::isEventName(data, "CheckTurnMovement"))
        return mMovementType == 1;

    return false;
}

void TalkNpc::onFukankunCameraNearClipSetting() {
    auto* info = (ClippingNearClipInfo*)mClippingInfo;
    *(f32*)((char*)info->mClippingActorInfo + 56) = 300.0f;
}

void TalkNpc::offFukankunCameraNearClipSetting() {
    auto* info = (ClippingNearClipInfo*)mClippingInfo;
    void* clippingActorInfo = info->mClippingActorInfo;
    *(f32*)((char*)clippingActorInfo + 56) = info->mSavedNearClip;
}

void TalkNpc::startDemoKoopa(const sead::Vector3f& pos) {
    const sead::Vector3f& trans = al::getTrans(this);
    sead::Vector3f dir = {pos.x - trans.x, pos.y - trans.y, pos.z - trans.z};
    al::verticalizeVec(&dir, al::getGravity(this), dir);
    if (!al::tryNormalizeOrZero(&dir))
        dir.set(al::getFront(this));
    al::setFront(this, dir);
    al::setNerve(this, &DemoKoopa);
}

void TalkNpc::endDemoKoopa() {
    al::setFront(this, mInitFront);
    al::setNerve(this, &NrvTalkNpc.Event);
}

void TalkNpc::exeEvent() {
    rs::tryConnectToCollisionTFSV(this, mMtxConnector, &mConnectorQuat);

    TalkNpcStateEvent* hackState = mStateEventHacking;
    s32 hackType = mHackingEventHackType;
    bool isDemo = rs::isActiveEventDemo(this);

    if (hackState && !isDemo && rs::isPlayerHackType(this, hackType)) {
        al::EventFlowExecutor* exec = hackState->mEventFlowExecutor;
        const sead::Vector3f& playerPos = rs::getPlayerPos(this);
        if (al::checkInsideTerritoryPos(exec, this, playerPos, 0.0f)) {
            al::setNerve(this, &NrvTalkNpc.HackEvent);
            return;
        }
    }

    al::updateNerveState(this);

    const char* volleyBall = rs::tryGetTalkNpcVolleyBallEntryName(this);
    const char* jumpingRope = rs::tryGetTalkNpcJumpingRopeEntryName(this);
    const char* radicon = rs::tryGetTalkNpcRadiconEntryName(this);

    if (volleyBall)
        mStateEvent->trySwitchEventEntry(volleyBall);
    if (jumpingRope)
        mStateEvent->trySwitchEventEntry(jumpingRope);
    if (radicon)
        mStateEvent->trySwitchEventEntry(radicon);

    if (mBgmAnimeSynchronizer)
        mBgmAnimeSynchronizer->trySyncBgm();
}

void TalkNpc::exeHackEvent() {
    rs::tryConnectToCollisionTFSV(this, mMtxConnector, &mConnectorQuat);

    if (rs::isActiveEventDemo(this)) {
        al::updateNerveState(this);
        return;
    }

    TalkNpcStateEvent* hackState = mStateEventHacking;
    if (hackState) {
        auto* areaGroup = (al::AreaObjGroup*)mLinkAreaGroup;
        if (rs::isPlayerHackType(this, mHackingEventHackType)) {
            bool inArea = false;
            if (areaGroup) {
                const sead::Vector3f& playerPos = rs::getPlayerPos(this);
                inArea = al::isInAreaObj(areaGroup, playerPos);
            }
            if (inArea) {
                al::updateNerveState(this);
                return;
            }
            al::EventFlowExecutor* exec = hackState->mEventFlowExecutor;
            const sead::Vector3f& playerPos = rs::getPlayerPos(this);
            if (al::checkInsideTerritoryPos(exec, this, playerPos, 0.0f)) {
                al::updateNerveState(this);
                return;
            }
        }
    }

    al::setNerve(this, &NrvTalkNpc.Event);
}

void TalkNpc::exeKinopioBrigadeFirstEvent() {
    rs::tryConnectToCollisionTFSV(this, mMtxConnector, &mConnectorQuat);

    if (al::updateNerveState(this)) {
        rs::onFlagKinopioBrigadeNpcFirstTalk(this);

        al::EventFlowExecutor* mainExec = mStateEvent->mEventFlowExecutor;
        const sead::Vector3f& recordFront =
            al::getRecordActorFront(mStateEventFirst->mEventFlowExecutor);
        al::recordActorFront(mainExec, recordFront);

        al::setNerve(this, &NrvTalkNpc.Event);
    }
}

void TalkNpc::exeDemoKoopa() {
    if (al::isFirstStep(this))
        rs::startNpcAction(this, "Wait");
}
