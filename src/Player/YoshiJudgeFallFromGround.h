#pragma once

#include <basis/seadTypes.h>

#include "Player/IJudge.h"

namespace al {
class LiveActor;
}

class IUsePlayerCollision;
class PlayerConst;

class YoshiJudgeFallFromGround : public IJudge {
public:
    YoshiJudgeFallFromGround(const al::LiveActor* actor, const PlayerConst* playerConst,
                             const IUsePlayerCollision* collision);

    void reset() override;
    void update() override;
    bool judge() const override;

private:
    const al::LiveActor* mActor;
    const PlayerConst* mPlayerConst;
    const IUsePlayerCollision* mCollision;
    s32 mCount = 0;
};

static_assert(sizeof(YoshiJudgeFallFromGround) == 0x28);
