#pragma once

#include <basis/seadTypes.h>

#include "Player/IJudge.h"

namespace al {
class WaterSurfaceFinder;
}
class PlayerConst;
class PlayerInput;

class PlayerJudgeStartSwimJump : public IJudge {
public:
    PlayerJudgeStartSwimJump(const PlayerInput* input, const PlayerConst* pConst,
                             const al::WaterSurfaceFinder* waterSurfaceFinder);

    void reset() override;
    void update() override;
    bool judge() const override;

private:
    const PlayerInput* mInput;
    const PlayerConst* mConst;
    const al::WaterSurfaceFinder* mWaterSurfaceFinder;
    s32 mRemainFrame = 0;
    u8 _24[4];
};

static_assert(sizeof(PlayerJudgeStartSwimJump) == 0x28);
