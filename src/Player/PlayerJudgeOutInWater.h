#pragma once

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}

class PlayerJudgeOutInWater : public IJudge {
public:
    PlayerJudgeOutInWater(const al::LiveActor* player, const IJudge* judgeInWater,
                          const IJudge* judgeInWaterSurface);

    void reset() override;
    void update() override;
    bool judge() const override;

private:
    const al::LiveActor* mPlayer;
    const IJudge* mJudgeInWater;
    const IJudge* mJudgeInWaterSurface;
    bool mIsInWater;
};

static_assert(sizeof(PlayerJudgeOutInWater) == 0x28);
