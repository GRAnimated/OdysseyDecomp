#pragma once

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}
class PlayerCounterAfterCapCatch;

class PlayerJudgeCapCatch : public IJudge {
public:
    PlayerJudgeCapCatch(const al::LiveActor* player,
                        const PlayerCounterAfterCapCatch* counterAfterCapCatch);

    void reset() override {}
    void update() override {}
    bool judge() const override;

private:
    const al::LiveActor* mPlayer;
    const PlayerCounterAfterCapCatch* mCounterAfterCapCatch;
};

static_assert(sizeof(PlayerJudgeCapCatch) == 0x18);
