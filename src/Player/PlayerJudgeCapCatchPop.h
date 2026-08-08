#pragma once

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}
class IPlayerModelChanger;
class IUsePlayerCollision;
class PlayerCapActionHistory;
class PlayerCounterAfterCapCatch;
class PlayerInput;

class PlayerJudgeCapCatchPop : public IJudge {
public:
    PlayerJudgeCapCatchPop(const al::LiveActor* player, const PlayerInput* input,
                           const IUsePlayerCollision* collision,
                           const IPlayerModelChanger* modelChanger,
                           const PlayerCapActionHistory* capActionHistory,
                           const PlayerCounterAfterCapCatch* counterAfterCapCatch);

    bool judge() const override;
    void reset() override {}
    void update() override {}

private:
    const al::LiveActor* mPlayer;
    const PlayerInput* mInput;
    const IUsePlayerCollision* mCollision;
    const IPlayerModelChanger* mModelChanger;
    const PlayerCapActionHistory* mCapActionHistory;
    const PlayerCounterAfterCapCatch* mCounterAfterCapCatch;
};

static_assert(sizeof(PlayerJudgeCapCatchPop) == 0x38);
