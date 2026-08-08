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

    bool judge() const override;
    void reset() override;
    void update() override;

private:
    const al::LiveActor* mActor;
    const IUsePlayerCollision* mCollision;
    const PlayerConst* mPlayerConst;
};

static_assert(sizeof(YoshiJudgeWallHitDown) == 0x28);
