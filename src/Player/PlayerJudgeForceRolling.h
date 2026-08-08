#pragma once

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}
class IUsePlayerCollision;

class PlayerJudgeForceRolling : public IJudge {
public:
    PlayerJudgeForceRolling(const al::LiveActor* player, const IUsePlayerCollision* collider);

    bool judge() const override;

    void reset() override {}

    void update() override {}

private:
    const al::LiveActor* mPlayer;
    const IUsePlayerCollision* mCollider;
};

static_assert(sizeof(PlayerJudgeForceRolling) == 0x18);
