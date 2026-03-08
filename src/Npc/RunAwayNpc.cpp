#include "Npc/RunAwayNpc.h"

#include <cmath>
#include <limits>
#include <math/seadVector.h>

#include "Library/Event/EventFlowFunction.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Player/PlayerUtil.h"
#include "Library/Rail/RailUtil.h"
#include "Library/Stage/StageSwitchUtil.h"

#include "Npc/NpcStateTurnSeparate.h"
#include "Npc/TalkNpcCap.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(RunAwayNpc, Patrol);
NERVE_IMPL(RunAwayNpc, Turn);
NERVE_IMPL(RunAwayNpc, Reaction);
NERVE_IMPL(RunAwayNpc, Surprised);
NERVE_IMPL(RunAwayNpc, ScaredToRun);
NERVE_IMPL(RunAwayNpc, Relieved);
NERVE_IMPL(RunAwayNpc, Hiding);
NERVE_IMPL(RunAwayNpc, HidingAndTurning);
NERVE_IMPL(RunAwayNpc, RunAway);
NERVE_IMPL(RunAwayNpc, PrepareToRunAway);
NERVE_IMPL(RunAwayNpc, ScaredByRegularEnemy);
NERVE_IMPL(RunAwayNpc, TurnToPlacementFront);

struct {
    RunAwayNpcNrvPatrol Patrol;
    RunAwayNpcNrvTurn Turn;
    RunAwayNpcNrvReaction Reaction;
    RunAwayNpcNrvSurprised Surprised;
    RunAwayNpcNrvScaredToRun ScaredToRun;
    RunAwayNpcNrvRelieved Relieved;
    RunAwayNpcNrvHiding Hiding;
    RunAwayNpcNrvHidingAndTurning HidingAndTurning;
    RunAwayNpcNrvRunAway RunAway;
    RunAwayNpcNrvPrepareToRunAway PrepareToRunAway;
    RunAwayNpcNrvScaredByRegularEnemy ScaredByRegularEnemy;
    RunAwayNpcNrvTurnToPlacementFront TurnToPlacementFront;
    const char* actionNames[9] = {"Wait",    "Talk",   "Turn",      "Reaction", "ReactionCap",
                                  "Scared",  "Run",    "Search",    "WaitSearch"};
} NrvRunAwayNpc;

const char* sReactionActionNames[] = {"Reaction", "ReactionCap"};

f32 calcAngleToPlayerH(al::LiveActor* actor, const sead::Vector3f* front) {
    sead::Vector3f up = sead::Vector3f::zero;
    sead::Vector3f calcFront = sead::Vector3f::zero;
    al::calcPoseDir(nullptr, &up, &calcFront, actor);
    const sead::Vector3f& playerPos = al::getPlayerPos(actor, 0);
    const sead::Vector3f& trans = al::getTrans(actor);
    sead::Vector3f toPlayer = {playerPos.x - trans.x, playerPos.y - trans.y,
                               playerPos.z - trans.z};
    al::verticalizeVec(&toPlayer, up, toPlayer);
    if (!al::tryNormalizeOrZero(&toPlayer))
        return 0.0f;
    const sead::Vector3f* ref = front ? front : &calcFront;
    return std::abs(al::calcAngleOnPlaneDegree(*ref, toPlayer, up));
}
}  // namespace

// NON_MATCHING: store ordering and combined strh for mIsPlayerClose+_201
RunAwayNpc::RunAwayNpc(const char* name) : al::LiveActor(name) {
    mStoredNerve = &NrvRunAwayNpc.Patrol;
}

void RunAwayNpc::init(const al::ActorInitInfo& info) {
    al::initMapPartsActor(this, info, "Runaway");

    const char* surpriseName =
        al::isActionOneTime(this, "Surprise") ? "Surprise" : "SurpriseOneTime";
    mSurpriseActionName.copy(surpriseName);

    const char* searchName =
        al::isActionOneTime(this, "Search") ? "Search" : "SearchOneTime";
    mSearchActionName.copy(searchName);

    al::initSubActorKeeperNoFile(this, info, 1);
    mCap = TalkNpcCap::tryCreate(this, info);
    if (mCap) {
        al::registerSubActor(this, mCap);
        al::onSyncClippingSubActor(this, mCap);
        al::onSyncAppearSubActor(this, mCap);
        al::onSyncHideSubActor(this, mCap);
        al::onSyncAlphaMaskSubActor(this, mCap);
    }

    auto* talkParam = rs::initTalkNpcParam(this, nullptr);
    al::initJointControllerKeeper(this, 8);
    mLookAtCtrl = rs::tryCreateAndAppendNpcJointLookAtController(this, talkParam, 2000.0f);

    bool isExistHip = al::isExistJoint(this, "Hip");
    mIsHipJoint = isExistHip;
    if (isExistHip)
        mCalcJointDir = al::calcJointSideDir;
    else
        mCalcJointDir = al::calcJointFrontDir;

    s32 defaultAction = 0;
    if (al::tryGetArg(&defaultAction, info, "DefaultAction"))
        mDefaultAction = defaultAction;

    al::initNerve(this, &NrvRunAwayNpc.Patrol, 1);
    auto* turnState = new NpcStateTurnSeparate("ターン", this);
    mTurnState = turnState;
    al::initNerveState(this, turnState, &NrvRunAwayNpc.Turn, "ターン");

    if (mIsHipJoint) {
        if (mDefaultAction == 7) {
            mDefaultAction = 8;
            NrvRunAwayNpc.actionNames[8] = "WaitSearch";
        }
        mReactionMaxStep = 30;
        s32 allRootAmount = al::calcJoitsAmountFromJoint(this, "AllRoot");
        s32 hipAmount = al::calcJoitsAmountFromJoint(this, "Hip");
        s32 maxAmount = (u32)allRootAmount < (u32)hipAmount ? hipAmount : allRootAmount;
        al::initPartialSklAnim(this, 2, 2, maxAmount);
        al::addPartialSklAnimPartsListRecursive(this, "AllRoot", 0);
        al::addPartialSklAnimPartsListRecursive(this, "Hip", 1);
        al::setSensorRadius(this, "Body", 43.0f);
    } else {
        rs::initCapWorldNpcTail(this);
        mReactionMaxStep = 45;
        s32 amount = al::calcJoitsAmountFromJoint(this, "AllRoot");
        al::initPartialSklAnim(this, 1, 1, amount);
        al::addPartialSklAnimPartsListRecursive(this, "AllRoot", 0);
    }

    mEventFlowExecutor = rs::initEventFlowForRunAwayNpc(this, info, nullptr, nullptr);
    rs::initEventCharacterName(mEventFlowExecutor, info, "RunAwayNpc");
    al::initEventReceiver(mEventFlowExecutor, static_cast<al::IEventFlowEventReceiver*>(this));
    rs::initEventCameraTalk(mEventFlowExecutor, info, "Default", -1.0f);
    rs::startEventFlow(mEventFlowExecutor, "Init");

    al::tryGetArg(&mHackType, info, "HackingEventHackType");
    switch (mHackType) {
    case 2:
        mIsPlayerHack = rs::isPlayerHackKuribo;
        break;
    case 24:
        mIsPlayerHack = rs::isPlayerHackKuriboWing;
        break;
    case 31:
        mIsPlayerHack = rs::isPlayerHackSenobi;
        break;
    }

    al::setSensorRadius(this, "Eye", 2000.0f);
    makeActorAlive();
    al::setSyncRailToNearestPos(this);
}

void RunAwayNpc::initAfterPlacement() {
    mPlacementFront = al::getFront(this);
}

void RunAwayNpc::control() {
    if (!al::isNerve(this, &NrvRunAwayNpc.Reaction) &&
        !al::isNerve(this, &NrvRunAwayNpc.Surprised) &&
        !al::isNerve(this, &NrvRunAwayNpc.ScaredToRun) &&
        !al::isNerve(this, &NrvRunAwayNpc.Turn) &&
        !al::isNerve(this, &NrvRunAwayNpc.Relieved)) {
        rs::updateEventFlow(mEventFlowExecutor);
    }

    if (al::isNerve(this, &NrvRunAwayNpc.Hiding) ||
        al::isNerve(this, &NrvRunAwayNpc.HidingAndTurning))
        return;

    const sead::Vector3f& playerPos = al::getPlayerPos(this, 0);
    f32 px = playerPos.x, py = playerPos.y, pz = playerPos.z;
    const sead::Vector3f& trans = al::getTrans(this);
    sead::Vector3f toPlayer = {px - trans.x, py - trans.y, pz - trans.z};
    al::tryNormalizeOrZero(&toPlayer);
    sead::Vector3f normToPlayer = toPlayer;
    const sead::Vector3f& front = al::getFront(this);
    f32 dot = normToPlayer.x * front.x + normToPlayer.y * front.y + normToPlayer.z * front.z;
    f32 dist;
    if (!(dot > 0.0f)) {
        dist = std::numeric_limits<f32>::max();
    } else {
        const sead::Vector3f& pp = al::getPlayerPos(this, 0);
        const sead::Vector3f& t = al::getTrans(this);
        f32 dx = pp.x - t.x;
        f32 dy = pp.y - t.y;
        f32 dz = pp.z - t.z;
        dist = std::sqrt(dx * dx + dy * dy + dz * dz) * 0.01f;
    }
    bool isScaredByPlayer = reinterpret_cast<u8*>(&mScareType)[1];
    mNormalizedDistToPlayer = dist;
    if (!isScaredByPlayer) {
        const sead::Vector3f& t = al::getTrans(this);
        f32 dx = mEnemySensorPos.x - t.x;
        f32 dy = mEnemySensorPos.y - t.y;
        f32 dz = mEnemySensorPos.z - t.z;
        if (!(std::sqrt(dx * dx + dy * dy + dz * dz) > 2000.0f) &&
            !(calcAngleToPlayerH(this, &mPlacementFront) > 110.0f))
            goto skipClear;
    }
    reinterpret_cast<u8*>(&mScareType)[0] = 0;
skipClear:
    mEnemySensorPos = {std::numeric_limits<f32>::max(), std::numeric_limits<f32>::max(),
                       std::numeric_limits<f32>::max()};
}

// NON_MATCHING: regalloc and instruction scheduling in inlined canSeeWithCylindricalLineOfSight
void RunAwayNpc::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isSensorNpc(self)) {
        if (!rs::sendMsgPushToPlayer(other, self))
            al::sendMsgPush(other, self);
        return;
    }

    if (mIsRunning)
        return;

    if (!al::isSensorEye(self))
        return;

    if (mScareUneffectiveWindow != 0) {
        mScareType = 0;
        mScareUneffectiveWindow--;
        return;
    }

    if (al::isSensorEnemy(other)) {
        if (rs::sendMsgIgnoredByRunawayNpc(other, self))
            return;

        if (!al::isNerve(this, &NrvRunAwayNpc.Hiding) &&
            !al::isNerve(this, &NrvRunAwayNpc.HidingAndTurning) &&
            !reinterpret_cast<u8*>(&mScareType)[1] &&
            rs::sendMsgEventFlowScareCheck(other, self, mEventFlowExecutor)) {
            if (mIsPlayerHack(this)) {
                const sead::Vector3f& playerPos = al::getPlayerPos(this, 0);
                f32 px = playerPos.x;
                f32 py = playerPos.y;
                f32 pz = playerPos.z;
                const sead::Vector3f& trans = al::getTrans(this);
                f32 dx = px - trans.x;
                f32 dy = py - trans.y;
                f32 dz = pz - trans.z;
                if (std::sqrt(dx * dx + dy * dy + dz * dz) < 1000.0f) {
                    mScareType = 256;
                    return;
                }
            }
            reinterpret_cast<u8*>(&mScareType)[0] = 1;
            return;
        }
        mEnemySensorPos = al::getSensorPos(other);

        bool canSee = mIsPlayerHack(this);
        if (canSeeWithCylindricalLineOfSight(mEnemySensorPos)) {
            reinterpret_cast<u8*>(&mScareType)[1] = canSee;
        }

        if (!reinterpret_cast<u8*>(&mScareType)[1]) {
            if (canSeeWithCylindricalLineOfSight(mEnemySensorPos))
                reinterpret_cast<u8*>(&mScareType)[0] = 1;
        }
    }
}

bool RunAwayNpc::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                            al::HitSensor* self) {
    if (rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;

    if (rs::checkMsgNpcTrampleReactionAll(msg, other, self, false)) {
        rs::requestHitReactionToAttacker(msg, self, other);
        mReactionType = 0;
        goto setReaction;
    }

    if (rs::isMsgNpcCapReactionAll(msg) || rs::isMsgSeedAttack(msg) ||
        rs::isMsgSeedAttackBig(msg)) {
        if (al::isNerve(this, &NrvRunAwayNpc.Reaction))
            return false;
        rs::requestHitReactionToAttacker(msg, self, other);
        mReactionType = 1;
        goto setReaction;
    }

    return false;

setReaction:
    if (al::isNerve(this, &NrvRunAwayNpc.Patrol) ||
        al::isNerve(this, &NrvRunAwayNpc.Hiding) ||
        al::isNerve(this, &NrvRunAwayNpc.HidingAndTurning) ||
        al::isNerve(this, &NrvRunAwayNpc.RunAway) ||
        al::isNerve(this, &NrvRunAwayNpc.PrepareToRunAway)) {
        mStoredNerve = al::getCurrentNerve(this);
    }
    al::setNerve(this, &NrvRunAwayNpc.Reaction);
    return true;
}

bool RunAwayNpc::receiveEvent(const al::EventFlowEventData*) {
    return false;
}

void RunAwayNpc::exePatrol() {
    if (reinterpret_cast<u8*>(&mScareType)[0]) {
        al::setNerve(this, &NrvRunAwayNpc.ScaredByRegularEnemy);
        return;
    }

    if (!al::isActionPlaying(this, "Talk") && !al::isActionPlaying(this, "Turn"))
        al::tryStartActionIfNotPlaying(this, getCurrentWaitActionName());

    if (reinterpret_cast<u8*>(&mScareType)[1]) {
        al::setNerve(this, &NrvRunAwayNpc.Surprised);
        return;
    }

    if (mNormalizedDistToPlayer <= 20.0f) {
        reactToPlayer();
        return;
    }

    mIsPlayerClose = false;
    sead::Vector3f headDir;
    mCalcJointDir(&headDir, this, "Head");
    f32 dot = headDir.x * mPlacementFront.x + headDir.y * mPlacementFront.y +
              headDir.z * mPlacementFront.z;
    if (dot >= 0.45f)
        reactToPlayer();
    else
        al::setNerve(this, &NrvRunAwayNpc.TurnToPlacementFront);
}

const char* RunAwayNpc::getCurrentWaitActionName() {
    if (mNormalizedDistToPlayer < 7.5f) {
        mIsPlayerClose = true;
        return "Wait";
    }
    if (mIsPlayerClose)
        return "Wait";
    return NrvRunAwayNpc.actionNames[(u32)mDefaultAction];
}

void RunAwayNpc::reactToPlayer() {
    rs::updateNpcJointLookAtController(mLookAtCtrl);
    if (mIsPlayerClose) {
        const sead::Vector3f& playerPos = al::getPlayerPos(this, 0);
        const sead::Vector3f& trans = al::getTrans(this);
        f32 dx = playerPos.x - trans.x;
        f32 dy = playerPos.y - trans.y;
        f32 dz = playerPos.z - trans.z;
        if (std::sqrt(dx * dx + dy * dy + dz * dz) <= 2000.0f &&
            calcAngleToPlayerH(this, &mPlacementFront) <= 110.0f &&
            calcAngleToPlayerH(this, nullptr) >= 15.0f) {
            mTurnState->startTurnToTarget(al::getPlayerPos(this, 0));
            if (mTurnState->tryStartTurnToTarget(al::getPlayerPos(this, 0), 80.0f)) {
                al::setNerve(this, &NrvRunAwayNpc.Turn);
                return;
            }
        }
    }

    const char* actionName = getCurrentWaitActionName();
    if (!al::isActionPlaying(this, actionName) && !al::isActionPlaying(this, "Talk")) {
        if (mWaitActionTimer == 0) {
            al::tryStartActionIfNotPlaying(this, getCurrentWaitActionName());
            mWaitActionTimer = 40;
        }
        mWaitActionTimer--;
    }
}

void RunAwayNpc::exeReaction() {
    if (al::isFirstStep(this))
        al::startAction(this, sReactionActionNames[mReactionType]);

    if (mIsRunning) {
        if (al::isStep(this, mReactionMaxStep))
            al::setNerve(this, &NrvRunAwayNpc.Hiding);
    } else {
        if (!al::isActionEnd(this))
            return;
        const al::Nerve* nerve = mStoredNerve;
        if (nerve == &NrvRunAwayNpc.Patrol) {
            mTurnState->startTurnToTarget(al::getPlayerPos(this, 0));
            mTurnState->tryStartTurnToTarget(al::getPlayerPos(this, 0), 80.0f);
            nerve = &NrvRunAwayNpc.Turn;
            mStoredNerve = &NrvRunAwayNpc.Turn;
        }
        al::setNerve(this, nerve);
    }
}

void RunAwayNpc::exeSurprised() {
    rs::updateNpcJointLookAtController(mLookAtCtrl);
    if (al::isFirstStep(this)) {
        mSurpriseActionName.getStringTop();
        al::startAction(this, mSurpriseActionName.cstr());
    }
    if (al::isActionEnd(this))
        al::setNerve(this, &NrvRunAwayNpc.ScaredToRun);
}

void RunAwayNpc::exeScaredToRun() {
    if (al::isFirstStep(this))
        al::startAction(this, "Scared");

    if ((u8)mScareType) {
        al::setNerve(this, &NrvRunAwayNpc.ScaredByRegularEnemy);
        return;
    }

    if ((u16)mScareType <= 0xFF) {
        al::setNerve(this, &NrvRunAwayNpc.Relieved);
        return;
    }

    if (mNormalizedDistToPlayer > 20.0f) {
        al::setNerve(this, &NrvRunAwayNpc.Relieved);
        reinterpret_cast<u8*>(&mScareType)[1] = 0;
        return;
    }

    if (mNormalizedDistToPlayer < 7.0f)
        al::setNerve(this, &NrvRunAwayNpc.PrepareToRunAway);
}

void RunAwayNpc::exeScaredByRegularEnemy() {
    if (al::isFirstStep(this))
        al::startAction(this, "Scared");

    if (!(u8)mScareType) {
        al::setNerve(this, &NrvRunAwayNpc.Patrol);
        return;
    }

    if ((u16)mScareType < 0x100)
        return;
    al::setNerve(this, &NrvRunAwayNpc.Surprised);
}

void RunAwayNpc::exeRelieved() {
    if (al::isFirstStep(this)) {
        mSearchActionName.getStringTop();
        al::startAction(this, mSearchActionName.cstr());
        mScareUneffectiveWindow = 60;
    }

    if (reinterpret_cast<u8*>(&mScareType)[1] && al::isGreaterEqualStep(this, 20))
        al::setNerve(this, &NrvRunAwayNpc.Surprised);
    else if (al::isActionEnd(this))
        al::setNerve(this, &NrvRunAwayNpc.TurnToPlacementFront);
}

void RunAwayNpc::resetScareUneffectiveWindow() {
    mScareUneffectiveWindow = 60;
}

void RunAwayNpc::exePrepareToRunAway() {
    if (al::isFirstStep(this)) {
        rs::invalidateNpcJointLookAtController(mLookAtCtrl);
        al::invalidateClipping(this);
        const sead::Vector3f& front = al::getFront(this);
        sead::Vector3f cross;
        cross.x = front.y * 0.0f - front.z * 0.0f;
        cross.y = front.z * 0.0f - front.x * 0.0f;
        cross.z = front.x - front.y * 0.0f;
        // cross = cross product of front and (0,1,0) → perpendicular, but simplified
        // Actually this is just: cross = {-fz*0, fz*0 - fx*0, fx - fy*0} which is {0,0,fx}
        // The pattern is cross product with up=(0,0,0)? No, it's (0,1,0) * 0? Let me
        // re-examine...
        al::tryNormalizeOrZero(&cross);

        const sead::Vector3f& playerPos = al::getPlayerPos(this, 0);
        const sead::Vector3f& trans = al::getTrans(this);
        sead::Vector3f toPlayer = {playerPos.x - trans.x, playerPos.y - trans.y,
                                   playerPos.z - trans.z};
        al::tryNormalizeOrZero(&toPlayer);
        if (toPlayer.x * cross.x + toPlayer.y * cross.y + toPlayer.z * cross.z < 0.0f) {
            al::reverseRail(this);
            mRunDirection = -1.0f;
        }
    }

    f32 speed = mRunDirection;
    const sead::Vector3f& railDir = al::getRailDir(this);
    sead::Vector3f dir = {speed * railDir.x, speed * railDir.y, speed * railDir.z};
    if (al::turnToDirection(this, dir, 1000.0f))
        al::setNerve(this, &NrvRunAwayNpc.RunAway);
}

void RunAwayNpc::exeRunAway() {
    if (al::isFirstStep(this)) {
        if (mIsHipJoint && al::isPartialSklAnimPlaying(this, "Turn", 1))
            al::clearPartialSklAnim(this, 1);
        al::startAction(this, "Run");
        al::tryOnStageSwitch(this, "SwitchRunAwayOn");
        rs::startEventFlow(mEventFlowExecutor, "Hiding");
    }

    f32 speed = mRunDirection;
    const sead::Vector3f& railDir = al::getRailDir(this);
    sead::Vector3f dir = {speed * railDir.x, speed * railDir.y, speed * railDir.z};
    al::setFront(this, dir);
    al::moveSyncRail(this, 10.0f);
    if (al::isRailReachedEnd(this) || al::isRailReachedStart(this))
        al::setNerve(this, &NrvRunAwayNpc.Hiding);
}

void RunAwayNpc::exeHiding() {
    if (al::isFirstStep(this)) {
        mIsRunning = true;
        al::validateClipping(this);
        if (mIsHipJoint) {
            if (_201) {
                sead::Vector3f offset = al::getSensorFollowPosOffset(this, "Body");
                offset.z += 32.0f;
                offset.y += 30.0f;
                al::setSensorFollowPosOffset(this, "Body", offset);
                _201 = false;
            }
            if (al::isPartialSklAnimPlaying(this, "Turn", 1))
                al::clearPartialSklAnim(this, 1);
        }
        al::startAction(this, "Scared");
    }

    if (mIsPlayerHack(this)) {
        sead::Vector3f dir;
        al::calcDirToActorH(&dir, this, al::getPlayerPos(this, 0));
        const sead::Vector3f& front = al::getFront(this);
        f32 dot = dir.x * front.x + dir.y * front.y + dir.z * front.z;
        if (dot > std::cos(0.2618f))
            al::setNerve(this, &NrvRunAwayNpc.HidingAndTurning);
    }
}

void RunAwayNpc::exeHidingAndTurning() {
    if (al::isFirstStep(this) && mIsHipJoint)
        al::startPartialSklAnim(this, "Turn", 1, 1, 0);

    const sead::Vector3f& trans = al::getTrans(this);
    const sead::Vector3f& playerPos = al::getPlayerPos(this, 0);
    sead::Vector3f awayDir = {trans.x - playerPos.x, trans.y - playerPos.y,
                              trans.z - playerPos.z};
    if (al::turnFrontToDirGetIsFinished(this, awayDir, 5.0f))
        al::setNerve(this, &NrvRunAwayNpc.Hiding);
}

void RunAwayNpc::exeTurn() {
    rs::updateNpcJointLookAtController(mLookAtCtrl);
    if (!al::updateNerveState(this))
        goto checkFlags;

    if (!mIsTurnedToPlayer)
        goto setPatrol;

    mTurnState->startTurnToTarget(al::getPlayerPos(this, 0));
    if (!mTurnState->tryStartTurnToTarget(al::getPlayerPos(this, 0), 80.0f) &&
        calcAngleToPlayerH(this, nullptr) < 10.0f) {
        al::setNerve(this, &NrvRunAwayNpc.Patrol);
        return;
    }

    if (mIsTurnedToPlayer && !(calcAngleToPlayerH(this, nullptr) < 10.0f))
        goto checkFlags;

setPatrol:
    al::setNerve(this, &NrvRunAwayNpc.Patrol);
    mIsTurnedToPlayer = true;
    return;

checkFlags:
    if (reinterpret_cast<u8*>(&mScareType)[1]) {
        al::setNerve(this, &NrvRunAwayNpc.Surprised);
        return;
    }
    if (reinterpret_cast<u8*>(&mScareType)[0])
        al::setNerve(this, &NrvRunAwayNpc.ScaredByRegularEnemy);
}

void RunAwayNpc::exeTurnToPlacementFront() {
    if (al::isFirstStep(this)) {
        if (!mIsHipJoint) {
            al::setNerve(this, &NrvRunAwayNpc.Patrol);
            return;
        }
        al::startAction(this, "Turn");
    }
    if (!mTurnState->tryStartTurnToDir(mPlacementFront, 25.0f)) {
        al::setNerve(this, &NrvRunAwayNpc.Patrol);
        return;
    }
    mIsTurnedToPlayer = false;
    al::setNerve(this, &NrvRunAwayNpc.Turn);
}

bool RunAwayNpc::canSeeWithCylindricalLineOfSight(const sead::Vector3f& pos) {
    sead::Vector3f headDir;
    mCalcJointDir(&headDir, this, "Head");
    sead::Vector3f headPos;
    al::calcJointPos(&headPos, this, "Head");

    sead::Vector3f toPos = {pos.x - headPos.x, pos.y - headPos.y, pos.z - headPos.z};
    sead::Vector3f lineEnd = {headDir.x * 2000.0f + headPos.x, headDir.y * 2000.0f + headPos.y,
                              headDir.z * 2000.0f + headPos.z};
    sead::Vector3f lineDir = {lineEnd.x - headPos.x, lineEnd.y - headPos.y,
                              lineEnd.z - headPos.z};
    f32 t = toPos.x * lineDir.x + toPos.y * lineDir.y + toPos.z * lineDir.z;
    if (t < 0.0f || t > 4000000.0f)
        return false;
    f32 distSq = toPos.x * toPos.x + toPos.y * toPos.y + toPos.z * toPos.z;
    return distSq + t * t / -4000000.0f <= 490000.0f;
}
