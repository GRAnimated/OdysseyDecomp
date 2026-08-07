#pragma once

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}

class IUsePlayerCollision;
class PlayerInput;

class PlayerJudgeStartRunOld : public IJudge {
public:
    PlayerJudgeStartRunOld(const al::LiveActor* player, const IUsePlayerCollision* collision,
                           const PlayerInput* input);

    void reset() override;
    void update() override;
    bool judge() const override;

private:
    const al::LiveActor* mPlayer;
    bool mIsStartRun;
    const IUsePlayerCollision* mCollision;
    const PlayerInput* mInput;
};

static_assert(sizeof(PlayerJudgeStartRunOld) == 0x28);
