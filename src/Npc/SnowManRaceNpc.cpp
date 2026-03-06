#include "Npc/SnowManRaceNpc.h"

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Base/StringUtil.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"

#include "Npc/NpcStateReaction.h"
#include "Npc/TalkNpcActionAnimInfo.h"
#include "Npc/TalkNpcParam.h"
#include "Sequence/ChangeStageInfo.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/ItemUtil.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerDemoUtil.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

namespace rs {
void registerLinkedPlayerStartInfoToHolder(const al::IUseSceneObjHolder*,
                                           const al::ActorInitInfo&, const char*,
                                           const sead::Vector3f*, const sead::Quatf*);
}  // namespace rs

namespace {
NERVE_IMPL(SnowManRaceNpc, RaceResultWin);
NERVE_IMPL(SnowManRaceNpc, RaceResultFirst);
NERVE_IMPL(SnowManRaceNpc, RaceResultLose);
NERVE_IMPL(SnowManRaceNpc, RaceResultCancel);
NERVE_IMPL(SnowManRaceNpc, RaceResultSecond);
NERVE_IMPL(SnowManRaceNpc, RaceResultThird);
NERVE_IMPL(SnowManRaceNpc, WaitNoPlayerHack);
NERVE_IMPL(SnowManRaceNpc, Reaction);
NERVE_IMPL_(SnowManRaceNpc, ReactionWinWait, Reaction);
NERVE_IMPL_(SnowManRaceNpc, GetPrizeSecond, GetPrize);
NERVE_IMPL_(SnowManRaceNpc, GetPrizeThird, GetPrize);
NERVE_IMPL_(SnowManRaceNpc, GetPrizeFirst, GetPrize);
NERVE_IMPL_(SnowManRaceNpc, GetPrizeEndFirst, GetPrizeEnd);
NERVE_IMPL_(SnowManRaceNpc, GetPrizeEndSecond, GetPrizeEnd);
NERVE_IMPL_(SnowManRaceNpc, GetPrizeEndThird, GetPrizeEnd);
NERVE_IMPL(SnowManRaceNpc, WaitPlayerHack);
NERVE_IMPL(SnowManRaceNpc, RaceResultWinWait);

NERVES_MAKE_STRUCT(SnowManRaceNpc, RaceResultWin, RaceResultFirst, RaceResultLose,
                   RaceResultCancel, RaceResultSecond, RaceResultThird, WaitNoPlayerHack,
                   Reaction, ReactionWinWait, GetPrizeSecond, GetPrizeThird, GetPrizeFirst,
                   GetPrizeEndFirst, GetPrizeEndSecond, GetPrizeEndThird, WaitPlayerHack,
                   RaceResultWinWait);
}  // namespace

SnowManRaceNpc::SnowManRaceNpc(const char* name) : al::LiveActor(name) {}

// NON_MATCHING: stack frame size, register allocation, switch codegen differences
void SnowManRaceNpc::init(const al::ActorInitInfo& info) {
    mActionAnimInfo = new TalkNpcActionAnimInfo();
    mActionAnimInfo->mWaitActionName = "Wait";

    al::initActorWithArchiveName(this, info, "SnowManLeader", nullptr);

    const char* placeScenarioStr = nullptr;
    al::tryGetStringArg(&placeScenarioStr, info, "PlaceScenario");
    if (al::isEqualString(placeScenarioStr, "Scenario1"))
        mPlaceScenario = 0;
    else if (al::isEqualString(placeScenarioStr, "Scenario2"))
        mPlaceScenario = 1;
    else if (al::isEqualString(placeScenarioStr, "Scenario2MoonRock"))
        mPlaceScenario = 2;
    else
        mPlaceScenario = 0;

    const char* stageName = nullptr;
    al::getStringArg(&stageName, info, "ChangeStageName");

    if (GameDataFunction::isRaceLose(this) || GameDataFunction::isRaceResultSecond(this) ||
        GameDataFunction::isRaceResultThird(this)) {
        if (mPlaceScenario == 0)
            al::getStringArg(&stageName, info, "ChangeStageNameSecond");
    } else if (GameDataFunction::isRaceCancel(this) && mPlaceScenario == 0) {
        al::getStringArg(&stageName, info, "ChangeStageNameSecond");
    }

    const al::PlacementInfo& placementInfo = al::getPlacementInfo(info);
    mChangeStageInfo = rs::createChangeStageInfo(this, placementInfo, nullptr, stageName, false, 1,
                                                 ChangeStageInfo::SNOW_MAN_RACE);

    if (mPlaceScenario == 2) {
        const char* stageNameSecond = nullptr;
        al::getStringArg(&stageNameSecond, info, "ChangeStageNameSecond");
        mChangeStageInfoSecond =
            rs::createChangeStageInfo(this, placementInfo, nullptr, stageNameSecond, false, 1,
                                      ChangeStageInfo::SNOW_MAN_RACE);
    }

    rs::registerLinkedPlayerStartInfoToHolder(this, info, nullptr, nullptr, nullptr);
    mDemoCamera = al::initDemoObjectCamera(this, info, nullptr, u8"会話カメラ");
    mTalkNpcParam = rs::initTalkNpcParam(this, nullptr);
    mTalkNpcParam->tryInitPartialSklAnim(this);

    if (GameDataFunction::isRaceWin(this))
        mRaceResult = 0;
    else if (GameDataFunction::isRaceLose(this))
        mRaceResult = 3;
    else if (GameDataFunction::isRaceCancel(this))
        mRaceResult = 4;
    else if (GameDataFunction::isRaceResultSecond(this))
        mRaceResult = 1;
    else if (GameDataFunction::isRaceResultThird(this))
        mRaceResult = 2;
    else
        mRaceResult = 5;

    switch (mRaceResult) {
    case 0:
        if (mPlaceScenario != 0)
            al::initNerve(this, &NrvSnowManRaceNpc.RaceResultFirst, 2);
        else
            al::initNerve(this, &NrvSnowManRaceNpc.RaceResultWin, 2);
        mYukimaruActor = al::tryCreateLinksActorFromFactorySingle(info, "RaceYukimaru");
        if (mYukimaruActor)
            mYukimaruActor->makeActorDead();
        break;
    case 1:
        al::initNerve(this, &NrvSnowManRaceNpc.RaceResultSecond, 2);
        goto createBothYukimaru;
    case 2:
        al::initNerve(this, &NrvSnowManRaceNpc.RaceResultThird, 2);
        goto createBothYukimaru;
    case 3:
        al::initNerve(this, &NrvSnowManRaceNpc.RaceResultLose, 2);
    createBothYukimaru:
        mYukimaruActor = al::tryCreateLinksActorFromFactorySingle(info, "RaceYukimaru");
        if (mYukimaruActor)
            mYukimaruActor->makeActorDead();
        {
            al::LiveActor* normalYukimaru =
                al::tryCreateLinksActorFromFactorySingle(info, "NormalYukimaru");
            if (normalYukimaru)
                normalYukimaru->makeActorDead();
        }
        break;
    case 4:
        al::initNerve(this, &NrvSnowManRaceNpc.RaceResultCancel, 2);
        mYukimaruActor = al::tryCreateLinksActorFromFactorySingle(info, "RaceYukimaru");
        if (mYukimaruActor)
            mYukimaruActor->makeActorDead();
        mIsHackStarted = false;
        {
            al::LiveActor* normalYukimaru =
                al::tryCreateLinksActorFromFactorySingle(info, "NormalYukimaru");
            if (normalYukimaru)
                normalYukimaru->makeActorDead();
        }
        mEntranceCamera = al::initEntranceCamera(this, info, u8"キャンセル");
        break;
    default:
        al::initNerve(this, &NrvSnowManRaceNpc.WaitNoPlayerHack, 2);
        mYukimaruActor = al::tryCreateLinksActorFromFactorySingle(info, "NormalYukimaru");
        if (mYukimaruActor)
            mYukimaruActor->makeActorDead();
        break;
    }

    const char* eventFlowName;
    const char* eventFlowSecondName;

    if (mPlaceScenario == 2) {
        eventFlowName = "SnowManRaceNpc2";
        eventFlowSecondName = "SnowManRaceNpc2_MoonRock";
    } else if (mPlaceScenario == 1) {
        eventFlowName = "SnowManRaceNpc2";
        eventFlowSecondName = "SnowManRaceNpc2";
    } else if (mPlaceScenario != 0) {
        eventFlowName = "SnowManRaceNpc2";
        eventFlowSecondName = nullptr;
    } else {
        eventFlowName = "SnowManRaceNpc";
        if (mRaceResult == 0)
            eventFlowSecondName = "SnowManRaceNpcWin";
        else
            eventFlowSecondName = "SnowManRaceNpc";
    }

    mEventFlowExecutor = rs::initEventFlow(this, info, eventFlowName, eventFlowSecondName);
    rs::initEventCharacterName(mEventFlowExecutor, info, "Yukimaru");
    al::initEventReceiver(mEventFlowExecutor, static_cast<al::IEventFlowEventReceiver*>(this));
    rs::initEventQueryJudge(mEventFlowExecutor,
                            static_cast<const al::IEventFlowQueryJudge*>(this));
    mReactionState = NpcStateReaction::create(this, nullptr);
    al::initNerveState(this, mReactionState, &NrvSnowManRaceNpc.Reaction,
                       u8"リアクション");
    al::addNerveState(this, mReactionState, &NrvSnowManRaceNpc.ReactionWinWait,
                      u8"リアクション[勝った後]");
    makeActorAlive();
}

// NON_MATCHING: IUseCamera base class offset, vtable index for initAfterPlacement virtual call
void SnowManRaceNpc::initAfterPlacement() {
    if (mYukimaruActor)
        mYukimaruActor->initAfterPlacement();

    if ((u32)mRaceResult < 4) {
        rs::tryStartEventCutSceneKeepHackDemo(this);
        setHackKeepTalkDemo();
    } else if (mRaceResult == 4) {
        al::startCamera(this, mEntranceCamera, -1);
        al::requestCancelCameraInterpole(this, 0);
        al::invalidateEndEntranceCamera(this);
    }
}

// NON_MATCHING: IUseCamera base class offset, Quatf::unit copy width
void SnowManRaceNpc::setHackKeepTalkDemo() {
    rs::startActionDemoPlayer(this, "WaitTalk");

    sead::Quatf quat = sead::Quatf::unit;
    al::calcQuat(&quat, this);

    sead::Vector3f offset(0.0f, 0.0f, 800.0f);
    offset.rotate(quat);
    sead::Vector3f pos = al::getTrans(this) + offset;

    rs::replaceDemoPlayer(this, pos, quat);
    rs::clearDemoAnimInterpolatePlayer(this);

    al::LiveActor* yukimaru = mYukimaruActor;
    sead::Quatf yukimaruQuat = sead::Quatf::unit;
    const sead::Vector3f& trans = al::getTrans(this);
    const sead::Vector3f& demoTrans = rs::getDemoPlayerTrans(this);
    sead::Vector3f diff = trans - demoTrans;

    f32 angle =
        al::calcAngleOnPlaneDegree(sead::Vector3f::ez, diff, sead::Vector3f::ey);
    al::rotateQuatYDirDegree(&yukimaruQuat, yukimaruQuat, angle);

    *(sead::Quatf*)((char*)yukimaru + 328) = yukimaruQuat;

    al::CameraTicket* camera;
    if (mRaceResult == 4)
        camera = mEntranceCamera;
    else
        camera = mDemoCamera;
    al::startCamera(this, camera, -1);
    al::requestCancelCameraInterpole(this, 0);
}

void SnowManRaceNpc::control() {
    if (mTimer > 0)
        mTimer--;
}

bool SnowManRaceNpc::receiveEvent(const al::EventFlowEventData* event) {
    if (al::isEventName(event, "RaceStartLv1")) {
        GameDataFunction::setRaceRivalLevel(this, 0);
        GameDataFunction::tryChangeNextStageWithStartRaceYukimaru(this, mChangeStageInfo);
        return true;
    }

    if (al::isEventName(event, "RaceStartLv2")) {
        GameDataFunction::setRaceRivalLevel(this, 1);
        if (mPlaceScenario == 2)
            GameDataFunction::tryChangeNextStageWithStartRaceYukimaru(this,
                                                                      mChangeStageInfoSecond);
        else
            GameDataFunction::tryChangeNextStageWithStartRaceYukimaru(this, mChangeStageInfo);
        return true;
    }

    if (al::isEventName(event, "YukimaruTalkDemoStart")) {
        setHackKeepTalkDemo();
        return true;
    }

    if (al::isEventName(event, "YukimaruTalkDemoEnd")) {
        if (al::isActiveCamera(mDemoCamera))
            al::endCamera(this, mDemoCamera, -1, false);
        if (!al::isNerve(this, &NrvSnowManRaceNpc.RaceResultWin) &&
            !al::isNerve(this, &NrvSnowManRaceNpc.RaceResultFirst))
            al::startAction(this, "Wait");
        return true;
    }

    if (al::isEventName(event, "YukimaruGetPrize")) {
        if (al::isNerve(this, &NrvSnowManRaceNpc.GetPrizeSecond) ||
            al::isNerve(this, &NrvSnowManRaceNpc.GetPrizeThird) ||
            al::isNerve(this, &NrvSnowManRaceNpc.GetPrizeEndSecond) ||
            al::isNerve(this, &NrvSnowManRaceNpc.GetPrizeEndThird) ||
            al::isNerve(this, &NrvSnowManRaceNpc.GetPrizeFirst) ||
            al::isNerve(this, &NrvSnowManRaceNpc.GetPrizeEndFirst)) {
            if (al::isNerve(this, &NrvSnowManRaceNpc.GetPrizeEndFirst))
                al::setNerve(this, &NrvSnowManRaceNpc.RaceResultFirst);
            else if (al::isNerve(this, &NrvSnowManRaceNpc.GetPrizeEndSecond))
                al::setNerve(this, &NrvSnowManRaceNpc.RaceResultSecond);
            else if (al::isNerve(this, &NrvSnowManRaceNpc.GetPrizeEndThird))
                al::setNerve(this, &NrvSnowManRaceNpc.RaceResultThird);
            else
                return false;
            return true;
        }

        if (al::isNerve(this, &NrvSnowManRaceNpc.RaceResultSecond))
            al::setNerve(this, &NrvSnowManRaceNpc.GetPrizeSecond);
        else if (al::isNerve(this, &NrvSnowManRaceNpc.RaceResultThird))
            al::setNerve(this, &NrvSnowManRaceNpc.GetPrizeThird);
        else if (al::isNerve(this, &NrvSnowManRaceNpc.RaceResultFirst))
            al::setNerve(this, &NrvSnowManRaceNpc.GetPrizeFirst);
        else
            return false;
        return false;
    }

    if (al::isEventName(event, "SetLv1ClearFlag")) {
        mIsLv1ClearFlag = true;
        return true;
    }

    return false;
}

const char* SnowManRaceNpc::judgeQuery(const char* query) const {
    if (!al::isEqualString(query, "JudgePlayLv1"))
        return nullptr;
    return mIsLv1ClearFlag ? "Played" : "NotPlayed";
}

void SnowManRaceNpc::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    rs::attackSensorNpcCommon(self, other);
}

// NON_MATCHING: Vector3f constant load pattern (GOT pointer vs inline)
bool SnowManRaceNpc::receiveMsg(const al::SensorMsg* msg, al::HitSensor* self, al::HitSensor* other) {
    if (rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;

    if (rs::isMsgPlayerLookAtPosition(msg)) {
        const sead::Vector3f lookAtOffset(0.0f, 300.0f, 0.0f);
        sead::Vector3f pos = al::getTrans(this) + lookAtOffset;
        rs::setMsgPlayerLookAtPosition(msg, pos);
        return true;
    }

    if (!mReactionState->receiveMsg(msg, self, other))
        return false;

    if (!al::isNerve(this, &NrvSnowManRaceNpc.Reaction) &&
        !al::isNerve(this, &NrvSnowManRaceNpc.ReactionWinWait)) {
        if (al::isNerve(this, &NrvSnowManRaceNpc.WaitNoPlayerHack) ||
            al::isNerve(this, &NrvSnowManRaceNpc.WaitPlayerHack))
            al::setNerve(this, &NrvSnowManRaceNpc.Reaction);
        else if (al::isNerve(this, &NrvSnowManRaceNpc.RaceResultWinWait))
            al::setNerve(this, &NrvSnowManRaceNpc.ReactionWinWait);
    }
    return true;
}

void SnowManRaceNpc::exeWaitNoPlayerHack() {
    if (rs::isPlayerHackYukimaru(this)) {
        al::setNerve(this, &NrvSnowManRaceNpc.WaitPlayerHack);
        return;
    }

    rs::tryUpdateNpcFacialAnim(this, mTalkNpcParam);

    if (al::isFirstStep(this)) {
        if (!al::isActionPlaying(this, "Wait") && !al::isActionPlaying(this, "WaitRandom"))
            al::startAction(this, "Wait");
        rs::startEventFlow(mEventFlowExecutor, "WaitNoPlayerHack");
    }

    updateRandomAction();
    rs::updateEventFlow(mEventFlowExecutor);
}

void SnowManRaceNpc::updateRandomAction() {
    if (al::isActionPlaying(this, "WaitRandom") && al::isActionEnd(this))
        al::startAction(this, "Wait");

    if (rs::isSuccessNpcEventBalloonMessage(this)) {
        if (mTimer == 0 && mIsFarFromPlayer) {
            if (!al::isActionPlaying(this, "WaitRandom")) {
                al::startAction(this, "WaitRandom");
                mTimer = 120;
                mIsFarFromPlayer = false;
            }
        }
    }

    if (!mIsFarFromPlayer)
        mIsFarFromPlayer = !rs::isNearPlayerH(this, 1500.0f);
}

void SnowManRaceNpc::exeWaitPlayerHack() {
    if (al::isFirstStep(this)) {
        if (!al::isActionPlaying(this, "Wait") && !al::isActionPlaying(this, "WaitRandom"))
            al::startAction(this, "Wait");
        if (mIsHackStarted)
            rs::startEventFlow(mEventFlowExecutor, "WaitPlayerHack");
        else
            rs::startEventFlow(mEventFlowExecutor, "SecondAndAfterTalk");
    }

    updateRandomAction();
    bool updated = rs::updateEventFlow(mEventFlowExecutor);
    bool isHackYukimaru = rs::isPlayerHackYukimaru(this);

    if (updated) {
        if (isHackYukimaru)
            al::setNerve(this, &NrvSnowManRaceNpc.WaitPlayerHack);
        else
            al::setNerve(this, &NrvSnowManRaceNpc.WaitNoPlayerHack);
    } else if (!isHackYukimaru) {
        al::setNerve(this, &NrvSnowManRaceNpc.WaitNoPlayerHack);
    }
}

void SnowManRaceNpc::exeReaction() {
    if (al::isFirstStep(this)) {
        mIsFarFromPlayer = true;
        mTimer = 0;
    }

    if (al::updateNerveState(this)) {
        if (al::isNerve(this, &NrvSnowManRaceNpc.Reaction)) {
            if (rs::isPlayerHackYukimaru(this))
                al::setNerve(this, &NrvSnowManRaceNpc.WaitPlayerHack);
            else
                al::setNerve(this, &NrvSnowManRaceNpc.WaitNoPlayerHack);
        } else if (al::isNerve(this, &NrvSnowManRaceNpc.ReactionWinWait)) {
            al::setNerve(this, &NrvSnowManRaceNpc.RaceResultWinWait);
        }
    }
}

void SnowManRaceNpc::exeRaceResultWin() {
    if (al::isFirstStep(this))
        rs::startEventFlow(mEventFlowExecutor, "RaceResultWin");

    if (rs::updateEventFlow(mEventFlowExecutor)) {
        if (mPlaceScenario != 0) {
            mIsHackStarted = false;
            if (rs::isPlayerHackYukimaru(this))
                al::setNerve(this, &NrvSnowManRaceNpc.WaitPlayerHack);
            else
                al::setNerve(this, &NrvSnowManRaceNpc.WaitNoPlayerHack);
        } else {
            al::setNerve(this, &NrvSnowManRaceNpc.RaceResultWinWait);
        }
    }
}

void SnowManRaceNpc::exeRaceResultWinWait() {
    if (al::isFirstStep(this)) {
        rs::startEventFlow(mEventFlowExecutor, "RaceResultWinWait");
        al::startAction(this, "Wait");
    }

    rs::tryUpdateNpcFacialAnim(this, mTalkNpcParam);
    rs::updateEventFlow(mEventFlowExecutor);
}

void SnowManRaceNpc::exeRaceResultFirst() {
    if (al::isFirstStep(this) &&
        !al::isCurrentEventEntry(mEventFlowExecutor, "RaceResultWin")) {
        rs::startEventFlow(mEventFlowExecutor, "RaceResultWin");
        mPrizeCount = 20;
    }

    if (rs::updateEventFlow(mEventFlowExecutor)) {
        mIsHackStarted = false;
        if (rs::isPlayerHackYukimaru(this))
            al::setNerve(this, &NrvSnowManRaceNpc.WaitPlayerHack);
        else
            al::setNerve(this, &NrvSnowManRaceNpc.WaitNoPlayerHack);
    }
}

void SnowManRaceNpc::exeRaceResultSecond() {
    if (al::isFirstStep(this) &&
        !al::isCurrentEventEntry(mEventFlowExecutor, "RaceResultSecond")) {
        al::startAction(this, "Wait");
        rs::startEventFlow(mEventFlowExecutor, "RaceResultSecond");
        mPrizeCount = 4;
    }

    if (rs::updateEventFlow(mEventFlowExecutor)) {
        mIsHackStarted = false;
        if (rs::isPlayerHackYukimaru(this))
            al::setNerve(this, &NrvSnowManRaceNpc.WaitPlayerHack);
        else
            al::setNerve(this, &NrvSnowManRaceNpc.WaitNoPlayerHack);
    }
}

void SnowManRaceNpc::exeRaceResultThird() {
    if (al::isFirstStep(this) &&
        !al::isCurrentEventEntry(mEventFlowExecutor, "RaceResultThird")) {
        al::startAction(this, "Wait");
        rs::startEventFlow(mEventFlowExecutor, "RaceResultThird");
        mPrizeCount = 2;
    }

    if (rs::updateEventFlow(mEventFlowExecutor)) {
        mIsHackStarted = false;
        if (rs::isPlayerHackYukimaru(this))
            al::setNerve(this, &NrvSnowManRaceNpc.WaitPlayerHack);
        else
            al::setNerve(this, &NrvSnowManRaceNpc.WaitNoPlayerHack);
    }
}

void SnowManRaceNpc::exeGetPrize() {
    al::LiveActor* yukimaru = mYukimaruActor;
    const sead::Vector3f& trans = al::getTrans(yukimaru);
    s32 step = al::getNerveStep(this);

    if (rs::tryAppearMultiCoinFromObj(yukimaru, trans, step, 300.0f)) {
        mPrizeCount--;
        if (mPrizeCount <= 0) {
            if (al::isNerve(this, &NrvSnowManRaceNpc.GetPrizeFirst))
                al::setNerve(this, &NrvSnowManRaceNpc.GetPrizeEndFirst);
            else if (al::isNerve(this, &NrvSnowManRaceNpc.GetPrizeSecond))
                al::setNerve(this, &NrvSnowManRaceNpc.GetPrizeEndSecond);
            else if (al::isNerve(this, &NrvSnowManRaceNpc.GetPrizeThird))
                al::setNerve(this, &NrvSnowManRaceNpc.GetPrizeEndThird);
        }
    }
}

void SnowManRaceNpc::exeGetPrizeEnd() {
    if (al::isFirstStep(this))
        mIsHackStarted = false;
    rs::updateEventFlow(mEventFlowExecutor);
}

void SnowManRaceNpc::exeRaceResultLose() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Wait");
        rs::startEventFlow(mEventFlowExecutor, "RaceResultLose");
    }

    if (rs::updateEventFlow(mEventFlowExecutor)) {
        mIsHackStarted = false;
        if (rs::isPlayerHackYukimaru(this))
            al::setNerve(this, &NrvSnowManRaceNpc.WaitPlayerHack);
        else
            al::setNerve(this, &NrvSnowManRaceNpc.WaitNoPlayerHack);
    }
}

void SnowManRaceNpc::exeRaceResultCancel() {
    if (al::isFirstStep(this))
        al::startAction(this, "Wait");

    if (al::isGreaterEqualStep(this, 60)) {
        al::validateEndEntranceCamera(this);
        al::setNerve(this, &NrvSnowManRaceNpc.WaitNoPlayerHack);
    }
}
