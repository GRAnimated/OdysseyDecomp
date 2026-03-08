#include "Npc/RaceManGoal.h"

#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <prim/seadSafeString.h>

#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/CollisionPartsTriangle.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"

#include "MapObj/GoalMark.h"
#include "Npc/GhostPlayer.h"
#include "System/GameDataFunction.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(RaceManGoal, Wait);
NERVE_IMPL(RaceManGoal, End);
NERVES_MAKE_NOSTRUCT(RaceManGoal, Wait, End);
}  // namespace

static const char* sGoalJointNames[] = {"GoalRed", "GoalBlue", "GoalGreen", "GoalPurple"};

RaceManGoal::RaceManGoal(const char* name) : al::LiveActor(name) {}

void RaceManGoal::init(const al::ActorInitInfo& initInfo) {
    al::initActorWithArchiveName(this, initInfo, "RaceGoal", nullptr);
    al::initNerve(this, &Wait, 0);
    al::invalidateClipping(this);

    mGoalMark = new GoalMark("ゴールマーク");
    al::initCreateActorWithPlacementInfo(mGoalMark, initInfo);
    mGoalMark->getName();

    mJointNames.allocBuffer(30, nullptr);
    mHeightOffsets.allocBuffer(mJointNames.capacity(), nullptr);

    al::tryGetArg(&mHeightOffset, initInfo, "HeightOffset");
    al::initJointControllerKeeper(this, mJointNames.capacity());

    for (s32 i = 1; i != 26; ++i) {
        auto* str = new sead::FixedSafeString<128>();
        str->format("Floor%d", i);
        mJointNames.pushBack(str->cstr());
    }

    mJointNames.pushBack("GoalRed");
    mJointNames.pushBack("GoalBlue");
    mJointNames.pushBack("GoalGreen");
    mJointNames.pushBack("GoalPurple");
    mJointNames.pushBack("GoalMario");

    for (s32 i = 0; i < mJointNames.size(); ++i) {
        f32* offset = new f32(0.0f);
        mHeightOffsets.pushBack(offset);
        al::initJointLocalTransControllerY(this, offset, mJointNames.at(i));
    }

    makeActorAlive();
    GameDataFunction::setRaceGoalTrans(this);
}

// NON_MATCHING: stack layout difference for hitPos variable
void RaceManGoal::initAfterPlacement() {
    for (s32 i = 0; i < mHeightOffsets.size(); ++i) {
        sead::Vector3f jointPos;
        al::calcJointPos(&jointPos, this, mJointNames.at(i));

        al::Triangle tri;
        sead::Vector3f from = jointPos + sead::Vector3f::ey * 500.0f;
        sead::Vector3f dir = sead::Vector3f::ey * -1500.0f;
        sead::Vector3f hitPos;
        if (alCollisionUtil::getFirstPolyOnArrow(this, &hitPos, &tri, from, dir, nullptr,
                                                  nullptr))
            *mHeightOffsets.at(i) = (hitPos.y - jointPos.y) + mHeightOffset;
    }
}

void RaceManGoal::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isSensorEye(self)) {
        if (al::isNerve(this, &End))
            al::sendMsgGoalKill(other, self);
    }
}

// NON_MATCHING: Vector3f copy codegen (8+4 vs 4+4+4) and instruction scheduling
bool RaceManGoal::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                             al::HitSensor* self) {
    if (!al::isMsgPlayerDisregard(msg) && !rs::isMsgPlayerDisregardHomingAttack(msg) &&
        !rs::isMsgPlayerDisregardTargetMarker(msg)) {
        if (!al::isNerve(this, &Wait))
            return false;

        if (!rs::isMsgHackAttack(msg) && !rs::isMsgHackAttackMapObj(msg) &&
            !al::isMsgPlayerItemGet(msg))
            return false;

        const sead::Vector3f& otherPos = al::getSensorPos(other);
        f32 otherX = otherPos.x;
        f32 otherZ = otherPos.z;
        const sead::Vector3f& selfPos = al::getSensorPos(self);
        f32 dx = otherX - selfPos.x;
        f32 dz = otherZ - selfPos.z;
        f32 xzDist = sead::Mathf::sqrt(dx * dx + 0.0f + dz * dz);
        f32 radius = al::getSensorRadius(other);
        if (!(xzDist < radius + 250.0f))
            return false;

        al::LiveActor* actor = al::getSensorHost(other);
        for (s32 i = 0; i < mGoalActors.size(); ++i) {
            if (mGoalActors.at(i) == actor)
                return false;
        }

        mGoalActors.pushBack(actor);
        mGoalCount = mGoalActors.size();

        mGoalPos = al::getSensorPos(other);

        const sead::Vector3f& attackPos = al::getSensorPos(other);
        f32 attackX = attackPos.x;
        f32 attackZ = attackPos.z;
        const sead::Vector3f& vel = al::getVelocity(actor);
        f32 prevX = attackX - vel.x;
        f32 prevZ = attackZ - vel.z;

        const sead::Vector3f& myPos = al::getSensorPos(self);
        f32 prevDist = sead::Mathf::sqrt((prevX - myPos.x) * (prevX - myPos.x) + 0.0f +
                                         (prevZ - myPos.z) * (prevZ - myPos.z));

        const sead::Vector3f& currPos = al::getSensorPos(other);
        f32 currX = currPos.x;
        f32 currZ = currPos.z;
        const sead::Vector3f& myPos2 = al::getSensorPos(self);
        f32 currDist = sead::Mathf::sqrt((currX - myPos2.x) * (currX - myPos2.x) + 0.0f +
                                         (currZ - myPos2.z) * (currZ - myPos2.z));

        f32 effectiveRadius = al::getSensorRadius(other) + 250.0f;
        if (!(effectiveRadius < prevDist))
            mGoalProgress = 1.0f;
        else
            mGoalProgress = (effectiveRadius - currDist) / (prevDist - currDist);

        al::setNerve(this, &End);
    }
    return true;
}

bool RaceManGoal::isAttachedActor(const al::LiveActor* actor) const {
    for (s32 i = 0; i < mGoalActors.size(); ++i) {
        if (mGoalActors.at(i) == actor)
            return true;
    }
    return false;
}

void RaceManGoal::exeWait() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Wait");
        mGoalMark->activate();
    }
}

void RaceManGoal::exeEnd() {
    const char* startAnim = mGoalCount == 1 ? "GoalWinStart" : "GoalLoseStart";
    if (al::isFirstStep(this))
        al::startAction(this, startAnim);

    if (al::isActionPlaying(this, startAnim) && al::isActionEnd(this)) {
        const char* loopAnim = mGoalCount == 1 ? "GoalWin" : "GoalLose";
        al::startAction(this, loopAnim);
    }
}

bool RaceManGoal::isGoalPlayer() const {
    return al::isNerve(this, &End);
}

void RaceManGoal::attachActor(GhostPlayer* ghostPlayer) {
    sead::Vector3f pos;
    al::calcJointPos(&pos, this, sGoalJointNames[ghostPlayer->mGoalIndex]);
    ghostPlayer->attachJumpToTarget(pos);
    mGoalActors.pushBack(ghostPlayer);
}

void RaceManGoal::calcMarioJointQuatPos(sead::Quatf* quat, sead::Vector3f* pos) {
    if (pos)
        al::calcJointPos(pos, this, "GoalMario");
    if (quat) {
        sead::Matrix34f* mtx = al::getJointMtxPtr(this, "GoalMario");
        mtx->toQuat(*quat);
    }
}

const char* RaceManGoal::getRaceFirstJointName() {
    return "GoalRed";
}
