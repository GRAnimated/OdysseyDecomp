#pragma once

#include <basis/seadTypes.h>

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}
class IUsePlayerCollision;
class PlayerBindKeeper;
class PlayerDamageKeeper;
class PlayerHackKeeper;
class PlayerInput;
class PlayerStateSwim;
class PlayerStateWait;

class PlayerJudgeRecoveryLifeFast : public IJudge {
public:
    PlayerJudgeRecoveryLifeFast(const al::LiveActor* player,
                                const PlayerDamageKeeper* damageKeeper,
                                const PlayerBindKeeper* bindKeeper,
                                const IUsePlayerCollision* collision,
                                const PlayerInput* input,
                                const PlayerHackKeeper* hackKeeper,
                                const PlayerStateWait* stateWait,
                                const PlayerStateSwim* stateSwim,
                                const IJudge* judgeInWater);

    bool judge() const override;
    bool isEnableRecoveryLifeState() const;
    void reset() override;
    void update() override;

private:
    const al::LiveActor* mPlayer;
    const PlayerDamageKeeper* mDamageKeeper;
    const PlayerBindKeeper* mBindKeeper;
    const IUsePlayerCollision* mCollision;
    const PlayerInput* mInput;
    const PlayerHackKeeper* mHackKeeper;
    const PlayerStateWait* mStateWait;
    const PlayerStateSwim* mStateSwim;
    const IJudge* mJudgeInWater;
};

static_assert(sizeof(PlayerJudgeRecoveryLifeFast) == 0x50);
