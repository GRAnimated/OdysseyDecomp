#pragma once

#include <basis/seadTypes.h>

#include "Player/IJudge.h"

namespace al {
class LiveActor;
class WaterSurfaceFinder;
}  // namespace al
class PlayerConst;
class PlayerCounterForceRun;

class PlayerJudgeWaterSurfaceRun : public IJudge {
public:
    PlayerJudgeWaterSurfaceRun(const al::LiveActor* player, const PlayerConst* pConst,
                               const al::WaterSurfaceFinder* waterSurfaceFinder,
                               const PlayerCounterForceRun* counterForceRun);

    f32 getBorderSpeedH() const;

    void update() override;

    void reset() override { mIsWaterSurfaceRun = false; }

    bool judge() const override { return mIsWaterSurfaceRun; }

private:
    friend class PlayerStateRunHakoniwa2D3D;
    friend class PlayerStateSpinCap;

    bool mIsWaterSurfaceRun = false;
    bool mIsEnable = false;
    const al::LiveActor* mPlayer;
    const PlayerConst* mConst;
    const al::WaterSurfaceFinder* mWaterSurfaceFinder;
    const PlayerCounterForceRun* mCounterForceRun;
};

static_assert(sizeof(PlayerJudgeWaterSurfaceRun) == 0x30);
