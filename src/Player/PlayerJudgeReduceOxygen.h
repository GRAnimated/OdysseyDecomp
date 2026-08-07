#pragma once

#include "Player/IJudge.h"

namespace al {
class WaterSurfaceFinder;
}
class PlayerConst;

class PlayerJudgeReduceOxygen : public IJudge {
public:
    PlayerJudgeReduceOxygen(const PlayerConst* playerConst,
                            const al::WaterSurfaceFinder* waterSurfaceFinder);

    void reset() override {}
    void update() override {}
    bool judge() const override;

private:
    const PlayerConst* mConst;
    const al::WaterSurfaceFinder* mWaterSurfaceFinder;
};

static_assert(sizeof(PlayerJudgeReduceOxygen) == 0x18);
