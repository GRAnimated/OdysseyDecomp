#include "Npc/ShibakenFunction.h"

#include <cmath>
#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Base/StringUtil.h"
#include "Library/Collision/CollisionParts.h"
#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/CollisionPartsTriangle.h"
#include "Library/Collision/KCollisionServer.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementInfo.h"

#include "MapObj/DigPoint.h"
#include "Npc/ActorStateReactionBase.h"
#include "Npc/NpcStateReactionParam.h"
#include "Npc/Shibaken.h"
#include "Npc/ShibakenStateBark.h"
#include "Npc/ShibakenStateCapCatch.h"
#include "Npc/ShibakenStateJump.h"
#include "Npc/ShibakenStatePointChase.h"
#include "Npc/ShibakenStateWait.h"
#include "Npc/ShibakenStateWaitFar.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(Shibaken, FindTurn);
NERVE_IMPL(Shibaken, Find);
NERVE_IMPL(Shibaken, Sleep);
NERVE_IMPL(Shibaken, SleepEnd);
NERVE_IMPL(Shibaken, WaitInit);
NERVE_IMPL(Shibaken, Wait);
NERVE_IMPL(Shibaken, WaitFar);
NERVE_IMPL(Shibaken, Reaction);
NERVE_IMPL(Shibaken, PointChase);
NERVE_IMPL(Shibaken, CapCatch);
NERVE_IMPL(Shibaken, Bark);
NERVE_IMPL(Shibaken, Jump);
NERVE_IMPL(Shibaken, Sit);
NERVE_IMPL(Shibaken, Reset);
NERVE_IMPL(Shibaken, PlayerChase);
NERVE_IMPL(Shibaken, SleepStart);
NERVE_IMPL(Shibaken, PlayerChaseTurn);

NERVES_MAKE_NOSTRUCT(Shibaken, FindTurn, Find, Sleep, SleepEnd);
NERVES_MAKE_STRUCT(Shibaken, WaitInit, Wait, WaitFar, Reaction, PointChase, CapCatch, Bark, Jump,
                   Sit, Reset, PlayerChase, SleepStart, PlayerChaseTurn);
}  // namespace

class IsNotMoveLimit : public al::CollisionPartsFilterBase {
    bool isInvalidParts(al::CollisionParts* parts) override {
        if (parts->getSpecialPurpose() &&
            !al::isEqualString("MoveLimit", parts->getSpecialPurpose()))
            return !al::isEqualString("ShibakenMoveLimit", parts->getSpecialPurpose());
        return false;
    }
};

IsNotMoveLimit sIsNotMoveLimit;

NpcStateReactionParam sShibakenReactionParam("Reaction", "ReactionBall");

void initShibakenDigPointLocater(ShibakenDigPointLocater* locater,
                                 const al::ActorInitInfo& info,
                                 const al::PlacementInfo& placementInfo) {
    locater->childCount = 0;
    locater->point = nullptr;
    locater->children = nullptr;
    locater->isValid = true;

    auto* digPoint = new DigPoint(u8"忠犬専用ここ掘れポイント");
    locater->point = digPoint;
    al::initCreateActorWithPlacementInfo(digPoint, info, placementInfo);
    locater->point->makeActorDead();

    s32 childNum = al::calcLinkChildNum(placementInfo, "NextDigPoint");
    locater->childCount = childNum;
    if (childNum >= 1) {
        locater->children = new ShibakenDigPointLocater*[childNum];
        for (s32 i = 0; i < locater->childCount; i++) {
            al::PlacementInfo childInfo;
            al::getLinksInfoByIndex(&childInfo, placementInfo, "NextDigPoint", i);
            auto* child =
                static_cast<ShibakenDigPointLocater*>(::operator new(0x18));
            initShibakenDigPointLocater(child, info, childInfo);
            locater->children[i] = child;
        }
    }
}

void updateShibakenDigPointLocaterHintTrans(ShibakenDigPointLocater* locater,
                                            const sead::Vector3f& trans) {
    locater->point->tryUpdateHintTrans(trans);
    for (s32 i = 0; i < locater->childCount; i++)
        updateShibakenDigPointLocaterHintTrans(locater->children[i], trans);
}

namespace {

sead::Vector3f sFitPoseForwardOffset = {0.0f, 0.0f, 30.0f};
sead::Vector3f sFitPoseBackwardOffset = {0.0f, 0.0f, -30.0f};

// NON_MATCHING: complex ground normal sampling with register allocation differences
__attribute__((noinline)) void fitPoseOnGround(Shibaken* shibaken) {
    if (!al::isExistActorCollider(shibaken))
        return;
    if (!al::isOnGround(shibaken, 0))
        return;

    sead::Vector3f accumNormal = {0.0f, 0.0f, 0.0f};
    s32 count = 0;

    if (al::isCollidedGround(shibaken)) {
        const sead::Vector3f* groundNormal = &al::getCollidedGroundNormal(shibaken);
        GameDataHolderAccessor accessor(shibaken);
        bool isWorldMoon = GameDataFunction::isWorldMoon(accessor);
        const sead::Vector3f& grav = al::getGravity(shibaken);
        f32 dot = grav.x * groundNormal->x + grav.y * groundNormal->y +
                  grav.z * groundNormal->z;
        bool isValid = isWorldMoon ? dot < -0.34202f : dot < -0.5f;
        if (isValid) {
            accumNormal += al::getCollidedGroundNormal(shibaken);
            count = 1;
        } else {
            count = 0;
        }
    }

    sead::Vector3f upDir = {0.0f, 0.0f, 0.0f};
    al::calcUpDir(&upDir, shibaken);

    al::Triangle triangle1;
    sead::Vector3f pos1 = sFitPoseForwardOffset;
    al::multVecPose(&pos1, shibaken, pos1);
    const sead::Vector3f& vel = al::getVelocity(shibaken);
    pos1 += vel;
    sead::Vector3f end1 = {upDir.x * 50.0f + pos1.x, upDir.y * 50.0f + pos1.y,
                           upDir.z * 50.0f + pos1.z};
    sead::Vector3f dir1 = {upDir.x * -2.0f * 50.0f, upDir.y * -2.0f * 50.0f,
                           upDir.z * -2.0f * 50.0f};
    if (alCollisionUtil::getFirstPolyOnArrow((al::IUseCollision*)shibaken, nullptr, &triangle1,
                                             end1, dir1, nullptr, nullptr)) {
        const sead::Vector3f& normal1 = triangle1.getNormal(0);
        GameDataHolderAccessor accessor(shibaken);
        bool isWorldMoon = GameDataFunction::isWorldMoon(accessor);
        const sead::Vector3f& grav = al::getGravity(shibaken);
        f32 dot = grav.x * normal1.x + grav.y * normal1.y + grav.z * normal1.z;
        bool isValid = isWorldMoon ? dot < -0.34202f : dot < -0.5f;
        if (isValid) {
            accumNormal += triangle1.getNormal(0);
            count++;
        }
    }

    al::Triangle triangle2;
    sead::Vector3f pos2 = sFitPoseBackwardOffset;
    al::multVecPose(&pos2, shibaken, pos2);
    const sead::Vector3f& vel2 = al::getVelocity(shibaken);
    pos2 += vel2;
    sead::Vector3f end2 = {upDir.x * 50.0f + pos2.x, upDir.y * 50.0f + pos2.y,
                           upDir.z * 50.0f + pos2.z};
    sead::Vector3f dir2 = {upDir.x * -2.0f * 50.0f, upDir.y * -2.0f * 50.0f,
                           upDir.z * -2.0f * 50.0f};
    if (alCollisionUtil::getFirstPolyOnArrow((al::IUseCollision*)shibaken, nullptr, &triangle2,
                                             end2, dir2, nullptr, nullptr)) {
        const sead::Vector3f& normal2 = triangle2.getNormal(0);
        GameDataHolderAccessor accessor(shibaken);
        bool isWorldMoon = GameDataFunction::isWorldMoon(accessor);
        const sead::Vector3f& grav = al::getGravity(shibaken);
        f32 dot = grav.x * normal2.x + grav.y * normal2.y + grav.z * normal2.z;
        bool isValid = isWorldMoon ? dot < -0.34202f : dot < -0.5f;
        if (isValid) {
            accumNormal += triangle2.getNormal(0);
            count++;
        }
    } else {
        if (count < 1)
            return;
    }

    if (al::isNearZero(accumNormal, 0.001f))
        return;

    f32 invCount = 1.0f / (f32)count;
    accumNormal.x *= invCount;
    accumNormal.y *= invCount;
    accumNormal.z *= invCount;
    if (al::tryNormalizeOrZero(&accumNormal)) {
        sead::Quatf* quat = al::getQuatPtr(shibaken);
        al::turnQuatYDirRate(quat, *quat, accumNormal, 0.2f);
    }
}

f32 calcPlayerDistanceSpeed(const Shibaken* shibaken) {
    f32 dist = al::calcDistanceH(shibaken, rs::getPlayerPos(shibaken));
    if (dist < 100.0f)
        return 0.0f;
    if (dist < 200.0f)
        return al::lerpValue(0.0f, 0.5f, al::normalize(dist, 100.0f, 200.0f));
    if (dist < 450.0f)
        return al::lerpValue(0.5f, 3.25f, al::normalize(dist, 200.0f, 450.0f));
    return 3.25f;
}
}  // namespace

bool ShibakenFunction::tryStartReaction(al::IUseNerve* user, ActorStateReactionBase* reaction,
                                        const al::Nerve* nerve, const al::SensorMsg* msg,
                                        al::HitSensor* other, al::HitSensor* self) {
    if (!al::isSensorEnemyBody(self))
        return false;
    if (al::isNerve(user, nerve) && al::isNewNerve(user))
        return false;
    if (rs::isMsgFireDamageAll(msg))
        return false;
    if (rs::isMsgSphinxRideAttackReflect(msg))
        return false;
    if (!reaction->receiveMsg(msg, other, self))
        return false;
    al::setNerve(user, nerve);
    return true;
}

void Shibaken::exeSit() {
    al::updateNerveState(this);
}

void Shibaken::exeWaitInit() {
    al::updateNerveState(this);
    if (rs::isNearPlayerH(this, 1500.0f))
        al::setNerve(this, &FindTurn);
}

void Shibaken::exeWait() {
    if (al::isFirstStep(this)) {
        sead::Vector3f gravity;
        if (al::isExistActorCollider(this) && al::isCollidedGround(this)) {
            gravity = -al::getCollidedGroundNormal(this);
        } else {
            gravity.set(al::getGravity(this));
        }
        al::scaleVelocityExceptDirection(this, gravity, 0.0f);
    }
    al::updateNerveState(this);
    if (mStateWait->isPlayingWait() && calcPlayerDistanceSpeed(this) > 0.5f) {
        al::setNerve(this, &NrvShibaken.PlayerChase);
    } else if (rs::isPlayerWaitSleep(this)) {
        al::setNerve(this, &NrvShibaken.SleepStart);
    } else if (mStateCapCatch->tryStart()) {
        al::setNerve(this, &NrvShibaken.CapCatch);
    } else {
        ShibakenFunction::tryStartJump(this, this, &NrvShibaken.Jump);
    }
}

void Shibaken::exeWaitFar() {
    al::updateNerveStateAndNextNerve(this, &NrvShibaken.PlayerChase);
    if (mStateCapCatch->tryStart())
        al::setNerve(this, &NrvShibaken.CapCatch);
}

void Shibaken::exeFindTurn() {
    if (al::isFirstStep(this))
        al::startAction(this, "Move");
    if (al::turnQuatFrontToPosDegreeH(this, rs::getPlayerPos(this), 4.0f))
        al::setNerve(this, &Find);
}

void Shibaken::exeFind() {
    if (al::isFirstStep(this))
        al::startAction(this, "Find");
    if (ShibakenFunction::tryStartJump(this, this, &NrvShibaken.Jump))
        return;
    al::setNerveAtActionEnd(this, &NrvShibaken.PlayerChase);
}

// NON_MATCHING: complex dig point holder logic
void Shibaken::exePlayerChase() {
    if (al::isFirstStep(this))
        al::tryStartActionIfNotPlaying(this, "Move");

    if (mStateBark->tryStart()) {
        al::setNerve(this, &NrvShibaken.Bark);
        return;
    }
    if (mStateCapCatch->tryStart()) {
        al::setNerve(this, &NrvShibaken.CapCatch);
        return;
    }
    if (ShibakenFunction::tryStartJump(this, this, &NrvShibaken.Jump))
        return;
    if (ShibakenFunction::chaseToPlayerAndTryStop(this)) {
        if (mStateWaitFar->tryStart())
            al::setNerve(this, &NrvShibaken.WaitFar);
        else
            al::setNerve(this, &NrvShibaken.Wait);
        return;
    }
    if (al::isCollidedWallVelocity(this)) {
        sead::Vector3f toPlayer = rs::getPlayerPos(this) - al::getTrans(this);
        al::verticalizeVec(&toPlayer, al::getGravity(this), toPlayer);
        if (!al::isNearZero(toPlayer, 0.001f)) {
            sead::Vector3f front = {0.0f, 0.0f, 0.0f};
            al::calcFrontDir(&front, this);
            if (toPlayer.x * front.x + toPlayer.y * front.y + toPlayer.z * front.z < 0.0f)
                al::setNerve(this, &NrvShibaken.PlayerChaseTurn);
        }
    }
}

void Shibaken::exePlayerChaseTurn() {
    if (al::isFirstStep(this))
        al::tryStartActionIfNotPlaying(this, "Move");
    if (al::turnQuatFrontToPosDegreeH(this, rs::getPlayerPos(this), 4.0f))
        al::setNerve(this, &NrvShibaken.PlayerChase);
}

void Shibaken::exePointChase() {
    if (!al::updateNerveState(this))
        return;
    if (mStatePointChase->isKillByDeathArea()) {
        al::hideModelIfShow(this);
        al::setVelocityZero(this);
        al::invalidateHitSensors(this);
        al::setNerve(this, &NrvShibaken.Reset);
    } else {
        al::setNerve(this, &NrvShibaken.Wait);
    }
}

void Shibaken::exeCapCatch() {
    if (!al::updateNerveState(this))
        return;
    if (al::isActionPlaying(this, "Move"))
        al::setNerve(this, &NrvShibaken.PlayerChase);
    else
        al::setNerve(this, &NrvShibaken.Wait);
}

void Shibaken::exeBark() {
    if (ShibakenFunction::tryStartJump(this, this, &NrvShibaken.Jump))
        return;
    al::updateNerveStateAndNextNerve(this, &NrvShibaken.Wait);
}

void Shibaken::exeSleepStart() {
    if (al::isFirstStep(this))
        al::startAction(this, "SleepStart");
    sead::Vector3f gravity;
    if (al::isCollidedGround(this)) {
        gravity = -al::getCollidedGroundNormal(this);
    } else {
        gravity.set(al::getGravity(this));
    }
    f32 accel = 1.0f;
    if (mWorldMoonFlag == 1)
        accel = 0.6f;
    al::addVelocityToDirection(this, gravity, accel);
    al::scaleVelocityHV(this, 0.8f, 0.8f);
    al::setNerveAtActionEnd(this, &Sleep);
}

void Shibaken::exeSleep() {
    if (al::isFirstStep(this))
        al::startAction(this, "Sleep");
    sead::Vector3f gravity;
    if (al::isCollidedGround(this)) {
        gravity = -al::getCollidedGroundNormal(this);
    } else {
        gravity.set(al::getGravity(this));
    }
    f32 accel = 1.0f;
    if (mWorldMoonFlag == 1)
        accel = 0.6f;
    al::addVelocityToDirection(this, gravity, accel);
    al::scaleVelocityHV(this, 0.8f, 0.8f);
    if (!rs::isPlayerWaitSleep(this))
        al::setNerve(this, &SleepEnd);
}

void Shibaken::exeSleepEnd() {
    if (al::isFirstStep(this))
        al::startAction(this, "SleepEnd");
    sead::Vector3f gravity;
    if (al::isCollidedGround(this)) {
        gravity = -al::getCollidedGroundNormal(this);
    } else {
        gravity.set(al::getGravity(this));
    }
    f32 accel = 1.0f;
    if (mWorldMoonFlag == 1)
        accel = 0.6f;
    al::addVelocityToDirection(this, gravity, accel);
    al::scaleVelocityHV(this, 0.8f, 0.8f);
    al::setNerveAtActionEnd(this, &NrvShibaken.Wait);
}

void Shibaken::exeJump() {
    if (!al::updateNerveState(this))
        return;
    if (mStateJump->isKillByDeathArea()) {
        al::hideModelIfShow(this);
        al::setVelocityZero(this);
        al::invalidateHitSensors(this);
        al::setNerve(this, &NrvShibaken.Reset);
    } else {
        al::setNerve(this, &NrvShibaken.Wait);
    }
}

void Shibaken::exeReaction() {
    al::updateNerveStateAndNextNerve(this, &NrvShibaken.Wait);
}

void Shibaken::exeReset() {
    if (!al::isJudgedToClipFrustum(this, mInitTrans, 500.0f, 300.0f))
        return;
    al::showModelIfHide(this);
    al::validateHitSensors(this);
    al::resetQuatPosition(this, mInitQuat, mInitTrans);
    al::setNerve(this, &NrvShibaken.Wait);
}

bool ShibakenFunction::isPlayingMoveAction(const Shibaken* shibaken) {
    return al::isActionPlaying(shibaken, "Move");
}

f32 ShibakenFunction::getJumpGravityAccel(const Shibaken* shibaken) {
    f32 result = 1.5f;
    if (shibaken->mWorldMoonFlag == 1)
        result = 0.75f;
    return result;
}

f32 ShibakenFunction::getJumpAirFriction(const Shibaken* shibaken) {
    f32 result = 1.0f;
    if (shibaken->mWorldMoonFlag == 1)
        result = 0.925f;
    return result;
}

f32 ShibakenFunction::getJumpStartSpeedV(const Shibaken* shibaken) {
    f32 result = 15.0f;
    if (shibaken->mWorldMoonFlag == 1)
        result = 25.0f;
    return result;
}

// NON_MATCHING: b.pl vs b.ge condition and length() scheduling
bool ShibakenFunction::chaseToPlayerAndTryStop(Shibaken* shibaken) {
    f32 speed = calcPlayerDistanceSpeed(shibaken);
    const sead::Vector3f& playerPos = rs::getPlayerPos(shibaken);
    chaseToTarget(shibaken, playerPos, speed, false, false);
    if (al::isCollidedWallVelocity(shibaken)) {
        const sead::Vector3f& wallNormal = al::getCollidedWallNormal(shibaken);
        if (checkStopChaseByFaceWall(shibaken, wallNormal))
            return true;
    }
    if (speed >= 0.5f)
        return false;
    sead::Vector3f hVel = {0.0f, 0.0f, 0.0f};
    sead::Vector3f up = {0.0f, 0.0f, 0.0f};
    al::calcUpDir(&up, shibaken);
    al::verticalizeVec(&hVel, up, al::getVelocity(shibaken));
    return sead::Vector3f(hVel).length() < 1.25f;
}

void ShibakenFunction::addFallVelocityToGround(Shibaken* shibaken, f32 hScale) {
    sead::Vector3f gravity;
    if (al::isCollidedGround(shibaken)) {
        gravity = -al::getCollidedGroundNormal(shibaken);
    } else {
        gravity.set(al::getGravity(shibaken));
    }
    f32 accel = 1.0f;
    if (shibaken->mWorldMoonFlag == 1)
        accel = 0.6f;
    al::addVelocityToDirection(shibaken, gravity, accel);
    f32 h = hScale < 0.0f ? 0.8f : hScale;
    al::scaleVelocityHV(shibaken, h, 0.8f);
}

void ShibakenFunction::limitFallVelocityOnGround(Shibaken* shibaken) {
    if (!al::isExistActorCollider(shibaken))
        return;
    if (!al::isCollidedGround(shibaken))
        return;
    sead::Vector3f negNormal = -al::getCollidedGroundNormal(shibaken);
    al::limitVelocityDirSign(shibaken, negNormal, 3.0f);
}

bool ShibakenFunction::isGroundNormal(const sead::Vector3f& normal, const Shibaken* shibaken) {
    GameDataHolderAccessor accessor(shibaken);
    bool isWorldMoon = GameDataFunction::isWorldMoon(accessor);
    const sead::Vector3f& grav = al::getGravity(shibaken);
    f32 dot = grav.x * normal.x + grav.y * normal.y + grav.z * normal.z;
    if (isWorldMoon)
        return dot < -0.34202f;
    return dot < -0.5f;
}

bool ShibakenFunction::executeReactionNerve(al::HostStateBase<Shibaken>* state) {
    if (al::isExistActorCollider(state->getHost()))
        addFallVelocityToGroundAndFitPoseOnGround(state->getHost(), -1.0f);
    return al::updateNerveState(state);
}

// NON_MATCHING: compiler generates fnmul+fsub for negated dot product, ours uses fadd+negated threshold
bool ShibakenFunction::checkStopChaseByFaceWall(const Shibaken* shibaken,
                                                 const sead::Vector3f& wallNormal) {
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    al::calcFrontDir(&front, shibaken);
    const sead::Vector3f& playerPos = rs::getPlayerPos(shibaken);
    sead::Vector3f toPlayer = playerPos - al::getTrans(shibaken);
    al::verticalizeVec(&toPlayer, al::getGravity(shibaken), toPlayer);
    if (!al::tryNormalizeOrZero(&toPlayer))
        return false;
    f32 angle = sead::Mathf::deg2rad(10.0f);
    f32 cos10 = cosf(angle);
    f32 negDot = -(wallNormal.x * toPlayer.x + wallNormal.y * toPlayer.y +
                   wallNormal.z * toPlayer.z);
    if (negDot >= cos10) {
        cos10 = cosf(angle);
        if (toPlayer.x * front.x + toPlayer.y * front.y + toPlayer.z * front.z >= cos10)
            return true;
    }
    return false;
}

bool ShibakenFunction::checkStopChaseByFaceWall(const Shibaken* shibaken) {
    if (!al::isCollidedWallVelocity(shibaken))
        return false;
    return checkStopChaseByFaceWall(shibaken, al::getCollidedWallNormal(shibaken));
}

// NON_MATCHING: fitPoseOnGround is extremely complex
void ShibakenFunction::addFallVelocityToGroundAndFitPoseOnGround(Shibaken* shibaken, f32 hScale) {
    if (!al::isExistActorCollider(shibaken))
        return;
    fitPoseOnGround(shibaken);
    sead::Vector3f gravity;
    if (al::isCollidedGround(shibaken)) {
        gravity = -al::getCollidedGroundNormal(shibaken);
    } else {
        gravity.set(al::getGravity(shibaken));
    }
    f32 gravAccel = 1.0f;
    if (shibaken->mWorldMoonFlag == 1)
        gravAccel = 0.6f;
    al::addVelocityToDirection(shibaken, gravity, gravAccel);
    f32 h = hScale < 0.0f ? 0.8f : hScale;
    al::scaleVelocityHV(shibaken, h, 0.8f);
}

void ShibakenFunction::chaseToTargetWalk(Shibaken* shibaken, const sead::Vector3f& target) {
    chaseToTarget(shibaken, target, 0.5f, true, false);
}

void ShibakenFunction::chaseToTargetWalkSniff(Shibaken* shibaken, const sead::Vector3f& target) {
    chaseToTarget(shibaken, target, 0.025f, true, true);
}

void ShibakenFunction::chaseToTargetRun(Shibaken* shibaken, const sead::Vector3f& target) {
    chaseToTarget(shibaken, target, 3.25f, false, false);
}

// NON_MATCHING: null check in IUseCollision cast and register allocation differences
bool ShibakenFunction::tryStartJump(al::IUseNerve* user, const Shibaken* shibaken,
                                    const al::Nerve* nerve) {
    if (!al::isExistActorCollider(shibaken))
        return false;
    if (al::isCollidedWallVelocity(shibaken)) {
        if (shibaken->mIsInvalidJumpLowWall)
            return false;
        const al::CollisionParts* wallParts = al::getCollidedWallCollisionParts(shibaken);
        const char* surfaceName = wallParts->getSpecialPurpose();
        if (surfaceName) {
            if (al::isEqualString("MoveLimit", surfaceName) ||
                al::isEqualString("ShibakenMoveLimit", surfaceName))
                return false;
        }
        sead::Vector3f upVec = {0.0f, 1.0f, 0.0f};
        al::verticalizeVec(&upVec, al::getCollidedWallNormal(shibaken), upVec);
        if (!al::tryNormalizeOrZero(&upVec))
            return false;
        sead::Vector3f fwd = {0.0f, 0.0f, 0.0f};
        al::calcFrontDir(&fwd, shibaken);
        al::verticalizeVec(&fwd, upVec, fwd);
        if (!al::tryNormalizeOrZero(&fwd))
            return false;
        const sead::Vector3f& trans = al::getTrans(shibaken);
        f32 radius = al::getColliderRadius(shibaken);
        sead::Vector3f start = {trans.x + upVec.x * radius * 3.0f,
                                trans.y + upVec.y * radius * 3.0f,
                                trans.z + upVec.z * radius * 3.0f};
        f32 r2 = al::getColliderRadius(shibaken);
        sead::Vector3f dir = {fwd.x * r2 + fwd.x * r2, fwd.y * r2 + fwd.y * r2,
                              fwd.z * r2 + fwd.z * r2};
        if (alCollisionUtil::getFirstPolyOnArrow(shibaken, nullptr, nullptr, start, dir,
                                                 &sIsNotMoveLimit, nullptr))
            return false;
        al::setNerve(user, nerve);
        return true;
    }
    if (!al::isOnGround(shibaken, 0))
        al::setNerve(user, nerve);
    return false;
}

bool ShibakenFunction::tryStartJump(al::HostStateBase<Shibaken>* state, const al::Nerve* nerve) {
    return tryStartJump(state, state->getHost(), nerve);
}

bool ShibakenFunction::executeFindTurnNerve(al::HostStateBase<Shibaken>* state,
                                            const sead::Vector3f& target, sead::Quatf* startQuat,
                                            sead::Quatf* targetQuat) {
    if (al::isFirstStep(state)) {
        al::startAction(state->getHost(), "Find");
        al::calcQuat(startQuat, state->getHost());
        sead::Vector3f toTarget = target - al::getTrans(state->getHost());
        sead::Vector3f up = {0.0f, 0.0f, 0.0f};
        al::calcUpDir(&up, state->getHost());
        al::verticalizeVec(&toTarget, up, toTarget);
        if (!al::tryNormalizeOrZero(&toTarget))
            al::calcFrontDir(&toTarget, state->getHost());
        al::makeQuatFrontUp(targetQuat, toTarget, up);
    }
    sead::Quatf* quat = al::getQuatPtr(state->getHost());
    f32 frameMax = al::getActionFrameMax(state->getHost(), al::getActionName(state->getHost()));
    f32 rate = al::calcNerveEaseOutRate(state, (s32)frameMax);
    al::slerpQuat(quat, *startQuat, *targetQuat, rate);
    return al::isActionEnd(state->getHost());
}

// NON_MATCHING: extremely complex velocity/rotation logic
void ShibakenFunction::chaseToTarget(Shibaken* shibaken, const sead::Vector3f& target, f32 speed,
                                     bool limitToTarget, bool skipTurn) {
    sead::Vector3f up = {0.0f, 0.0f, 0.0f};
    al::calcUpDir(&up, shibaken);

    if (!skipTurn) {
        sead::Vector3f toTarget = target - al::getTrans(shibaken);
        al::verticalizeVec(&toTarget, up, toTarget);
        if (al::isCollidedWallVelocity(shibaken))
            al::verticalizeVec(&toTarget, al::getCollidedWallNormal(shibaken), toTarget);
        if (al::tryNormalizeOrZero(&toTarget)) {
            sead::Vector3f front = {0.0f, 0.0f, 0.0f};
            al::calcFrontDir(&front, shibaken);
            f32 angle = al::calcAngleOnPlaneDegree(front, toTarget, up);
            f32 clampedAngle = angle;
            if (clampedAngle < -4.0f)
                clampedAngle = -4.0f;
            else if (clampedAngle > 4.0f)
                clampedAngle = 4.0f;
            al::rotateQuatYDirDegree(shibaken, clampedAngle * 0.5f);
        }
    }

    sead::Vector3f upDir = {0.0f, 0.0f, 0.0f};
    sead::Vector3f frontDir = {0.0f, 0.0f, 0.0f};
    al::calcUpDir(&upDir, shibaken);
    al::calcFrontDir(&frontDir, shibaken);

    if (al::isCollidedGround(shibaken)) {
        const sead::Vector3f& groundNormal = al::getCollidedGroundNormal(shibaken);
        GameDataHolderAccessor accessor(shibaken);
        bool isWorldMoon = GameDataFunction::isWorldMoon(accessor);
        const sead::Vector3f& grav = al::getGravity(shibaken);
        f32 dot =
            grav.x * groundNormal.x + grav.y * groundNormal.y + grav.z * groundNormal.z;
        bool isNormal = isWorldMoon ? dot < -0.34202f : dot < -0.5f;
        if (!isNormal) {
            sead::Vector3f adjustedUp = upDir;
            al::verticalizeVec(&adjustedUp, al::getCollidedGroundNormal(shibaken), adjustedUp);
            if (al::tryNormalizeOrZero(&adjustedUp)) {
                upDir = adjustedUp;
                al::verticalizeVec(&frontDir, upDir, frontDir);
                al::normalize(&frontDir);
                const sead::Vector3f& vel = al::getVelocity(shibaken);
                if (vel.x * upDir.x + vel.y * upDir.y + vel.z * upDir.z > 0.0f)
                    al::verticalizeVec(al::getVelocityPtr(shibaken), upDir,
                                       *al::getVelocityPtr(shibaken));
            }
        }
    }

    sead::Vector3f upVel = {0.0f, 0.0f, 0.0f};
    al::parallelizeVec(&upVel, up, al::getVelocity(shibaken));
    const sead::Vector3f& vel = al::getVelocity(shibaken);
    f32 hSpeed =
        sead::Mathf::sqrt((vel.x - upVel.x) * (vel.x - upVel.x) +
                          (vel.y - upVel.y) * (vel.y - upVel.y) +
                          (vel.z - upVel.z) * (vel.z - upVel.z)) +
        speed;

    if (limitToTarget) {
        sead::Vector3f toTarget = target - al::getTrans(shibaken);
        al::verticalizeVec(&toTarget, al::getGravity(shibaken), toTarget);
        if (al::isCollidedWallVelocity(shibaken))
            al::verticalizeVec(&toTarget, al::getCollidedWallNormal(shibaken), toTarget);
        f32 distScale =
            sead::Mathf::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y +
                              toTarget.z * toTarget.z) *
            0.9f;
        if (hSpeed >= distScale)
            hSpeed = distScale;
    }

    if (al::isCollidedWall(shibaken) && frontDir.y > 0.0f)
        frontDir.y = 0.0f;

    f32 gravAccel = 1.0f;
    if (shibaken->mWorldMoonFlag == 1)
        gravAccel = 0.6f;

    f32 upComponent = upDir.x * upVel.x + upDir.y * upVel.y + upDir.z * upVel.z - gravAccel;
    f32 groundFriction = shibaken->mWorldMoonFlag == 1 ? 0.95f : 0.8f;
    sead::Vector3f newVel;
    newVel.x = hSpeed * frontDir.x * 0.8f + groundFriction * upDir.x * upComponent;
    newVel.y = hSpeed * frontDir.y * 0.8f + groundFriction * upDir.y * upComponent;
    newVel.z = hSpeed * frontDir.z * 0.8f + groundFriction * upDir.z * upComponent;
    al::setVelocity(shibaken, newVel);

    if (al::isExistActorCollider(shibaken) && al::isCollidedGround(shibaken)) {
        sead::Vector3f negGndNormal = -al::getCollidedGroundNormal(shibaken);
        al::limitVelocityDirSign(shibaken, negGndNormal, 3.0f);
    }
    if (al::isCollidedWallVelocity(shibaken)) {
        sead::Vector3f negWallNormal = -al::getCollidedWallNormal(shibaken);
        al::limitVelocityDirSign(shibaken, negWallNormal, 0.4f);
    }
}
