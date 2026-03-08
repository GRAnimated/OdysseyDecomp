#include "Npc/OpeningStageStartCapManHero.h"

#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "Library/Anim/AnimPlayerSkl.h"
#include "Library/Base/StringUtil.h"
#include "Library/Camera/CameraUtil.h"
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
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSceneFunction.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Obj/PartsModel.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementInfo.h"
#include "Library/Player/PlayerUtil.h"
#include "Library/Rail/RailKeeper.h"
#include "Library/Rail/RailUtil.h"
#include "Library/Resource/ResourceFunction.h"
#include "Library/Stage/StageSwitchUtil.h"
#include "Library/Thread/FunctorV0M.h"

#include "Npc/EventActorStateDemoTalk.h"
#include "Npc/SimpleActionLiveActor.h"
#include "Npc/SimpleActionPartsModel.h"
#include "Scene/QuestInfoHolder.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "System/GameDataHolderWriter.h"
#include "Util/CapManHeroDemoUtil.h"
#include "Util/DemoUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerDemoUtil.h"
#include "Util/PlayerUtil.h"

namespace {
NERVE_IMPL(OpeningStageStartCapManHero, Wander);
NERVE_IMPL(OpeningStageStartCapManHero, EventKoopaDemo);
NERVE_IMPL(OpeningStageStartCapManHero, EventTalkDemo);
NERVE_IMPL(OpeningStageStartCapManHero, RunAway);
NERVE_IMPL(OpeningStageStartCapManHero, NoticePlayerStand);
NERVE_IMPL(OpeningStageStartCapManHero, WaitPlayerTurn);
NERVE_IMPL(OpeningStageStartCapManHero, Surprise);
NERVE_IMPL(OpeningStageStartCapManHero, WaitBridge);
NERVE_IMPL_(OpeningStageStartCapManHero, WanderReverse, Wander);
NERVE_IMPL_(OpeningStageStartCapManHero, WanderDoubtTouch, WanderDoubtTouch);
NERVE_IMPL_(OpeningStageStartCapManHero, WanderDoubtTouchReverse, WanderDoubtTouch);
NERVE_IMPL(OpeningStageStartCapManHero, RunAwayWait);
NERVES_MAKE_STRUCT(OpeningStageStartCapManHero, Wander, EventKoopaDemo, EventTalkDemo, RunAway,
                   NoticePlayerStand, WaitPlayerTurn, Surprise, WaitBridge, WanderReverse,
                   WanderDoubtTouch, WanderDoubtTouchReverse, RunAwayWait);
}  // namespace

struct TransQuat {
    sead::Vector3f trans;
    sead::Quatf quat;
};

SimpleActionLiveActor::SimpleActionLiveActor(const char* name) : al::LiveActor(name) {}

void SimpleActionLiveActor::control() {
    if (mPendingAction) {
        al::startAction(this, mPendingAction);
        mPendingAction = nullptr;
    }
}

SimpleActionPartsModel::SimpleActionPartsModel(const char* name) : al::PartsModel(name) {}

void SimpleActionPartsModel::control() {
    if (mIsActionPending) {
        if (al::tryStartAction(this, mActionName.cstr()))
            al::showModelIfHide(this);
        mIsActionPending = false;
    }
    if (mIsBlendWeightDirty) {
        al::setSklAnimBlendWeightDouble(this, mBlendWeight0, mBlendWeight1);
        mIsBlendWeightDirty = false;
    }
    if (mIsClearInterpole) {
        mIsClearInterpole = false;
        al::clearSklAnimInterpole(this);
    }
}

// NON_MATCHING: VTT setup differences in multiple-inheritance constructor
OpeningStageStartCapManHero::OpeningStageStartCapManHero(const char* name) : al::LiveActor(name) {}

// NON_MATCHING: regalloc, imm/offset differences in large init function
void OpeningStageStartCapManHero::init(const al::ActorInitInfo& initInfo) {
    al::initActorWithArchiveName(this, initInfo, "CapWorldOpening", nullptr);
    al::initNerve(this, &NrvOpeningStageStartCapManHero.Wander, 1);
    al::invalidateClipping(this);
    al::initJointControllerKeeper(this, 1);
    CapManHeroDemoUtil::initCapManHeroTailJointController(this);
    mQuestInfo = rs::tryCreateAndRegisterQuestInfoToHolder(this, initInfo);

    GameDataHolderAccessor accessor(this);
    if (GameDataFunction::isEnableCap(accessor)) {
        mIsStartDemo = false;
        al::tryOnStageSwitch(this, "SwitchDemoEndOn");
        if (mQuestInfo)
            rs::invalidateQuest(mQuestInfo);
        makeActorAlive();
        return;
    }

    mDemoTalkState = new EventActorStateDemoTalk(this);

    al::tryGetLinksMatrixTR(&mKoopaDemoMtx, initInfo, "LinkKoopaDemoMtx");
    mDemoActorHolder = al::alDemoFunction::createDemoActorHolder("DemoMeetCapNpcSubStage", initInfo,
                                                                 &mKoopaDemoMtx, 1, false);

    const al::Resource* demoCameraRes = al::findOrCreateResource("ObjectData/DemoCamera", nullptr);
    mDemoAnimCamera = al::initDemoAnimCamera(this, initInfo, demoCameraRes,
                                             reinterpret_cast<const sead::Matrix34f*>(getName()),
                                             "DemoMeetFirstCamera");

    al::FunctorV0M<OpeningStageStartCapManHero*, void (OpeningStageStartCapManHero::*)()> functor(
        this, &OpeningStageStartCapManHero::startDemo);
    al::listenStageSwitchOnStart(this, functor);

    mAddDemoInfo = al::registDemoRequesterToAddDemoInfo(this, initInfo, 0);

    auto* playerRestartPos = new TransQuat();
    mPlayerRestartPos = playerRestartPos;

    if (al::calcLinkChildNum(initInfo, "PlayerRestartPos") >= 1) {
        auto* restart = static_cast<TransQuat*>(mPlayerRestartPos);
        al::tryGetLinksQT(&restart->quat, &restart->trans, initInfo, "PlayerRestartPos");

        al::PlacementInfo placementInfo;
        al::getLinksInfoByIndex(&placementInfo, initInfo, "PlayerRestartPos", 0);
        bool isValidEntranceCamera = false;
        al::tryGetArg(&isValidEntranceCamera, placementInfo, "IsValidEntranceCamera");
        if (isValidEntranceCamera) {
            const al::PlacementInfo& actorPlacementInfo = al::getPlacementInfo(initInfo);
            mEntranceCamera =
                al::initEntranceCamera(this, actorPlacementInfo, "DemoEndCameraCapManHero");
        }
    }

    auto* talkStartPos = new TransQuat();
    mTalkStartPos = talkStartPos;
    al::tryGetLinksQT(&talkStartPos->quat, &talkStartPos->trans, initInfo, "LinkTalkStartPos");

    mEventFlowExecutor = rs::initEventFlow(this, initInfo, "CapManHero", "MeetFirst");
    al::initEventReceiver(mEventFlowExecutor, static_cast<al::IEventFlowEventReceiver*>(this));

    const al::PlacementInfo& placementInfo = al::getPlacementInfo(initInfo);
    mWaitRailKeeper = al::tryCreateRailKeeper(placementInfo, "WaitRail");

    auto* marioCap = new al::LiveActor("装着表示用帽子");
    mMarioCap = marioCap;
    al::initChildActorWithArchiveNameWithPlacementInfo(marioCap, initInfo, "MarioCap", "Display");
    mMarioCap->makeActorDead();

    mMarioCapEye = al::getSubActor(mMarioCap, "ロックオン用キャップの目");
    mMarioCapEye->makeActorDead();

    auto* capBroken = new SimpleActionPartsModel("DemoMarioCapBroken");
    mCapBroken = capBroken;
    capBroken->initPartsFixFile(this, initInfo, "DemoMarioCapBroken", nullptr, "CapBroken");
    al::startAction(mCapBroken, "Wait");

    mJointSpringControllerHolder =
        al::JointSpringControllerHolder::tryCreateAndInitJointControllerKeeper(
            mCapBroken, "InitJointSpringCtrl");

    sead::Vector3f dir = {0.0f, 0.0f, 1.0f};
    al::rotateVectorDegreeY(&dir, 177.5f);

    const sead::Vector3f& playerPos = rs::getPlayerPos(this);
    sead::Vector3f pos;
    pos.x = dir.x * 200.0f + playerPos.x;
    pos.y = dir.y * 200.0f + playerPos.y;
    pos.z = dir.z * 200.0f + playerPos.z;
    al::setTrans(this, pos);

    if (mQuestInfo)
        reinterpret_cast<sead::Vector3f*>(mQuestInfo)[1] = al::getTrans(this);

    makeActorAlive();
}

// NON_MATCHING: Vector3f copy codegen (word-by-word vs coalesced qword)
void OpeningStageStartCapManHero::control() {
    if (mQuestInfo)
        reinterpret_cast<sead::Vector3f*>(mQuestInfo)[1] = al::getTrans(this);
}

// NON_MATCHING: devirtualized tail call vs virtual dispatch
al::RailRider* OpeningStageStartCapManHero::getRailRider() const {
    if (al::isNerve(this, &NrvOpeningStageStartCapManHero.WaitBridge) && mWaitRailKeeper)
        return mWaitRailKeeper->getRailRider();
    return LiveActor::getRailRider();
}

// NON_MATCHING: regalloc differences
void OpeningStageStartCapManHero::startDemo() {
    if (!mIsStartDemo)
        return;

    if (al::isNerve(this, &NrvOpeningStageStartCapManHero.WaitBridge) ||
        al::isNerve(this, &NrvOpeningStageStartCapManHero.RunAway)) {
        appear();
        al::invalidateClipping(this);
        CapManHeroDemoUtil::invalidateDitherAnimIfExist(this);
        GameDataHolderWriter writer(this);
        GameDataFunction::enableCap(writer);
        al::requestCaptureScreenCover(this, 2);
        al::tryOnStageSwitch(this, "SwitchDemoOnOff");
        al::setNerve(this, &NrvOpeningStageStartCapManHero.EventTalkDemo);
    }
}

// NON_MATCHING: regalloc, string compare codegen differences
bool OpeningStageStartCapManHero::receiveEvent(const al::EventFlowEventData* eventData) {
    if (!al::isEventName(eventData, "Event"))
        return false;

    const char* param = al::getEventDataParamString(eventData, "Event");
    if (!param)
        return false;

    if (al::isEqualSubString(param, "StartCamera_")) {
        al::StringTmp<32> cameraName("%s", param);
        al::tryReplaceString(&cameraName, param, "StartCamera_", "");
        if (al::isActiveCamera(mDemoAnimCamera))
            al::endCamera(this, mDemoAnimCamera, 0, false);
        al::startAnimCamera(this, mDemoAnimCamera, cameraName.cstr(), 0);
        const char* camName = cameraName.cstr();
        if (camName) {
            mIsClearSklAnim = al::isEqualString(camName, "DemoMeetCapNpcC01") ||
                              al::isEqualString(camName, "DemoMeetCapNpcC02") ||
                              al::isEqualString(camName, "DemoMeetCapNpcC04") ||
                              al::isEqualString(camName, "DemoMeetCapNpcC05") ||
                              al::isEqualString(camName, "DemoMeetCapNpcC03") ||
                              al::isEqualString(camName, "DemoMeetCapNpcC06") ||
                              al::isEqualString(camName, "DemoMeetCapNpcC07") ||
                              al::isEqualString(camName, "DemoMeetCapNpcC08") ||
                              al::isEqualString(camName, "DemoMeetCapNpcC10A");
        } else {
            mIsClearSklAnim = false;
        }
        return true;
    }

    if (al::isEqualString(param, "EndCamera")) {
        al::endCamera(this, mDemoAnimCamera, -1, false);
        return true;
    }

    if (al::isEqualString(param, "KoopaDemoStart")) {
        al::requestCaptureScreenCover(this, 2);
        al::setNerve(this, &NrvOpeningStageStartCapManHero.EventKoopaDemo);
        return true;
    }

    if (al::isEqualString(param, "KoopaDemoWait"))
        return !al::isNerve(this, &NrvOpeningStageStartCapManHero.EventKoopaDemo);

    if (al::isEqualSubString(param, "StartMarioCapAction")) {
        al::StringTmp<64> actionName("%s", param);
        al::tryReplaceString(&actionName, param, "StartMarioCapAction_", "");
        al::startAction(mMarioCap, actionName.cstr());
        al::showModelIfHide(mMarioCap);
        al::hideSilhouetteModelIfShow(mMarioCap);
        return true;
    }

    if (al::isEqualSubString(param, "StartCapManEyeAction")) {
        al::StringTmp<64> actionName("%s", param);
        al::tryReplaceString(&actionName, param, "StartCapManEyeAction_", "");
        al::startAction(mMarioCapEye, actionName.cstr());
        al::showModelIfHide(mMarioCapEye);
        return true;
    }

    if (al::isEqualSubString(param, "StartSubActorAction")) {
        al::StringTmp<64> actionName("%s", param);
        al::tryReplaceString(&actionName, param, "StartSubActorAction_", "");
        mCapBroken->mActionName.format("%s", actionName.cstr());
        mCapBroken->mIsActionPending = true;

        for (s32 i = 0; i < al::getSubActorNum(this); i++) {
            al::LiveActor* subActor = al::getSubActor(this, i);
            if (subActor != mCapBroken) {
                if (al::tryStartAction(subActor, actionName.cstr()))
                    al::showModelIfHide(subActor);
            }
        }
        return true;
    }

    if (!al::isEqualSubString(param, "ScreenCover"))
        return true;

    if (mIsScreenCoverActive) {
        mIsScreenCoverActive = false;
        return true;
    }

    s32 coverFrames = al::isEqualString(param, "ScreenCover3") ? 3 : 2;
    al::requestCaptureScreenCover(this, coverFrames);
    mIsScreenCoverActive = true;
    return false;
}

bool OpeningStageStartCapManHero::isFirstDemo() const {
    return true;
}

bool OpeningStageStartCapManHero::isEnableSkipDemo() const {
    return true;
}

void OpeningStageStartCapManHero::skipDemo() {
    if (al::isNerve(this, &NrvOpeningStageStartCapManHero.EventTalkDemo)) {
        rs::startEventFlow(mEventFlowExecutor, "MeetFirstSkip");
        rs::updateEventFlow(mEventFlowExecutor);
        al::endCamera(this, mDemoAnimCamera, -1, false);
    } else if (al::isNerve(this, &NrvOpeningStageStartCapManHero.EventKoopaDemo)) {
        mDemoActorHolder->kill();
    }
    endDemo(true);
}

void OpeningStageStartCapManHero::endDemo(bool isSkip) {
    auto* restart = static_cast<TransQuat*>(mPlayerRestartPos);
    rs::replaceDemoPlayer(this, restart->trans, restart->quat);
    rs::startActionDemoPlayer(this, "DemoOpeningCap");
    rs::forcePutOnDemoCap(this);
    rs::showDemoPlayer(this);
    if (isSkip)
        rs::endEventCutSceneDemoBySkip(this);
    else
        rs::endEventCutSceneDemo(this);
    al::tryOnStageSwitch(this, "SwitchDemoEndOn");
    if (mEntranceCamera)
        al::startCamera(this, mEntranceCamera, -1);
    al::validateEndEntranceCamera(this);
    al::killForceBeforeDemo(mMarioCap);
    al::killForceBeforeDemo(mMarioCapEye);
    al::killForceBeforeDemo(mCapBroken);
    if (mQuestInfo)
        rs::invalidateQuest(mQuestInfo);
    al::tryOffStageSwitch(this, "SwitchDemoOnOff");
    al::LiveActor::kill();
}

void OpeningStageStartCapManHero::updateOnlyDemoGraphics() {
    if (al::isNerve(this, &NrvOpeningStageStartCapManHero.EventKoopaDemo) && mDemoActorHolder) {
        // byte at offset 76 in DemoActorHolder
        if (*reinterpret_cast<u8*>(reinterpret_cast<uintptr_t>(mDemoActorHolder) + 76))
            mDemoActorHolder->updateGraphics();
    }
}

void OpeningStageStartCapManHero::restartAfterOpening() {
    al::setSyncRailToCoord(this, 5450.0f);
    al::setNerve(this, &NrvOpeningStageStartCapManHero.RunAway);
}

bool OpeningStageStartCapManHero::isStartRunAway() const {
    if (!_119)
        return false;
    if (al::isNerve(this, &NrvOpeningStageStartCapManHero.NoticePlayerStand))
        return false;
    if (al::isNerve(this, &NrvOpeningStageStartCapManHero.WaitPlayerTurn))
        return false;
    return !al::isNerve(this, &NrvOpeningStageStartCapManHero.Surprise);
}

void OpeningStageStartCapManHero::noticePlayerStand() {
    _119 = true;
    al::invalidateClipping(this);
    al::setNerve(this, &NrvOpeningStageStartCapManHero.NoticePlayerStand);
}

void OpeningStageStartCapManHero::startRunAway() {
    al::setNerve(this, &NrvOpeningStageStartCapManHero.Surprise);
}

void OpeningStageStartCapManHero::startWait() {
    mWaitTimer = 240;
    al::setRailPosToEnd(this);
    al::setTrans(this, al::getRailPos(this));
    al::setNerve(this, &NrvOpeningStageStartCapManHero.WaitBridge);
    al::setVelocityZero(this);
    al::validateClipping(this);
    rs::startEventFlow(mEventFlowExecutor, "WaitDoubt");
    al::setRailPosToStart(this);
    al::setTrans(this, al::getRailPos(this));
}

// NON_MATCHING: constant loading, regalloc in vector math
void OpeningStageStartCapManHero::exeWander() {
    if (al::isFirstStep(this))
        al::tryStartActionIfNotPlaying(this, "Doubt");

    sead::Vector3f dir = {0.0f, 0.0f, 1.0f};
    f32 angle = al::isNerve(this, &NrvOpeningStageStartCapManHero.WanderReverse) ? 110.0f : 245.0f;
    al::rotateVectorDegreeY(&dir, angle);

    const sead::Vector3f& trans = al::getTrans(this);
    const sead::Vector3f& playerPos = rs::getPlayerPos(this);
    sead::Vector3f toPlayer = {trans.x - playerPos.x, 0.0f, trans.z - playerPos.z};
    al::normalize(&toPlayer);

    bool turnDone = al::turnVecToVecDegree(&toPlayer, toPlayer, dir, 1.0f);

    const sead::Vector3f& playerPos2 = rs::getPlayerPos(this);
    sead::Vector3f newPos;
    newPos.x = toPlayer.x * 200.0f + playerPos2.x;
    newPos.y = toPlayer.y * 200.0f + playerPos2.y;
    newPos.z = toPlayer.z * 200.0f + playerPos2.z;
    al::setTrans(this, newPos);
    al::faceToTarget(this, rs::getPlayerPos(this));
    al::resetPosition(this);

    if (turnDone) {
        if (al::isNerve(this, &NrvOpeningStageStartCapManHero.Wander))
            al::setNerve(this, &NrvOpeningStageStartCapManHero.WanderDoubtTouch);
        else
            al::setNerve(this, &NrvOpeningStageStartCapManHero.WanderDoubtTouchReverse);
    }
}

void OpeningStageStartCapManHero::exeWanderDoubtTouch() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "DoubtTouch");
        al::setVelocityZero(this);
    }
    if (al::isActionEnd(this)) {
        if (al::isNerve(this, &NrvOpeningStageStartCapManHero.WanderDoubtTouchReverse))
            al::setNerve(this, &NrvOpeningStageStartCapManHero.Wander);
        else if (al::isNerve(this, &NrvOpeningStageStartCapManHero.WanderDoubtTouch))
            al::setNerve(this, &NrvOpeningStageStartCapManHero.WanderReverse);
    }
}

void OpeningStageStartCapManHero::exeNoticePlayerStand() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Panic");
        al::startHitReaction(this, "帽子の国マリオ立ち上がり驚き");
        mRunSpeed = 0.0f;
    }
    if (runAwayToRail())
        al::setNerve(this, &NrvOpeningStageStartCapManHero.WaitPlayerTurn);
}

// NON_MATCHING: regalloc in vector multiply
bool OpeningStageStartCapManHero::runAwayToRail() {
    sead::Vector3f railPos;
    al::calcRailPosAtCoord(&railPos, this, 0.0f);

    mRunSpeed = (mRunSpeed + 15.0f) * 0.7f;

    al::turnToTarget(this, railPos, 20.0f);

    const sead::Vector3f& trans = al::getTrans(this);
    sead::Vector3f diff;
    diff.x = railPos.x - trans.x;
    diff.y = railPos.y - trans.y;
    diff.z = railPos.z - trans.z;

    f32 dist = sead::Mathf::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
    if (dist < mRunSpeed) {
        al::setVelocityZero(this);
        return true;
    }

    sead::Vector3f normalized;
    al::normalize(&normalized, diff);
    sead::Vector3f vel;
    vel.x = normalized.x * mRunSpeed;
    vel.y = normalized.y * mRunSpeed;
    vel.z = normalized.z * mRunSpeed;
    al::setVelocity(this, vel);
    return false;
}

void OpeningStageStartCapManHero::exeWaitPlayerTurn() {
    if (al::isFirstStep(this)) {
        rs::startEventFlow(mEventFlowExecutor, "WaitPlayerTurn");
        al::startAction(this, "OpeningStandUpWait");
    }
    rs::updateEventFlow(mEventFlowExecutor);
}

void OpeningStageStartCapManHero::exeSurprise() {
    if (al::isFirstStep(this))
        al::startAction(this, "OpeningReaction");
    if (al::isActionEnd(this))
        al::setNerve(this, &NrvOpeningStageStartCapManHero.RunAway);
}

// NON_MATCHING: DeMorgan boolean inversion, null-check pattern
void OpeningStageStartCapManHero::exeRunAway() {
    if (al::isFirstStep(this)) {
        al::tryStartActionIfNotPlaying(this, "PanicRun");
        al::setSyncRailToNearestPos(this, al::getTrans(this));
    }

    al::turnToRailDir(this, 20.0f);

    bool isDistantFromPlayer = false;
    if (al::isRailReachedNearRailPoint(this, 400.0f)) {
        f32 playerCoord = al::calcNearestRailCoord(this, rs::getPlayerPos(this));
        f32 railCoord = al::getRailCoord(this);
        isDistantFromPlayer = (railCoord - playerCoord) > 1000.0f;
    }

    f32 railCoord = al::getRailCoord(this);
    bool inWaitZone = (railCoord > 5000.0f && railCoord < 5500.0f) ||
                      (railCoord > 15350.0f && railCoord < 15800.0f);

    if (isDistantFromPlayer && inWaitZone) {
        f32 prevCoord = mPrevRailCoord;
        if ((railCoord > 5400.0f && railCoord < 5500.0f &&
             (prevCoord <= 5400.0f || prevCoord >= 5500.0f)) ||
            (railCoord > 15700.0f && railCoord < 15800.0f &&
             (prevCoord >= 15800.0f || prevCoord <= 15700.0f))) {
            al::setNerve(this, &NrvOpeningStageStartCapManHero.RunAwayWait);
            return;
        }
        mRunSpeed = (mRunSpeed + 1.0f) * 0.85f;
    } else {
        f32 accel = al::isNearPlayer(this, 1000.0f) ? 18.0f : 15.0f;
        mRunSpeed = (mRunSpeed + accel) * 0.7f;
    }

    al::tryAddRippleSmall(this);

    if (al::moveSyncRail(this, mRunSpeed)) {
        mWaitTimer = 240;
        al::setRailPosToEnd(this);
        al::setTrans(this, al::getRailPos(this));
        al::setNerve(this, &NrvOpeningStageStartCapManHero.WaitBridge);
        al::setVelocityZero(this);
        al::validateClipping(this);
        rs::startEventFlow(mEventFlowExecutor, "WaitDoubt");
        al::setRailPosToStart(this);
        al::setTrans(this, al::getRailPos(this));
    }
}

// NON_MATCHING: null-check csel, fcmp comparison flag (b.gt vs b.hi)
void OpeningStageStartCapManHero::exeRunAwayWait() {
    if (al::isFirstStep(this)) {
        rs::startEventFlow(mEventFlowExecutor, "RunAwayWait");
        al::setVelocityZero(this);
        mRunSpeed = 0.0f;
        mPrevRailCoord = al::getRailCoord(this);
    }

    rs::updateEventFlow(mEventFlowExecutor);

    sead::Vector3f dir;
    al::calcDirToActorH(&dir, this, al::getPlayerActor(this, 0));
    al::turnToDirection(this, dir, 15.0f);

    f32 playerCoord = al::calcNearestRailCoord(this, rs::getPlayerPos(this));
    f32 railCoord = al::getRailCoord(this);

    if ((railCoord - playerCoord) <= 1000.0f)
        al::setNerve(this, &NrvOpeningStageStartCapManHero.RunAway);
}

// NON_MATCHING: regalloc, imm/offset differences
void OpeningStageStartCapManHero::exeWaitBridge() {
    if (al::isFirstStep(this)) {
        mWaitTimer = 240;
        f32 startCoord = al::calcRailCoordByPoint(this, al::getRailPointNo(this));
        al::calcRailPosAtCoord(&mBridgeStartPos, this, startCoord);
        f32 endCoord = al::calcRailCoordByPoint(this, al::getNextRailPointNo(this));
        al::calcRailPosAtCoord(&mBridgeEndPos, this, endCoord);
        al::startAction(this, "Warning");
    }

    rs::updateEventFlow(mEventFlowExecutor);

    if (mWaitTimer >= 1) {
        sead::Vector3f dir;
        al::calcDirToActorH(&dir, this, al::getPlayerActor(this, 0));
        al::turnToDirection(this, dir, 8.0f);
        mWaitTimer--;
        return;
    }

    if (!al::isActionPlaying(this, "MoveCapWorldBridge"))
        al::tryStartActionIfNotPlaying(this, "Turn");

    sead::Vector3f railDir;
    al::calcRailMoveDir(&railDir, this);
    if (!al::turnToDirection(this, railDir, 20.0f))
        return;

    al::tryStartActionIfNotPlaying(this, "MoveCapWorldBridge");

    s32 step = al::getNerveStep(this);
    f32 rate = al::easeInOut((f32)(step - 240) / 60.0f);

    const sead::Vector3f& trans = al::getTrans(this);
    f32 x = trans.x;
    f32 y = trans.y;
    f32 z = trans.z;

    sead::Vector3f lerpPos;
    al::lerpVec(&lerpPos, mBridgeStartPos, mBridgeEndPos, rate);

    f32 dist =
        sead::Mathf::sqrt((lerpPos.x - x) * (lerpPos.x - x) + (lerpPos.y - y) * (lerpPos.y - y) +
                          (lerpPos.z - z) * (lerpPos.z - z));

    if (al::isGreaterEqualStep(this, 290)) {
        sead::Vector3f toPlayer;
        al::calcDirToActorH(&toPlayer, this, al::getPlayerActor(this, 0));
        al::turnToDirection(this, toPlayer, 8.0f);
    }

    if (al::moveSyncRailTurn(this, dist))
        al::setNerve(this, &NrvOpeningStageStartCapManHero.WaitBridge);
}

// NON_MATCHING: regalloc, imm/offset differences in large function
void OpeningStageStartCapManHero::exeEventTalkDemo() {
    if (al::isFirstStep(this) && !rs::isActiveDemo(this)) {
        if (!rs::tryStartEventCutSceneDemo(this)) {
            al::setNerve(this, &NrvOpeningStageStartCapManHero.EventTalkDemo);
            return;
        }
        rs::requestValidateDemoSkip(this, this);

        auto* talkStart = static_cast<TransQuat*>(mTalkStartPos);
        TransQuat localTalkStart = *talkStart;
        rs::replaceDemoPlayer(this, localTalkStart.trans, localTalkStart.quat);

        sead::Vector3f front;
        al::calcQuatFront(&front, localTalkStart.quat);
        al::setFront(this, front);
        al::setTrans(this, localTalkStart.trans);
        al::resetPosition(this);

        rs::startEventFlow(mEventFlowExecutor, "MeetFirst");

        if (al::isDead(mMarioCap))
            mMarioCap->appear();
        al::hideModelIfShow(mMarioCap);
        al::setQuat(mMarioCap, localTalkStart.quat);
        al::setTrans(mMarioCap, localTalkStart.trans);
        al::resetPosition(mMarioCap);
        rs::addDemoActor(mMarioCap, false);

        if (al::isDead(mMarioCapEye))
            mMarioCapEye->appear();
        al::hideModelIfShow(mMarioCapEye);
        al::setQuat(mMarioCapEye, localTalkStart.quat);
        al::setTrans(mMarioCapEye, localTalkStart.trans);
        al::resetPosition(mMarioCapEye);
        rs::addDemoActor(mMarioCapEye, false);

        mCapBroken->mIsActionPending = false;

        if (al::isDead(mCapBroken))
            mCapBroken->appear();
        al::hideModelIfShow(mCapBroken);
        al::setQuat(mCapBroken, localTalkStart.quat);
        al::setTrans(mCapBroken, localTalkStart.trans);
        al::resetPosition(mCapBroken);

        mJointSpringControllerHolder->offControlAll();
        al::addDemoActorFromAddDemoInfo(this, mAddDemoInfo);
    }

    if (rs::isCloseNpcDemoEventTalkMessage(this))
        mDemoTalkState->kill();

    rs::resetMarioDynamics(this);

    if (rs::updateEventFlow(mEventFlowExecutor)) {
        al::endCamera(this, mDemoAnimCamera, -1, false);
        endDemo(false);
        return;
    }

    if (al::isActionPlaying(this, "DemoMeetCapNpcC10A")) {
        if (al::getActionFrame(this) == 161.0f)
            al::requestCaptureScreenCover(this, 2);
    }

    if (!mDemoTalkState->isDead()) {
        mDemoTalkState->update();

        const char* myAction = al::getActionName(this);
        const char* brokenAction = al::getActionName(mCapBroken);
        if (brokenAction && al::isEqualString(myAction, brokenAction)) {
            f32 weight0 = al::getSklAnimBlendWeight(this, 0);
            f32 weight1 = al::getSklAnimBlendWeight(this, 1);
            mCapBroken->mIsBlendWeightDirty = true;
            mCapBroken->mBlendWeight0 = weight0;
            mCapBroken->mBlendWeight1 = weight1;
        }

        if (rs::isCloseNpcDemoEventTalkMessage(this))
            mDemoTalkState->kill();
    } else {
        const char* actionName = nullptr;
        if (al::isExistEventDemoTalkAction(mEventFlowExecutor)) {
            actionName = al::getEventDemoTalkActionName(mEventFlowExecutor);
        } else {
            const char* curAction = al::getActionName(this);
            if (al::isEqualSubString(curAction, "Blend") && al::getActionFrame(this) == 0.0f)
                actionName = al::getActionName(this);
        }
        if (actionName) {
            mDemoTalkState->mActionName = actionName;
            al::startAction(this, actionName);
            mDemoTalkState->appear();
            mCapBroken->mActionName.format("%s", actionName);
            mCapBroken->mIsActionPending = true;
        }
    }

    if (mIsClearSklAnim) {
        al::clearSklAnimInterpole(this);
        mCapBroken->mIsClearInterpole = true;
        mIsClearSklAnim = false;
    }
}

void OpeningStageStartCapManHero::exeEventKoopaDemo() {
    if (al::isFirstStep(this)) {
        mDemoActorHolder->startSequence();
        rs::setDemoEnvironmentChangeFlag(this, true);
    }
    if (al::isStep(this, 1))
        al::addDemoActorFromDemoActorHolder(this, mDemoActorHolder);
    mDemoActorHolder->updateSequence();

    s32 frame = mDemoActorHolder->getCurrentDemoFrame();
    if (frame == mDemoActorHolder->getCurrentDemoFrameMax() - 1)
        al::requestCaptureScreenCover(this, 3);

    if (mDemoActorHolder->isEndSequence()) {
        rs::setDemoEnvironmentChangeFlag(this, false);
        mDemoActorHolder->kill();
        al::setCameraInterpoleStep(mDemoAnimCamera, 0);
        al::setNerve(this, &NrvOpeningStageStartCapManHero.EventTalkDemo);
    }
}
