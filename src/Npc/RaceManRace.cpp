#include "Npc/RaceManRace.h"

#include <container/seadPtrArray.h>
#include <math/seadQuat.h>
#include <prim/seadSafeString.h>

#include "Library/Area/AreaObj.h"
#include "Library/Area/AreaObjGroup.h"
#include "Library/Area/AreaObjUtil.h"
#include "Library/Base/StringUtil.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/ParabolicPath.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementInfo.h"
#include "Library/Rail/Rail.h"
#include "Library/Resource/Resource.h"
#include "Library/Resource/ResourceFunction.h"
#include "Library/Scene/SceneObjUtil.h"
#include "Library/Se/SeFunction.h"
#include "Library/Stage/StageSwitchUtil.h"
#include "Library/Yaml/ByamlIter.h"

#include "Npc/GhostPlayer.h"
#include "Npc/RaceManGoal.h"
#include "Npc/RaceManThrowCap.h"
#include "Npc/TalkNpcCap.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolder.h"
#include "System/GameDataFile.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerDemoUtil.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

struct RaceRankAreaInfo {
    al::AreaObj* mAreaObj;
    s32 mRankPriority;
    al::Rail* mRail;
};

struct RaceRankEntry {
    s32 priority;
    f32 distH;
    f32 railCoord;
    bool isPlayer;
    bool isDone;
};

namespace {
NERVE_IMPL(RaceManRace, StartWait);
NERVE_IMPL(RaceManRace, Race);
NERVE_IMPL(RaceManRace, WaitReEnterStage);
NERVE_IMPL(RaceManRace, GoalNpcLoseWait);
NERVE_IMPL(RaceManRace, GoalNpcWinPlayerGoal);
NERVE_IMPL(RaceManRace, GoalNpcWin);
NERVE_IMPL(RaceManRace, StartRetry);
NERVE_IMPL(RaceManRace, EventRetry);
NERVE_IMPL(RaceManRace, WaitEndStage);
NERVE_IMPL(RaceManRace, AttachJumpPlayer);
NERVE_IMPL(RaceManRace, AttachJumpPlayerEnd);
NERVE_IMPL(RaceManRace, ResultTurn);
NERVE_IMPL(RaceManRace, Result);

NERVES_MAKE_STRUCT(RaceManRace, StartWait, Race, WaitReEnterStage, GoalNpcLoseWait,
                   GoalNpcWinPlayerGoal, GoalNpcWin, StartRetry, EventRetry, WaitEndStage,
                   AttachJumpPlayer, AttachJumpPlayerEnd, ResultTurn, Result);
NERVES_MAKE_NOSTRUCT(RaceManRace, Race, GoalNpcLoseWait, GoalNpcWinPlayerGoal);

s32 compareRaceRankEntry(const RaceRankEntry* a, const RaceRankEntry* b) {
    if (a->isDone && !b->isDone)
        return -1;
    if (!a->isDone && b->isDone)
        return 1;
    if (a->priority > b->priority)
        return -1;
    if (a->priority == b->priority) {
        if (a->railCoord > b->railCoord)
            return -1;
        if (a->railCoord == b->railCoord && a->distH < b->distH)
            return -1;
    }
    if (b->isDone && !a->isDone)
        return 1;
    if (!b->isDone && a->isDone)
        return 0;
    if (b->priority > a->priority)
        return 1;
    if (b->priority != a->priority)
        return 0;
    if (b->railCoord > a->railCoord)
        return 1;
    if (b->railCoord != a->railCoord)
        return 0;
    return b->distH < a->distH;
}
}  // namespace

RaceManRace::RaceManRace(const char* name) : al::LiveActor(name) {}

// NON_MATCHING: stack frame size, regswap, imm/offset differences in string construction and nerve
// addresses (STRUCT nerves compile to 3 instructions vs target's 2)
void RaceManRace::init(const al::ActorInitInfo& initInfo) {
    al::initActorWithArchiveName(this, initInfo, "NokonokoNpc", "RaceManStart");
    al::setSceneObj(this, this, 46);
    mRaceManGoal =
        static_cast<RaceManGoal*>(al::createLinksActorFromFactory(initInfo, "RaceManGoal", 0));
    if (!mRaceManGoal) {
        kill();
        return;
    }
    mTalkNpcCap = TalkNpcCap::tryCreate(this, initInfo);
    if (mTalkNpcCap) {
        al::initSubActorKeeperNoFile(this, initInfo, 1);
        al::registerSubActorSyncAll(this, mTalkNpcCap);
    }
    mEventFlowExecutor = rs::initEventFlow(this, initInfo, "RaceMan", nullptr);
    rs::initEventCharacterName(mEventFlowExecutor, initInfo, "RaceMan");
    al::initEventReceiver(mEventFlowExecutor, this);
    if (rs::isDefinedEventCamera(mEventFlowExecutor, "Default"))
        rs::initEventCameraTalk2(mEventFlowExecutor, initInfo, "Default");

    s32 raceRivalLevel = GameDataFunction::getRaceRivalLevel(this);
    sead::Vector3f goalPos;
    mRaceManGoal->calcMarioJointQuatPos(nullptr, &goalPos);

    for (s32 i = 0; i < mGhostPlayers.capacity(); i++) {
        const char* stageName =
            GameDataFunction::getGameDataHolder(this)->getGameDataFile()->getStageNameCurrent();
        al::StringTmp<256> resourceName("StageData/%sMap", stageName);
        auto* resource = al::findOrCreateResource(resourceName, nullptr);
        stageName =
            GameDataFunction::getGameDataHolder(this)->getGameDataFile()->getStageNameCurrent();

        sead::FixedSafeString<512> byamlKey;

        s32 scenarioNo = GameDataFunction::getScenarioNo(this);
        if (raceRivalLevel < 0)
            byamlKey.format("%s_%d_x_%02d", stageName, scenarioNo, i);
        else
            byamlKey.format("%s_%d_%d_%02d", stageName, scenarioNo, raceRivalLevel, i);

        al::StringTmp<512> byamlPath("%s.byml", byamlKey.cstr());

        if (resource->isExistFile(byamlPath)) {
            // found with exact level; proceed
        } else {
            scenarioNo = GameDataFunction::getScenarioNo(this);
            byamlKey.format("%s_%d_x_%02d", stageName, scenarioNo, i);
            al::StringTmp<512> byamlPath2("%s.byml", byamlKey.cstr());
            if (!resource->isExistFile(byamlPath2))
                continue;
        }

        auto* ghostName = new al::StringTmp<256>(u8"駆けっこゴースト[%d]", i);
        auto* ghost = new GhostPlayer(ghostName->cstr(), i,
                                      (i == 3 && raceRivalLevel == 1) ? 4 : -1);
        auto* byamlIter =
            new al::ByamlIter(resource->getByml(al::StringTmp<512>("%s.byml", byamlKey.cstr())));
        ghost->initGhostPlayDataFromByaml(byamlIter);
        ghost->initWithArchiveName(initInfo, "NokonokoNpc", "RaceMan");
        ghost->_1a0 = goalPos;
        al::invalidateClipping(ghost);
        ghost->makeActorAlive();

        auto* cap = TalkNpcCap::tryCreate(ghost, initInfo);
        if (cap) {
            al::initSubActorKeeperNoFile(ghost, initInfo, 1);
            al::registerSubActorSyncAll(ghost, cap);
            auto* throwCap = new RaceManThrowCap("RaceManCap");
            throwCap->init(initInfo);
            ghost->initThrowCap(throwCap);
        }
        mGhostPlayers.pushBack(ghost);
    }

    al::invalidateClipping(this);
    al::initNerve(this, &NrvRaceManRace.StartWait, 0);
    makeActorAlive();
    al::hideModel(this);
    for (s32 i = 0; i < mGhostPlayers.size(); i++)
        mGhostPlayers.at(i)->appear();

    mCameraTicket =
        al::initObjectCameraNoPlacementInfo(this, u8"ゴール時のカメラ", nullptr);
    mParabolicPath = new al::ParabolicPath();

    mAreaObjGroup = al::createLinkAreaGroup(this, initInfo, "RaceManRacePriorRankArea",
                                            u8"順位優先エリア", "AreaCubeBase");
    if (mAreaObjGroup) {
        mRankAreas.allocBuffer(mAreaObjGroup->getSize(), nullptr);
        for (s32 i = 0; i < mAreaObjGroup->getSize(); i++) {
            auto* areaObj = mAreaObjGroup->getAreaObj(i);
            al::PlacementInfo railInfo;
            al::Rail* rail = nullptr;
            if (al::tryGetRailIter(&railInfo, *areaObj->getPlacementInfo(), "Rail")) {
                rail = new al::Rail();
                rail->init(railInfo);
            }
            s32 rankPriority = 0;
            al::tryGetArg(&rankPriority, *areaObj->getPlacementInfo(), "RaceRankPriority");
            auto* info = new RaceRankAreaInfo{areaObj, rankPriority, rail};
            mRankAreas.pushBack(info);
        }
    }

    GameDataFunction::setRaceStartTrans(this);
}

void RaceManRace::control() {}

void RaceManRace::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isNerve(this, &Race))
        return;
    if (al::isNerve(this, &GoalNpcLoseWait))
        return;
    if (al::isSensorPlayer(other))
        al::sendMsgPush(other, self);
}

// NON_MATCHING: compiler tail-calls isMsgPlayerDisregardTargetMarker and shares return-true block
bool RaceManRace::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                             al::HitSensor* self) {
    if (!al::isSensorName(self, "Body"))
        return false;
    if (al::isMsgPlayerDisregard(msg))
        return true;
    if (rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;
    if (rs::isMsgPlayerDisregardTargetMarker(msg))
        return true;
    return false;
}

bool RaceManRace::receiveEvent(const al::EventFlowEventData* data) {
    if (!al::isEventName(data, "RaceRetry"))
        return false;
    al::setNerve(this, &NrvRaceManRace.WaitReEnterStage);
    return true;
}

const char* RaceManRace::getSceneObjName() const {
    return getName();
}

void RaceManRace::startCountDown() {
    al::tryOnStageSwitch(this, "SwitchStartCountDownOn");
}

void RaceManRace::startRace() {
    al::tryOnStageSwitch(this, "SwitchEndCountDownOn");
    if (mGhostPlayers.size() >= 1)
        al::startSe(mGhostPlayers.at(0), "Count2");
    al::setNerve(this, &Race);
}

void RaceManRace::trySetRaceManStep(s32 step) {
    for (s32 i = 0; i < mGhostPlayers.size(); i++) {
        auto* ghost = mGhostPlayers.at(i);
        if (!ghost->isPlayDone())
            ghost->mRaceManStep = step;
    }
}

void RaceManRace::checkGoalRaceMan() {
    for (s32 i = 0; i < mGhostPlayers.size(); i++) {
        auto* ghost = mGhostPlayers.at(i);
        if (!ghost->isPlayDone())
            continue;
        auto* goal = mRaceManGoal;
        if (goal->isAttachedActor(ghost))
            continue;
        if (goal->getAttachedGoalActorNum() == 0) {
            mIsGoalPlayer = true;
            ghost->_19d = true;
        } else {
            if (mIsGoalPlayer)
                ghost->_19d = true;
            else
                ghost->_19e = true;
        }
        goal->attachActor(ghost);
    }
}

// NON_MATCHING: pool allocator pattern, stack layout, csel vs branch differences, regswap
s32 RaceManRace::calcRank() const {
    RaceRankEntry pool[5];
    pool[0].priority = (s32)(uintptr_t)&pool[1];
    pool[1].priority = (s32)(uintptr_t)&pool[2];
    pool[2].priority = (s32)(uintptr_t)&pool[3];
    pool[3].priority = (s32)(uintptr_t)&pool[4];
    *(uintptr_t*)&pool[4] = 0;

    auto* freeHead = &pool[0];

    RaceRankEntry* ptrBuf[5];
    sead::PtrArray<RaceRankEntry> sortArray;
    sortArray.setBuffer(5, ptrBuf);

    const sead::Vector3f& playerPos = rs::getPlayerPos(this);
    f32 playerDistH = al::calcDistanceH(mRaceManGoal, playerPos);

    s32 playerPriority = -1;
    f32 playerRailCoord = -1.0f;

    RaceRankAreaInfo* bestArea = nullptr;
    for (s32 i = 0; i < mRankAreas.size(); i++) {
        auto* areaInfo = mRankAreas.at(i);
        if (al::isInAreaPos(areaInfo->mAreaObj, playerPos)) {
            if (!bestArea || bestArea->mAreaObj->getPriority() < areaInfo->mAreaObj->getPriority())
                bestArea = areaInfo;
        }
    }
    if (bestArea) {
        playerPriority = bestArea->mRankPriority;
        if (bestArea->mRail)
            playerRailCoord = bestArea->mRail->calcNearestRailPosCoord(playerPos, 20.0f);
    }

    {
        auto* entry = freeHead;
        if (freeHead)
            freeHead = *(RaceRankEntry**)freeHead;
        entry->priority = playerPriority;
        entry->distH = playerDistH;
        entry->railCoord = playerRailCoord;
        entry->isPlayer = true;
        entry->isDone = false;
        sortArray.pushBack(entry);
    }

    for (s32 i = 0; i < mGhostPlayers.size(); i++) {
        auto* ghost = mGhostPlayers.at(i);
        const sead::Vector3f& ghostPos = al::getTrans(ghost);
        f32 ghostDistH = al::calcDistanceH(mRaceManGoal, ghostPos);
        bool isDone = ghost->isPlayDone();

        s32 ghostPriority = -1;
        f32 ghostRailCoord = -1.0f;

        RaceRankAreaInfo* ghostBestArea = nullptr;
        for (s32 j = 0; j < mRankAreas.size(); j++) {
            auto* areaInfo = mRankAreas.at(j);
            if (al::isInAreaPos(areaInfo->mAreaObj, ghostPos)) {
                if (!ghostBestArea ||
                    ghostBestArea->mAreaObj->getPriority() < areaInfo->mAreaObj->getPriority())
                    ghostBestArea = areaInfo;
            }
        }
        if (ghostBestArea) {
            ghostPriority = ghostBestArea->mRankPriority;
            if (ghostBestArea->mRail)
                ghostRailCoord = ghostBestArea->mRail->calcNearestRailPosCoord(ghostPos, 20.0f);
            else
                ghostRailCoord = -1.0f;
        }

        {
            auto* entry = freeHead;
            if (freeHead)
                freeHead = *(RaceRankEntry**)freeHead;
            entry->priority = ghostPriority;
            entry->distH = ghostDistH;
            entry->railCoord = ghostRailCoord;
            entry->isPlayer = false;
            entry->isDone = isDone;
            sortArray.pushBack(entry);
        }
    }

    sortArray.sort(compareRaceRankEntry);

    s32 rank = sortArray.size();
    for (s32 i = 0; i < sortArray.size(); i++) {
        if (sortArray.at(i)->isPlayer) {
            rank = i + 1;
            break;
        }
    }

    s32 goalCount = mRaceManGoal->getAttachedGoalActorNum();
    if (rank <= goalCount)
        return goalCount;
    return rank;
}

bool RaceManRace::isGoalNpc(sead::Vector3f* pos) const {
    for (s32 i = 0; i < mGhostPlayers.size(); i++) {
        if (mGhostPlayers.at(i)->isPlayDone()) {
            *pos = al::getTrans(mGhostPlayers.at(i));
            return true;
        }
    }
    return false;
}

void RaceManRace::onGoalPlayer(bool isWin) {
    mIsWin = isWin;
    al::startCamera(this, mCameraTicket, -1);
    if (isWin)
        al::setNerve(this, &GoalNpcLoseWait);
    else
        al::setNerve(this, &GoalNpcWinPlayerGoal);
}

void RaceManRace::onGoalNpc(const sead::Vector3f& pos) {
    al::resetPosition(this, pos);
    al::setNerve(this, &NrvRaceManRace.GoalNpcWin);
}

void RaceManRace::exeGoalNpcLoseWait() {}

void RaceManRace::exeGoalNpcWinPlayerGoal() {}

void RaceManRace::exeGoalNpcWin() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Wait");
        al::hideModelIfShow(this);
        sead::Vector3f jointPos;
        al::calcJointPos(&jointPos, mRaceManGoal, mRaceManGoal->getRaceFirstJointName());
        al::resetPosition(this, jointPos);
    }
}

// NON_MATCHING: nerve address codegen (STRUCT nerve: 3 instr vs 2)
void RaceManRace::startRetry() {
    al::showModelIfHide(this);
    for (s32 i = 0; i < mGhostPlayers.size(); i++)
        mGhostPlayers.at(i)->kill();
    al::setNerve(this, &NrvRaceManRace.StartRetry);
}

// NON_MATCHING: nerve address codegen (STRUCT nerve: 3 instr vs 2), regswap
void RaceManRace::exeStartRetry() {
    if (rs::tryStartEventCutSceneDemo(this)) {
        auto* executor = mEventFlowExecutor;
        const char* entryName = al::isInWater(this) ? "RetryWater" : "Retry";
        rs::startEventFlow(executor, entryName);
        al::setNerve(this, &NrvRaceManRace.EventRetry);
    }
}

void RaceManRace::exeEventRetry() {
    if (rs::updateEventFlow(mEventFlowExecutor))
        al::setNerve(this, &NrvRaceManRace.WaitEndStage);
}

void RaceManRace::exeWaitReEnterStage() {}

void RaceManRace::exeWaitEndStage() {}

bool RaceManRace::isWaitReEnterStage() const {
    return al::isNerve(this, &NrvRaceManRace.WaitReEnterStage);
}

bool RaceManRace::isWaitEndStage() const {
    return al::isNerve(this, &NrvRaceManRace.WaitEndStage);
}

// NON_MATCHING: quat copy uses w-regs (32-bit) vs target x-regs (64-bit), regswap
void RaceManRace::attachDemoPlayerToGoal() {
    sead::Vector3f goalPos;
    mRaceManGoal->calcMarioJointQuatPos(&mGoalQuat, &goalPos);
    mStartQuat = rs::getDemoPlayerQuat(this);

    sead::Vector3f upDir;
    al::calcQuatUp(&upDir, mGoalQuat);
    sead::Vector3f frontDir;
    al::calcQuatFront(&frontDir, rs::getDemoPlayerQuat(this));
    al::makeQuatUpFront(&mGoalQuat, upDir, frontDir);

    rs::forcePutOnDemoCap(this);
    auto* path = mParabolicPath;

    const sead::Vector3f& playerPos = rs::getPlayerPos(this);
    const sead::Vector3f& gravity = al::getGravity(this);
    sead::Vector3f upGrav = {-gravity.x, -gravity.y, -gravity.z};
    path->initFromUpVector(playerPos, goalPos, upGrav, 300.0f);
    al::setNerve(this, &NrvRaceManRace.AttachJumpPlayer);
}

void RaceManRace::exeAttachJumpPlayer() {
    if (al::isFirstStep(this))
        rs::startActionDemoPlayer(this, "Jump");
    f32 rate = al::calcNerveRate(this, -1, 40);
    sead::Vector3f pos;
    mParabolicPath->calcPosition(&pos, rate);
    sead::Quatf quat;
    al::slerpQuat(&quat, mStartQuat, mGoalQuat, rate);
    rs::replaceDemoPlayer(this, pos, quat);
    if (al::isGreaterEqualStep(this, 40))
        al::setNerve(this, &NrvRaceManRace.AttachJumpPlayerEnd);
}

void RaceManRace::exeAttachJumpPlayerEnd() {
    if (al::isFirstStep(this))
        rs::startActionDemoPlayer(this, "Land");
    if (rs::isActionEndDemoPlayer(this))
        al::setNerve(this, &NrvRaceManRace.ResultTurn);
}

// NON_MATCHING: nerve address codegen (STRUCT nerve: 3 instr vs 2), regswap
void RaceManRace::exeResultTurn() {
    if (al::isFirstStep(this))
        rs::startActionDemoPlayer(this, "TurnPoint");
    sead::Vector3f frontDir;
    al::calcFrontDir(&frontDir, mRaceManGoal);
    sead::Quatf quat = rs::getDemoPlayerQuat(this);
    bool done = al::turnQuatFrontToDirDegreeH(&quat, frontDir, 10.0f);
    rs::setDemoPlayerQuat(this, quat);
    if (done)
        al::setNerve(this, &NrvRaceManRace.Result);
}

void RaceManRace::exeResult() {
    if (al::isFirstStep(this))
        rs::startActionDemoPlayer(this, mIsWin ? "RaceResultWin" : "RaceResultLose");
}

void RaceManRace::exeStartWait() {
    if (al::isFirstStep(this))
        al::startAction(this, "Wait");
}

void RaceManRace::exeRace() {
    if (al::isFirstStep(this)) {
        for (s32 i = 0; i < mGhostPlayers.size(); i++)
            mGhostPlayers.at(i)->start();
    }
}
