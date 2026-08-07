#include "Player/PlayerJudgeReduceOxygen.h"

#include "Library/Nature/WaterSurfaceFinder.h"

#include "Player/PlayerConst.h"

PlayerJudgeReduceOxygen::PlayerJudgeReduceOxygen(const PlayerConst* playerConst,
                                                 const al::WaterSurfaceFinder* waterSurfaceFinder)
    : mConst(playerConst), mWaterSurfaceFinder(waterSurfaceFinder) {}

bool PlayerJudgeReduceOxygen::judge() const {
    if (!mWaterSurfaceFinder->isFoundSurface())
        return true;
    return mWaterSurfaceFinder->getDistance() > mConst->getSwimSurfaceStartDist();
}
