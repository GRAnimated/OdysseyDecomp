#include "Npc/RaceManStart.h"

#include <math/seadVector.h>

#include "Library/Base/StringUtil.h"
#include "Library/Demo/DemoFunction.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementId.h"
#include "Library/Placement/PlacementInfo.h"
#include "Library/Stage/StageSwitchUtil.h"

#include "Npc/NpcStateReaction.h"
#include "Npc/TalkNpcCap.h"
#include "Sequence/ChangeStageInfo.h"
#include "System/GameDataFunction.h"
#include "System/GameDataUtil.h"
#include "Util/ItemUtil.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"
#include "Util/StageInputFunction.h"

namespace {
NERVE_IMPL(RaceManStart, Wait);
NERVE_IMPL(RaceManStart, RaceRank1);
NERVE_IMPL(RaceManStart, RaceRank2);
NERVE_IMPL(RaceManStart, RaceRank3);
NERVE_IMPL(RaceManStart, RaceRankLose);
NERVE_IMPL(RaceManStart, Reaction);
NERVE_IMPL(RaceManStart, EventPrize);
NERVE_IMPL(RaceManStart, EventPrizeCoin);
NERVE_IMPL(RaceManStart, End);
NERVE_IMPL(RaceManStart, AppearCoin);
NERVE_IMPL(RaceManStart, AppearCoinEnd);
NERVE_IMPL(RaceManStart, Talk);
NERVE_IMPL(RaceManStart, EventLose);

NERVES_MAKE_STRUCT(RaceManStart, Wait, RaceRank1, RaceRank2, RaceRank3, RaceRankLose, Reaction,
                   EventPrize, EventPrizeCoin, End);
NERVES_MAKE_NOSTRUCT(RaceManStart, AppearCoin, AppearCoinEnd, Talk, EventLose);
}  // namespace

RaceManStart::RaceManStart(const char* name) : al::LiveActor(name) {}

// NON_MATCHING: register allocation and stack layout differences in large function
void RaceManStart::init(const al::ActorInitInfo& initInfo) {
    al::initActorWithArchiveName(this, initInfo, "NokonokoNpc", nullptr);
    al::tryGetStringArg(&mStageNameLv1, initInfo, "StageNameLv1");
    al::tryGetArg(&mScenarioNoLv1, initInfo, "ScenarioNoLv1");
    al::tryGetStringArg(&mStartPosLv1, initInfo, "StartPosLv1");
    al::tryGetStringArg(&mStageNameLv2, initInfo, "StageNameLv2");
    al::tryGetArg(&mScenarioNoLv2, initInfo, "ScenarioNoLv2");
    al::tryGetStringArg(&mStartPosLv2, initInfo, "StartPosLv2");

    mTalkNpcCap = TalkNpcCap::tryCreate(this, initInfo);
    if (mTalkNpcCap) {
        al::initSubActorKeeperNoFile(this, initInfo, 1);
        al::registerSubActorSyncAll(this, mTalkNpcCap);
    }

    const char* mtpAnim = nullptr;
    if (al::tryGetStringArg(&mtpAnim, initInfo, "MtpAnim"))
        al::startMtpAnim(this, mtpAnim);

    mEventFlowExecutor = rs::initEventFlow(this, initInfo, "RaceManStart", nullptr);
    rs::initEventCharacterName(mEventFlowExecutor, initInfo, "RaceMan");
    rs::initEventMovementTurnSeparate(mEventFlowExecutor, initInfo);
    al::initEventReceiver(mEventFlowExecutor, this);
    rs::initEventQueryJudge(mEventFlowExecutor, this);

    mTalkNpcParam = rs::initTalkNpcParam(this, nullptr);
    al::initJointControllerKeeper(this, 1);
    mJointLookAtController = rs::tryCreateAndAppendNpcJointLookAtController(this, mTalkNpcParam);

    if (rs::isDefinedEventCamera(mEventFlowExecutor, "Default"))
        rs::initEventCameraObject(mEventFlowExecutor, initInfo, "Default");
    if (rs::isDefinedEventCamera(mEventFlowExecutor, "Prize"))
        rs::initEventCameraObject(mEventFlowExecutor, initInfo, "Prize");

    {
        al::PlacementInfo placementInfo;
        al::getLinksInfo(&placementInfo, initInfo, "ShineActor");
        mShineActorId = new al::PlacementId();
        al::tryGetPlacementId(mShineActorId, placementInfo);
    }

    {
        al::PlacementInfo placementInfo;
        al::tryGetLinksInfo(&placementInfo, initInfo, "ShineActorMoonLockOpened");
        mShineActorMoonLockId = new al::PlacementId();
        al::tryGetPlacementId(mShineActorMoonLockId, placementInfo);
    }

    GameDataHolderAccessor accessor1(this);
    bool isGotShine = GameDataFunction::isGotShine(accessor1, mShineActorId);
    bool isOpenMoonRock = GameDataFunction::isOpenMoonRock(accessor1);
    mEndRaceLv = isGotShine & (isOpenMoonRock | GameDataFunction::isWorldPeach(accessor1));

    GameDataHolderAccessor accessor2(this);
    bool isGotShine2 = GameDataFunction::isGotShine(accessor2, mShineActorMoonLockId);
    bool isOpenMoonRock2 = GameDataFunction::isOpenMoonRock(accessor2);
    bool unlocked = isOpenMoonRock2 | GameDataFunction::isWorldPeach(accessor2);
    bool isGotShine2b = GameDataFunction::isGotShine(accessor2, mShineActorMoonLockId);
    bool isRaceWin = !isGotShine2b && GameDataFunction::isRaceWin(accessor2);
    mNextRaceLv = unlocked & (isGotShine2 | isRaceWin);

    GameDataHolderAccessor accessor3(this);
    const char* playerStartId = GameDataFunction::tryGetPlayerStartId(accessor3);

    bool shouldProceed;
    GameDataHolderAccessor accessor4(this);
    if (GameDataFunction::isRaceWin(accessor4)) {
        mRaceResult = 1;
        shouldProceed = true;
    } else if (GameDataFunction::isRaceResultSecond(accessor4)) {
        mRaceResult = 2;
        shouldProceed = true;
    } else if (GameDataFunction::isRaceResultThird(accessor4)) {
        mRaceResult = 3;
        shouldProceed = true;
    } else if (GameDataFunction::isRaceLose(accessor4)) {
        mRaceResult = 4;
        shouldProceed = true;
    } else {
        mRaceResult = -1;
        GameDataHolderAccessor accessor5(this);
        shouldProceed = GameDataFunction::isRaceCancel(accessor5);
    }

    if (playerStartId && shouldProceed) {
        const char* startPos = mEndRaceLv != 0 ? mStartPosLv2 : mStartPosLv1;
        bool isEqual = al::isEqualString(startPos, playerStartId);
        al::initNerve(this, &NrvRaceManStart.Wait, 1);
        mAddDemoInfo = al::registDemoRequesterToAddDemoInfo(this, initInfo, 0);

        if (isEqual) {
            bool isLv2 = mEndRaceLv == 1;
            mIsPlayFirstTimeThisScene = true;
            mIsPlayFirstTimeThisSceneLv2 = isLv2;

            GameDataHolderAccessor accessor6(this);
            if (GameDataFunction::isRaceCancel(accessor6)) {
                mIsRaceCancel = true;
                al::setNerve(this, &NrvRaceManStart.Wait);
            } else {
                mIsEventStarted = rs::tryStartEventCutSceneDemo(this);

                GameDataHolderAccessor accessor7(this);
                const char* worldName = GameDataFunction::getWorldDevelopNameCurrent(accessor7);
                bool isBigWorld =
                    al::isEqualString(worldName, "Lake") || al::isEqualString(worldName, "Clash") ||
                    al::isEqualString(worldName, "Sea") || al::isEqualString(worldName, "Lava") ||
                    al::isEqualString(worldName, "Sky") || al::isEqualString(worldName, "Moon");

                switch (mRaceResult) {
                case 1:
                    al::invalidateClipping(this);
                    al::setNerve(this, &NrvRaceManStart.RaceRank1);
                    rs::tryInitItem(this, 10, initInfo, false);
                    mCoinMax = isBigWorld ? 20 : 10;
                    break;
                case 2:
                    al::invalidateClipping(this);
                    rs::tryInitItem(this, 10, initInfo, false);
                    mCoinMax = isBigWorld ? 4 : 2;
                    al::setNerve(this, &NrvRaceManStart.RaceRank2);
                    break;
                case 3:
                    al::invalidateClipping(this);
                    rs::tryInitItem(this, 10, initInfo, false);
                    mCoinMax = isBigWorld ? 2 : 1;
                    al::setNerve(this, &NrvRaceManStart.RaceRank3);
                    break;
                case 4:
                case 5:
                    al::setNerve(this, &NrvRaceManStart.RaceRankLose);
                    break;
                }
            }
        }
    } else {
        al::initNerve(this, &NrvRaceManStart.Wait, 1);
        mAddDemoInfo = al::registDemoRequesterToAddDemoInfo(this, initInfo, 0);
    }

    mNpcStateReaction = NpcStateReaction::create(this, nullptr);
    al::initNerveState(this, mNpcStateReaction, &NrvRaceManStart.Reaction, nullptr);
    GameDataFunction::setRaceStartNpcTrans(this);
    makeActorAlive();
}

bool RaceManStart::receiveEvent(const al::EventFlowEventData* data) {
    if (!al::isEventName(data, "GivePrize"))
        return false;
    rs::addDemoRacePrizeCoin(this);
    al::setNerve(this, &AppearCoin);
    return true;
}

// NON_MATCHING: compiler tail-merging optimization shares csel differently
const char* RaceManStart::judgeQuery(const char* query) const {
    if (al::isEqualString(query, "IsPrizeCoinEnd")) {
        if (al::isNerve(this, &AppearCoinEnd))
            return "IsEnd";
        return "IsNotYet";
    }
    if (al::isEqualString(query, "JudgeIsGotShineFirst")) {
        if (GameDataFunction::isGotShine(this, mShineActorId))
            return "IsGot";
        return "IsNotYet";
    }
    if (al::isEqualString(query, "JudgeIsGotShineSecond")) {
        if (GameDataFunction::isGotShine(this, mShineActorMoonLockId))
            return "IsGot";
        return "IsNotYet";
    }
    if (al::isEqualString(query, "JudgeIsPlayFirstTimeThisScene")) {
        if (mIsPlayFirstTimeThisScene)
            return "IsDone";
        return "IsNotYet";
    }
    if (al::isEqualString(query, "JudgeIsPlayFirstTimeThisSceneLv2")) {
        if (mIsPlayFirstTimeThisSceneLv2)
            return "IsDone";
        return "IsNotYet";
    }
    if (al::isEqualString(query, "JudgeEndRaceLv"))
        return mEndRaceLv != 0 ? "IsLv2" : "IsLv1";
    if (al::isEqualString(query, "JudgeNextRaceLv"))
        return mNextRaceLv != 0 ? "IsLv2" : "IsLv1";
    if (al::isEqualString(query, "JudgeIsKidsMode")) {
        if (rs::isKidsMode(this))
            return "Yes";
        return "No";
    }
    if (al::isEqualString(query, "JudgeIsSeparatePlayMode")) {
        if (rs::isSeparatePlay(this))
            return "Yes";
        return "No";
    }
    return nullptr;
}

void RaceManStart::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    rs::attackSensorNpcCommon(self, other);
}

bool RaceManStart::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) {
    if (rs::isMsgPlayerDisregardHomingAttack(msg) || rs::isMsgPlayerDisregardTargetMarker(msg))
        return true;

    if ((!al::isNerve(this, &NrvRaceManStart.Reaction) ||
         (al::isNerve(this, &NrvRaceManStart.Reaction) && al::isGreaterEqualStep(this, 10))) &&
        mNpcStateReaction->receiveMsg(msg, other, self)) {
        al::setNerve(this, &NrvRaceManStart.Reaction);
        return true;
    }

    return mNpcStateReaction->receiveMsgNoReaction(msg, other, self);
}

void RaceManStart::control() {
    if (mJointLookAtController)
        rs::updateNpcJointLookAtController(mJointLookAtController);
    rs::tryUpdateNpcEyeLineAnim(this, mTalkNpcParam);
}

void RaceManStart::exeWait() {
    if (al::isFirstStep(this)) {
        al::tryStartActionIfNotPlaying(this, "Wait");
        rs::startEventFlow(mEventFlowExecutor, "Wait");
    }
    if (rs::updateEventFlow(mEventFlowExecutor))
        al::setNerve(this, &Talk);
}

void RaceManStart::exeRaceRank1() {
    if (al::isFirstStep(this))
        al::startAction(this, "Wait");

    if (!mIsEventStarted && !rs::tryStartEventCutSceneDemo(this))
        return;

    al::addDemoActorFromAddDemoInfo(this, mAddDemoInfo);
    bool gotShine1 = GameDataFunction::isGotShine(this, mShineActorId);
    bool gotShine2 = GameDataFunction::isGotShine(this, mShineActorMoonLockId);
    s32 endRaceLv = mEndRaceLv;

    if ((gotShine1 || endRaceLv != 0) && (gotShine2 || endRaceLv != 1)) {
        rs::startEventFlow(mEventFlowExecutor, "PrizeCoin10");
        al::setNerve(this, &NrvRaceManStart.EventPrizeCoin);
    } else {
        rs::startEventFlow(mEventFlowExecutor, "PrizeMoon");
        al::setNerve(this, &NrvRaceManStart.EventPrize);
    }
}

void RaceManStart::exeRaceRankCoin() {
    if (al::isFirstStep(this))
        al::startAction(this, "Wait");

    if (!mIsEventStarted && !rs::tryStartEventCutSceneDemo(this))
        return;

    rs::startEventFlow(mEventFlowExecutor, "PrizeCoin");
    al::setNerve(this, &NrvRaceManStart.EventPrizeCoin);
}

void RaceManStart::exeRaceRank2() {
    exeRaceRankCoin();
}

void RaceManStart::exeRaceRank3() {
    exeRaceRankCoin();
}

void RaceManStart::exeRaceRankLose() {
    if (al::isFirstStep(this))
        al::startAction(this, "Wait");

    if (!mIsEventStarted && !rs::tryStartEventCutSceneDemo(this))
        return;

    rs::startEventFlow(mEventFlowExecutor, "Lose");
    al::setNerve(this, &EventLose);
}

void RaceManStart::exeEventPrize() {
    if (!rs::updateEventFlow(mEventFlowExecutor))
        return;

    if (mEndRaceLv == 0)
        al::tryOnStageSwitch(this, "SwitchRaceManPrizeLv1On");
    else if (mEndRaceLv == 1)
        al::tryOnStageSwitch(this, "SwitchRaceManPrizeLv2On");

    al::validateClipping(this);
    al::setNerve(this, &NrvRaceManStart.Wait);
}

void RaceManStart::exeEventPrizeCoin() {
    rs::updateEventFlow(mEventFlowExecutor);
}

void RaceManStart::exeAppearCoin() {
    const sead::Vector3f& headPos = rs::getPlayerHeadPos(this);
    s32 step = al::getNerveStep(this);
    if (rs::tryAppearMultiCoinFromObj(this, headPos, step, 100.0f)) {
        al::startHitReaction(this, "コイン出現");
        mCoinCount++;
    }
    if (mCoinCount == mCoinMax)
        al::setNerve(this, &AppearCoinEnd);
}

void RaceManStart::exeAppearCoinEnd() {
    if (al::isFirstStep(this))
        rs::startEventFlow(mEventFlowExecutor, "PrizeEnd");
    if (rs::updateEventFlow(mEventFlowExecutor)) {
        al::validateClipping(this);
        al::setNerve(this, &NrvRaceManStart.Wait);
    }
}

void RaceManStart::exeEventRetry() {
    if (!rs::updateEventFlow(mEventFlowExecutor))
        return;
    if ((u32)mNextRaceLv <= 1)
        al::setNerve(this, &NrvRaceManStart.End);
    else
        al::setNerve(this, &NrvRaceManStart.Wait);
}

void RaceManStart::exeEventLose() {
    if (rs::updateEventFlow(mEventFlowExecutor))
        al::setNerve(this, &NrvRaceManStart.Wait);
}

void RaceManStart::exeTalk() {
    if (rs::updateEventFlow(mEventFlowExecutor))
        al::setNerve(this, &NrvRaceManStart.End);
}

void RaceManStart::exeEnd() {
    if (mNextRaceLv == 0) {
        auto* holder = GameDataFunction::getGameDataHolder(this);
        ChangeStageInfo info(holder, mStartPosLv1, mStageNameLv1, true, mScenarioNoLv1,
                             ChangeStageInfo::RESET_MINI_GAME2);
        GameDataFunction::setRaceRivalLevel(this, 0);
        GameDataFunction::tryChangeNextStageWithStartRaceFlag(this, &info);
    } else if (mNextRaceLv == 1) {
        auto* holder = GameDataFunction::getGameDataHolder(this);
        ChangeStageInfo info(holder, mStartPosLv2, mStageNameLv2, true, mScenarioNoLv2,
                             ChangeStageInfo::RESET_MINI_GAME2);
        GameDataFunction::setRaceRivalLevel(this, 1);
        GameDataFunction::tryChangeNextStageWithStartRaceFlag(this, &info);
    }
}

void RaceManStart::exeReaction() {
    if (al::updateNerveState(this))
        al::setNerve(this, &NrvRaceManStart.Wait);
}
