#include "Npc/FukuwaraiNpc.h"

#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/HitSensor/SensorFunction.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Message/MessageHolder.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementId.h"
#include "Library/Placement/PlacementInfo.h"

#include "Npc/FukuwaraiFaceParts.h"
#include "Npc/FukuwaraiWatcher.h"
#include "Npc/NpcStateReaction.h"
#include "Npc/TalkNpcCap.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/DemoUtil.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerDemoUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(FukuwaraiNpc, Wait);
NERVE_IMPL_(FukuwaraiNpc, ReactionWait, Reaction);
NERVE_IMPL_(FukuwaraiNpc, ReactionAskEnd, Reaction);
NERVE_IMPL_(FukuwaraiNpc, ReactionPlay, Reaction);
NERVE_IMPL(FukuwaraiNpc, Play);
NERVE_IMPL_(FukuwaraiNpc, AskEnd, Play);
NERVE_IMPL_(FukuwaraiNpc, SetStartPosition, Wait);
NERVE_IMPL(FukuwaraiNpc, Memorize);
NERVE_IMPL(FukuwaraiNpc, EventMemorize);
NERVE_IMPL_(FukuwaraiNpc, WaitStartResult, Wait);
NERVE_IMPL(FukuwaraiNpc, EventStartResult);
NERVE_IMPL(FukuwaraiNpc, EventStartResultEnd);
NERVE_IMPL(FukuwaraiNpc, EventResult);
NERVE_IMPL_(FukuwaraiNpc, EventResultJudgeEnd, EventResult);
NERVE_IMPL_(FukuwaraiNpc, EventEndResult, EventResult);
NERVE_IMPL(FukuwaraiNpc, EventEnd);

NERVES_MAKE_NOSTRUCT(FukuwaraiNpc, Wait, ReactionWait, ReactionAskEnd, ReactionPlay, Play, AskEnd,
                     SetStartPosition, Memorize, EventMemorize, WaitStartResult, EventStartResult,
                     EventStartResultEnd, EventResult, EventResultJudgeEnd, EventEndResult,
                     EventEnd);
}  // namespace

FukuwaraiNpc::FukuwaraiNpc(const char* name, FukuwaraiWatcher* watcher, al::LiveActor* face,
                           bool isMario)
    : al::LiveActor(name), mWatcher(watcher), mIsMarioFace(isMario) {}

// NON_MATCHING: stack slot offset differences (sp+0x8 vs sp+0x10 for SafeString/PlacementInfo)
void FukuwaraiNpc::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, "KinopioNpc", "Fukuwarai");

    mTalkNpcCap = TalkNpcCap::tryCreate(this, info);
    if (mTalkNpcCap) {
        al::initSubActorKeeperNoFile(this, info, 1);
        al::registerSubActor(this, mTalkNpcCap);
    }

    mEventFlowExecutor = rs::initEventFlow(this, info, nullptr, nullptr);
    al::initEventReceiver(mEventFlowExecutor, this);
    rs::initEventQueryJudge(mEventFlowExecutor, this);
    rs::initEventCameraObject(mEventFlowExecutor, info, "Object");
    rs::initEventMovementTurnSeparate(mEventFlowExecutor, info);
    rs::startEventFlow(mEventFlowExecutor, "Wait");

    mTalkNpcParam = rs::initTalkNpcParam(this, nullptr);

    al::initJointControllerKeeper(this, 1);
    mNpcJointLookAtController = rs::tryCreateAndAppendNpcJointLookAtController(this, mTalkNpcParam);

    al::MessageTagDataHolder* tagHolder = al::initMessageTagDataHolder(2);
    al::registerMessageTagDataScore(tagHolder, "Score", &mScore);
    al::registerMessageTagDataScore(tagHolder, "SuccessScore", &mSuccessScore);
    rs::initEventMessageTagDataHolder(mEventFlowExecutor, tagHolder);

    {
        al::PlacementInfo placementInfo;
        al::getLinksInfo(&placementInfo, info, "ShineActor");
        mShineActorPlacementId = new al::PlacementId();
        al::tryGetPlacementId(mShineActorPlacementId, placementInfo);
    }
    {
        al::PlacementInfo placementInfo;
        al::tryGetLinksInfo(&placementInfo, info, "ShineActorMoonLockOpened");
        mMoonLockPlacementId = new al::PlacementId();
        al::tryGetPlacementId(mMoonLockPlacementId, placementInfo);
    }

    bool isNextLv = true;
    bool isGotShine =
        GameDataFunction::isGotShine(GameDataHolderAccessor(this), mShineActorPlacementId);
    if (!GameDataFunction::isOpenMoonRock(GameDataHolderAccessor(this)))
        isNextLv = mIsMarioFace;
    if ((isGotShine & isNextLv) == 1)
        mSuccessScore = 80;

    al::tryGetLinksQT(&mQuat, &mPosition, info, "FukuwaraiStartPos");

    al::initNerve(this, &Wait, 3);
    mNpcStateReaction = NpcStateReaction::create(this, nullptr);
    al::initNerveState(this, mNpcStateReaction, &ReactionWait, u8"リアクション[Wait]");
    al::initNerveState(this, mNpcStateReaction, &ReactionAskEnd, u8"リアクション[AskEnd]");
    al::initNerveState(this, mNpcStateReaction, &ReactionPlay, u8"リアクション[Play]");

    makeActorAlive();
}

s32 FukuwaraiNpc::getNextLv() const {
    bool isGotShine =
        GameDataFunction::isGotShine(GameDataHolderAccessor(this), mShineActorPlacementId);
    bool isNextLv = GameDataFunction::isOpenMoonRock(GameDataHolderAccessor(this)) || mIsMarioFace;
    return isGotShine & (s32)isNextLv;
}

void FukuwaraiNpc::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (!al::isSensorPlayer(other))
        return;
    if (al::isSensorPlayerDecoration(other))
        return;

    if (al::isSensorName(self, "Talk")) {
        if (al::isNerve(this, &Play)) {
            rs::startEventFlow(mEventFlowExecutor, "AskEnd");
            al::setNerve(this, &AskEnd);
            return;
        }
        if (al::isNerve(this, &AskEnd)) {
            al::setNerve(this, &AskEnd);
            return;
        }
    }

    rs::attackSensorNpcCommon(self, other);
}

// NON_MATCHING: extra adrp+add for nerve base address reload (regalloc)
bool FukuwaraiNpc::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                              al::HitSensor* self) {
    if (rs::isMsgPlayerDisregardHomingAttack(message) ||
        rs::isMsgPlayerDisregardTargetMarker(message))
        return true;

    if (!mNpcStateReaction->receiveMsg(message, other, self))
        return mNpcStateReaction->receiveMsgNoReaction(message, other, self);

    if (!al::isNerve(this, &ReactionWait) && !al::isNerve(this, &ReactionAskEnd) &&
        !al::isNerve(this, &ReactionPlay)) {
        if (al::isNerve(this, &Wait))
            al::setNerve(this, &ReactionWait);
        else
            al::setNerve(this, al::isNerve(this, &AskEnd) ? (const al::Nerve*)&ReactionAskEnd :
                                                            (const al::Nerve*)&ReactionPlay);
    }

    return true;
}

void FukuwaraiNpc::control() {
    if (mNpcJointLookAtController)
        rs::updateNpcJointLookAtController(mNpcJointLookAtController);
    rs::tryUpdateNpcEyeLineAnim(this, mTalkNpcParam);
}

bool FukuwaraiNpc::receiveEvent(const al::EventFlowEventData* data) {
    if (al::isNerve(this, &Wait)) {
        if (!al::isEventName(data, "SetStartPosition"))
            return false;
        al::invalidateClipping(this);
        rs::addDemoActor(mWatcher, true);
        rs::replaceDemoPlayer(this, mPosition, mQuat);
        rs::forcePutOnDemoCap(this);
        al::setNerve(this, &SetStartPosition);
        return true;
    }

    if (al::isNerve(this, &SetStartPosition)) {
        if (!al::isEventName(data, "Memorize"))
            return false;
        al::setNerve(this, &Memorize);
        return true;
    }

    if (al::isNerve(this, &Memorize) || al::isNerve(this, &EventMemorize)) {
        if (!al::isEventName(data, "PlayStart"))
            return false;
        mIsPlayedThisScene = true;
        if (getNextLv())
            mIsPlayedThisSceneLv2 = true;
        al::setNerve(this, &Play);
        return true;
    }

    if (al::isNerve(this, &Play) || al::isNerve(this, &AskEnd)) {
        if (!al::isEventName(data, "WaitResult"))
            return false;
        rs::addDemoActor(mWatcher, true);
        al::setNerve(this, &WaitStartResult);
        return true;
    }

    if (al::isNerve(this, &WaitStartResult) || al::isNerve(this, &EventStartResult) ||
        al::isNerve(this, &EventStartResultEnd))
        return false;

    if (al::isNerve(this, &EventResult)) {
        if (!al::isEventName(data, "JudgeEnd"))
            return false;
        al::setNerve(this, &EventResultJudgeEnd);
        return true;
    }

    if (al::isNerve(this, &EventResultJudgeEnd))
        return false;

    if (al::isNerve(this, &EventEndResult)) {
        if (!al::isEventName(data, "End"))
            return false;
        if (getNextLv())
            mSuccessScore = 80;
        al::setNerve(this, &EventEnd);
        return true;
    }

    al::isNerve(this, &EventEnd);
    return false;
}

// NON_MATCHING: regalloc and exit point sharing differences in csel chains
const char* FukuwaraiNpc::judgeQuery(const char* query) const {
    if (al::isEqualString(query, "JudgeSuccess")) {
        s32 score = mScore;
        if (score > 89)
            return "VeryGood";

        s32 successScore = mSuccessScore;
        const char* bad = "VeryBad";
        const char* normal = "Normal";
        if (score > 29)
            bad = "Bad";
        if (score <= 49)
            normal = bad;
        if (score < successScore)
            return normal;
        return "Good";
    }

    if (al::isEqualString(query, "JudgeIsGotShineFirst")) {
        if (GameDataFunction::isGotShine(GameDataHolderAccessor(this), mShineActorPlacementId))
            return "IsGot";
        return "IsNotYet";
    }

    if (al::isEqualString(query, "JudgeIsGotShineSecond")) {
        if (GameDataFunction::isGotShine(GameDataHolderAccessor(this), mMoonLockPlacementId))
            return "IsGot";
        return "IsNotYet";
    }

    if (al::isEqualString(query, "JudgeIsPlayFirstTimeThisScene")) {
        if (mIsPlayedThisScene)
            return "IsDone";
        return "IsNotYet";
    }

    if (al::isEqualString(query, "JudgeIsPlayFirstTimeThisSceneLv2")) {
        if (mIsPlayedThisSceneLv2)
            return "IsDone";
        return "IsNotYet";
    }

    if (al::isEqualString(query, "JudgeLv")) {
        if (getNextLv())
            return "IsLv2";
        return "IsLv1";
    }

    if (al::isEqualString(query, "JudgeIsKuriboMario")) {
        if (mIsMarioFace)
            return "No";
        return mIsCompleteKuriboMario ? "Yes" : "No";
    }

    return nullptr;
}

void FukuwaraiNpc::exeWait() {
    if (al::isFirstStep(this))
        al::tryStartActionIfNotPlaying(this, "Wait");
    rs::updateEventFlow(mEventFlowExecutor);
}

void FukuwaraiNpc::exeReaction() {
    if (al::updateNerveState(this)) {
        const al::Nerve* nerve = &Wait;
        if (!al::isNerve(this, &ReactionWait)) {
            if (al::isNerve(this, &ReactionAskEnd))
                nerve = &AskEnd;
            else
                nerve = &Play;
        }
        al::setNerve(this, nerve);
    }
}

void FukuwaraiNpc::exePlay() {
    if (al::isNerve(this, &AskEnd) && al::isGreaterStep(this, 3)) {
        rs::startEventFlow(mEventFlowExecutor, "Play");
        al::setNerve(this, &Play);
        return;
    }
    rs::updateEventFlow(mEventFlowExecutor);
}

void FukuwaraiNpc::exeMemorize() {
    rs::startEventFlow(mEventFlowExecutor, "Memorize");
    al::setNerve(this, &EventMemorize);
}

void FukuwaraiNpc::exeEventMemorize() {
    rs::updateEventFlow(mEventFlowExecutor);
}

void FukuwaraiNpc::exeEventStartResult() {
    if (rs::updateEventFlow(mEventFlowExecutor)) {
        rs::forcePutOnDemoCap(this);
        al::setNerve(this, &EventStartResultEnd);
    }
}

void FukuwaraiNpc::exeEventStartResultEnd() {}

void FukuwaraiNpc::exeEventResult() {
    rs::updateEventFlow(mEventFlowExecutor);
}

void FukuwaraiNpc::exeEventEnd() {}

bool FukuwaraiNpc::isCompleteKuriboMario() const {
    s32 partsNum = mWatcher->getPartsNum();
    if (partsNum < 1)
        return true;

    for (s32 i = 0; i < partsNum; i++) {
        FukuwaraiFaceParts* parts = mWatcher->getParts(i);
        if (!parts->isKuriboMarioParts() && parts->isPlaced())
            return false;
        if (parts->isKuriboMarioParts() && !parts->isPlaced())
            return false;
        if (parts->isKuriboMarioParts() && parts->isPlaced()) {
            if (parts->calcScoreDistRate() < 0.2f)
                return false;
        }
    }

    return true;
}

void FukuwaraiNpc::startResultWipe() {
    al::invalidateClipping(this);
    rs::startEventFlow(mEventFlowExecutor, "StartResult");
    al::setNerve(this, &EventStartResult);
}

bool FukuwaraiNpc::isEventStartResultEnd() const {
    return al::isNerve(this, &EventStartResultEnd);
}

void FukuwaraiNpc::startResult(s32 score) {
    mScore = score;
    mIsCompleteKuriboMario = isCompleteKuriboMario();
    al::invalidateClipping(this);
    rs::startEventFlow(mEventFlowExecutor, "Result");
    al::setNerve(this, &EventResult);
}

bool FukuwaraiNpc::isEventResultJudgeEnd() const {
    return al::isNerve(this, &EventResultJudgeEnd);
}

void FukuwaraiNpc::startEndResult() {
    rs::startEventFlow(mEventFlowExecutor, "EndResult");
    al::setNerve(this, &EventEndResult);
}

void FukuwaraiNpc::reset() {
    al::EventFlowExecutor* executor = mEventFlowExecutor;
    mScore = 0;
    rs::startEventFlow(executor, "Wait");
    al::setNerve(this, &Wait);
}

bool FukuwaraiNpc::isSetStartPosition() const {
    return al::isNerve(this, &SetStartPosition);
}

bool FukuwaraiNpc::isMemorize() const {
    return al::isNerve(this, &Memorize);
}

bool FukuwaraiNpc::isPlay() const {
    if (al::isNerve(this, &Play))
        return true;
    return al::isNerve(this, &AskEnd);
}

bool FukuwaraiNpc::isWaitStartResult() const {
    return al::isNerve(this, &WaitStartResult);
}

bool FukuwaraiNpc::isEnd() const {
    return al::isNerve(this, &EventEnd);
}
