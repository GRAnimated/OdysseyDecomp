#pragma once

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}

class PlayerJudgeDeadSceneEnd : public IJudge {
public:
    PlayerJudgeDeadSceneEnd(const al::LiveActor* player, const IJudge* judge);

    void reset() override {}
    void update() override {}
    bool judge() const override;

private:
    const al::LiveActor* mPlayer;
    const IJudge* mJudge;
};

static_assert(sizeof(PlayerJudgeDeadSceneEnd) == 0x18);
