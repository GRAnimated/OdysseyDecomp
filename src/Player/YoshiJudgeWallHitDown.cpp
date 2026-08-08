#include "Player/YoshiJudgeWallHitDown.h"

#include "Library/LiveActor/ActorPoseKeeper.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"

#include "Player/PlayerConst.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

YoshiJudgeWallHitDown::YoshiJudgeWallHitDown(IUsePlayerHack** hacker,
                                             const al::LiveActor* actor,
                                             const IUsePlayerCollision* collision,
                                             const PlayerConst* playerConst)
    : HackerJudge(hacker), mActor(actor), mCollision(collision), mPlayerConst(playerConst) {}

bool YoshiJudgeWallHitDown::judge() const {
    if (!rs::isCollidedWall(mCollision))
        return false;
    if (rs::isActionCodeNoActionWall(mCollision))
        return false;

    const sead::Vector3f& gravity = al::getGravity(mActor);
    sead::Vector3f wallNormal = rs::getCollidedWallNormal(mCollision);
    al::verticalizeVec(&wallNormal, gravity, wallNormal);
    if (!al::tryNormalizeOrZero(&wallNormal))
        return false;

    sead::Vector3f alongSkyFront(0.0f, 0.0f, 0.0f);
    if (!rs::calcAlongSkyFront(&alongSkyFront, mActor))
        return false;

    return al::calcAngleDegree(wallNormal, -alongSkyFront) <=
           mPlayerConst->getCollisionHitDownAngleH();
}

void YoshiJudgeWallHitDown::reset() {}

void YoshiJudgeWallHitDown::update() {}

