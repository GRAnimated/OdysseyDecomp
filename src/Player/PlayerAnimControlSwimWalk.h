#pragma once

#include <basis/seadTypes.h>

class PlayerAnimator;
class PlayerConst;

class PlayerAnimControlSwimWalk {
public:
    PlayerAnimControlSwimWalk(PlayerAnimator* animator, const PlayerConst* playerConst);

    void update(f32 speed);

private:
    PlayerAnimator* mAnimator;
    const PlayerConst* mPlayerConst;
};

static_assert(sizeof(PlayerAnimControlSwimWalk) == 0x10);
