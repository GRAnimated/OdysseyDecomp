#pragma once

#include <basis/seadTypes.h>

#include "Player/IJudge.h"

namespace al {
class LiveActor;
class WaterSurfaceFinder;
}  // namespace al
class IUsePlayerCollision;
class IUsePlayerHeightCheck;
class PlayerAreaChecker;
class PlayerConst;
class PlayerCounterForceRun;

class PlayerJudgeInWater : public IJudge {
public:
    PlayerJudgeInWater(const al::LiveActor* player, const PlayerConst* pConst,
                       const IUsePlayerCollision* collision, const PlayerAreaChecker* areaChecker,
                       const al::WaterSurfaceFinder* waterSurfaceFinder,
                       const IUsePlayerHeightCheck* heightCheck,
                       const PlayerCounterForceRun* counterForceRun, bool checkIceWater,
                       bool checkGroundOffset, bool ignoreSurface);

    bool judge() const override;
    void reset() override;
    void update() override;

private:
    const al::LiveActor* mPlayer;
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    const PlayerAreaChecker* mAreaChecker;
    const al::WaterSurfaceFinder* mWaterSurfaceFinder;
    const IUsePlayerHeightCheck* mHeightCheck;
    const PlayerCounterForceRun* mCounterForceRun;

    union {
        struct {
            bool checkIceWaterFlag;
            bool checkGroundOffsetFlag;
            bool ignoreSurfaceFlag;
            u8 padding43[5];
        };

        u32 mFlags;
    };
};

static_assert(sizeof(PlayerJudgeInWater) == 0x48);
