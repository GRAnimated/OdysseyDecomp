#pragma once

#include <basis/seadTypes.h>

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}

class PlayerJudgeDeadWipeStart;

class PlayerJudgeDrawForward : public IJudge {
public:
    PlayerJudgeDrawForward(const al::LiveActor* playerActor,
                           const PlayerJudgeDeadWipeStart* playerJudgeDeadWipeStart);

    bool judge() const override;
    void reset() override {}
    void update() override {}

private:
    const al::LiveActor* mPlayerActor;
    const PlayerJudgeDeadWipeStart* mPlayerJudgeDeadWipeStart;
};

static_assert(sizeof(PlayerJudgeDrawForward) == 0x18);
