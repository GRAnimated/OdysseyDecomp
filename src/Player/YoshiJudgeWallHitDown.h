#pragma once

#include "Util/HackerJudge.h"

namespace al {
class LiveActor;
}

class IUsePlayerCollision;
class IUsePlayerHack;
class PlayerConst;

class YoshiJudgeWallHitDown : public HackerJudge {
public:
    YoshiJudgeWallHitDown(IUsePlayerHack** hacker, const al::LiveActor* actor,
                          const IUsePlayerCollision* collision, const PlayerConst* playerConst);

    void reset() override;
    void update() override;
    bool judge() const override;

private:
    const al::LiveActor* mActor;
    const IUsePlayerCollision* mCollision;
    const PlayerConst* mPlayerConst;
};

static_assert(sizeof(YoshiJudgeWallHitDown) == 0x28);
