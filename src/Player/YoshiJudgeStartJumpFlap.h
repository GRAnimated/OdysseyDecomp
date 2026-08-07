#pragma once

#include "Util/HackerJudge.h"

namespace al {
class LiveActor;
}

class IUsePlayerCollision;
class IUsePlayerHack;
class PlayerConst;

class YoshiJudgeStartJumpFlap : public HackerJudge {
public:
    YoshiJudgeStartJumpFlap(IUsePlayerHack** hacker, const al::LiveActor* actor,
                            const PlayerConst* playerConst,
                            const IUsePlayerCollision* collision);

    void reset() override;
    void update() override;
    bool judge() const override;

    void start() { mIsStarted = true; }

private:
    const al::LiveActor* mActor;
    const PlayerConst* mPlayerConst;
    const IUsePlayerCollision* mCollision;
    bool mIsStarted = false;
};

static_assert(sizeof(YoshiJudgeStartJumpFlap) == 0x30);
