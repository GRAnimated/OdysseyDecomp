#pragma once

#include <basis/seadTypes.h>

#include "Player/IJudge.h"

class TouchTargetKeeper;

class PlayerJudgeTouchMove : public IJudge {
public:
    explicit PlayerJudgeTouchMove(const TouchTargetKeeper* keeper);

    void reset() override;
    void update() override;
    bool judge() const override;

private:
    const TouchTargetKeeper* mTouchTargetKeeper;
    s32 mKeepFrame;
    s32 mRemainFrame;
};

static_assert(sizeof(PlayerJudgeTouchMove) == 0x18);
