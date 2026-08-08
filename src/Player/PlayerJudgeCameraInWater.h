#pragma once

#include "Library/Nature/WaterSurfaceFinder.h"

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}

class PlayerJudgeCameraInWater : public IJudge {
public:
    PlayerJudgeCameraInWater(const al::LiveActor* player, const IJudge* inWaterJudge,
                             const al::WaterSurfaceFinder* waterSurfaceFinder);

    void reset() override {}
    void update() override {}
    bool judge() const override;

private:
    const al::LiveActor* mPlayer;
    const IJudge* mInWaterJudge;
    const al::WaterSurfaceFinder* mWaterSurfaceFinder;
};

static_assert(sizeof(PlayerJudgeCameraInWater) == 0x20);
