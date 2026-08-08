#include "Player/PlayerJudgeWallHitDownForceRun.h"

#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"

#include "Player/PlayerConst.h"
#include "Player/PlayerCounterForceRun.h"
#include "Player/PlayerTrigger.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
bool judgeCollision(const PlayerConst* pConst, const sead::Vector3f& front,
                    const sead::Vector3f& collisionNormal, const sead::Vector3f& up,
                    const sead::Vector3f& groundNormal);
}  // namespace

PlayerJudgeWallHitDownForceRun::PlayerJudgeWallHitDownForceRun(
    const al::LiveActor* player, const IUsePlayerCollision* collision,
    const PlayerConst* pConst, const PlayerCounterForceRun* counterForceRun,
    const PlayerTrigger* trigger)
    : mPlayer(player), mCollision(collision), mConst(pConst),
      mCounterForceRun(counterForceRun), mTrigger(trigger) {
    (void)mTrigger;
}

bool PlayerJudgeWallHitDownForceRun::judge() const {
    if (!mCounterForceRun->isForceRun() ||
        mTrigger->isOn(PlayerTrigger::ECollisionTrigger_val9))
        return false;

    bool isWall = rs::isCollidedWall(mCollision) && !rs::isActionCodeNoActionWall(mCollision);
    bool isCeiling =
        rs::isCollidedCeiling(mCollision) && !rs::isActionCodeNoActionCeiling(mCollision);
    if (!isWall && !isCeiling)
        return false;

    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    if (!rs::calcAlongSkyFront(&front, mPlayer))
        return false;

    sead::Vector3f groundNormal = {0.0f, 0.0f, 0.0f};
    rs::calcGroundNormalOrUpDir(&groundNormal, mPlayer, mCollision);
    sead::Vector3f up = al::getGravity(mPlayer);
    up.negate();

    if (isWall && judgeCollision(mConst, front, rs::getCollidedWallNormal(mCollision), up,
                                 groundNormal))
        return true;
    if (isCeiling && judgeCollision(mConst, front, rs::getCollidedCeilingNormal(mCollision), up,
                                    groundNormal))
        return true;
    return false;
}

namespace {
bool judgeCollision(const PlayerConst* pConst, const sead::Vector3f& front,
                    const sead::Vector3f& collisionNormal, const sead::Vector3f& up,
                    const sead::Vector3f& groundNormal) {
    sead::Vector3f normalH = {0.0f, 0.0f, 0.0f};
    al::verticalizeVec(&normalH, up, collisionNormal);
    if (!al::tryNormalizeOrZero(&normalH))
        return false;
    if (al::calcAngleDegree(normalH, -front) > pConst->getCollisionHitDownAngleH())
        return false;
    f32 escape = pConst->getCollisionHitDownEscapeAngleV();
    return al::isInRange(al::calcAngleDegree(groundNormal, collisionNormal), escape,
                         180.0f - escape);
}
}  // namespace

void PlayerJudgeWallHitDownForceRun::reset() {}

void PlayerJudgeWallHitDownForceRun::update() {}
