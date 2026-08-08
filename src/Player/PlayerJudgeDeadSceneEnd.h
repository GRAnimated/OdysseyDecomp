#pragma once

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}

class PlayerJudgeDeadSceneEnd : public IJudge {
public:
    PlayerJudgeDeadSceneEnd(const al::LiveActor* player, const IJudge* judge);

    bool judge() const override;
    void reset() override {}
    void update() override {}

private:
    const al::LiveActor* mPlayer;
    const IJudge* mJudge;
};

