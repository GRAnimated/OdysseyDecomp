#pragma once

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}
class IUsePlayerCollision;
class PlayerConst;
class PlayerTrigger;

class PlayerJudgeWallHitDownRolling : public IJudge {
public:
    PlayerJudgeWallHitDownRolling(const al::LiveActor* player,
                                  const IUsePlayerCollision* collision,
                                  const PlayerConst* pConst,
                                  const PlayerTrigger* trigger);
    void reset() override;
    void update() override;
    bool judge() const override;

private:
    const al::LiveActor* mPlayer;
    const IUsePlayerCollision* mCollision;
    const PlayerConst* mConst;
    const PlayerTrigger* mTrigger;
};

static_assert(sizeof(PlayerJudgeWallHitDownRolling) == 0x28);
