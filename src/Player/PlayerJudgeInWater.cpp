#include "Player/PlayerJudgeInWater.h"

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nature/WaterSurfaceFinder.h"

#include "Player/PlayerAreaChecker.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerCounterForceRun.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerUtil.h"

namespace {
bool isInWaterArea(const PlayerConst* pConst, const PlayerAreaChecker* areaChecker,
                   const sead::Vector3f& trans, bool checkIceWater, bool ignoreSurface) {
    if (checkIceWater)
        return areaChecker->isInWaterIceDouble(trans, pConst->getSwimCenterOffset());
    if (ignoreSurface)
        return areaChecker->isInWaterWallCatch(trans, pConst->getSwimWallCatchOffset());
    return areaChecker->isInWaterDouble(trans, pConst->getSwimCenterOffset());
}
}  // namespace

PlayerJudgeInWater::PlayerJudgeInWater(
    const al::LiveActor* player, const PlayerConst* pConst, const IUsePlayerCollision* collision,
    const PlayerAreaChecker* areaChecker, const al::WaterSurfaceFinder* waterSurfaceFinder,
    const IUsePlayerHeightCheck* heightCheck, const PlayerCounterForceRun* counterForceRun,
    bool checkIceWater, bool checkGroundOffset, bool ignoreSurface)
    : mPlayer(player), mConst(pConst), mCollision(collision), mAreaChecker(areaChecker),
      mWaterSurfaceFinder(waterSurfaceFinder), mHeightCheck(heightCheck),
      mCounterForceRun(counterForceRun), checkIceWaterFlag(checkIceWater),
      checkGroundOffsetFlag(checkGroundOffset), ignoreSurfaceFlag(ignoreSurface) {
    (void)ignoreSurfaceFlag;
}

bool PlayerJudgeInWater::judge() const {
    if (rs::isPlayer2D(mPlayer))
        return false;

    const PlayerConst* pConst = mConst;
    const bool checkIceWater = checkIceWaterFlag;
    const bool ignoreSurface = ignoreSurfaceFlag;
    if (!isInWaterArea(pConst, mAreaChecker, al::getTrans(mPlayer), checkIceWater, ignoreSurface))
        return false;

    if (!mWaterSurfaceFinder->isFoundSurface())
        return true;
    if (mCounterForceRun && mCounterForceRun->isForceRun())
        return false;
    if (ignoreSurfaceFlag)
        return true;

    f32 height = mConst->getSwimCenterOffset();
    if (checkGroundOffsetFlag) {
        f32 groundOffset = 0.0f;
        if (rs::isCollidedGround(mCollision)) {
            const sead::Vector3f& groundNormal = rs::getCollidedGroundNormal(mCollision);
            sead::Vector3f up = {0.0f, 0.0f, 0.0f};
            al::calcUpDir(&up, mPlayer);
            if (!al::isParallelDirection(up, groundNormal, 0.01f) && up.dot(groundNormal) > 0.0f) {
                sead::Quatf rotation = sead::Quatf::unit;
                al::makeQuatRotationRate(&rotation, up, groundNormal, 1.0f);
                sead::Vector3f offset =
                    al::getTrans(mPlayer) - rs::getCollidedGroundPos(mCollision);
                offset.rotate(rotation);
                groundOffset = sead::Mathf::clampMin(al::getGravity(mPlayer).dot(offset), 0.0f);
            }
        }
        height = sead::Mathf::clampMin(height - (groundOffset + 5.0f), 0.0f);
    }

    return !rs::isInPuddleHeight(mWaterSurfaceFinder, mHeightCheck, height);
}
void PlayerJudgeInWater::reset() {}

void PlayerJudgeInWater::update() {}

