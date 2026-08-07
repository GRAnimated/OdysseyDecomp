#pragma once

#include "Player/IJudge.h"

class PlayerCarryKeeper;
class PlayerHackKeeper;
class PlayerStateSwim;

class PlayerJudgeTalkSwim : public IJudge {
public:
    PlayerJudgeTalkSwim(const PlayerHackKeeper* hackKeeper,
                        const PlayerCarryKeeper* carryKeeper,
                        const PlayerStateSwim* stateSwim);

    void reset() override;
    void update() override;
    bool judge() const override;

private:
    const PlayerHackKeeper* mHackKeeper;
    const PlayerCarryKeeper* mCarryKeeper;
    const PlayerStateSwim* mStateSwim;
};

static_assert(sizeof(PlayerJudgeTalkSwim) == 0x20);
