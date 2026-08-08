#include "Player/PlayerJudgeWallHitDownRolling.h"

#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"

#include "Player/PlayerConst.h"
#include "Player/PlayerTrigger.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

PlayerJudgeWallHitDownRolling::PlayerJudgeWallHitDownRolling(
    const al::LiveActor* player, const IUsePlayerCollision* collision,
    const PlayerConst* pConst, const PlayerTrigger* trigger)
    : mPlayer(player), mCollision(collision), mConst(pConst), mTrigger(trigger) {}

bool PlayerJudgeWallHitDownRolling::judge() const {
    if (!rs::isCollidedWall(mCollision) || rs::isActionCodeNoActionWall(mCollision) ||
        mTrigger->isOn(PlayerTrigger::ECollisionTrigger_val9))
        return false;

    const sead::Vector3f& gravity = al::getGravity(mPlayer);
    const sead::Vector3f collidedWallNormal = rs::getCollidedWallNormal(mCollision);
    sead::Vector3f wallNormal = {0.0f, 0.0f, 0.0f};
    al::verticalizeVec(&wallNormal, gravity, collidedWallNormal);
    if (!al::tryNormalizeOrZero(&wallNormal))
        return false;

    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    if (!rs::calcAlongSkyFront(&front, mPlayer))
        return false;
    if (al::calcAngleDegree(wallNormal, -front) > mConst->getCollisionHitDownAngleH())
        return false;

    sead::Vector3f groundNormal = {0.0f, 0.0f, 0.0f};
    if (!rs::isCollidedGround(mCollision))
        return true;

    const sead::Vector3f wallPos = rs::getCollidedWallPos(mCollision);
    const sead::Vector3f groundPos = rs::getCollidedGroundPos(mCollision);
    f32 groundOffset = (groundPos - wallPos).dot(gravity);
    if (al::isNearZeroOrLess(groundOffset - mConst->getCollisionSmallStepHeight(), 0.001f))
        return false;

    groundNormal = rs::getCollidedGroundNormal(mCollision);
    return al::calcAngleDegree(groundNormal, collidedWallNormal) >
           mConst->getCollisionHitDownEscapeAngleV();
}

void PlayerJudgeWallHitDownRolling::reset() {}

void PlayerJudgeWallHitDownRolling::update() {}
