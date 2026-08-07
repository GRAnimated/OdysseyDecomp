#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

class IJudge;
class PlayerAnimator;
class PlayerConst;
class PlayerEffect;

class PlayerAnimControlRun {
public:
    PlayerAnimControlRun(PlayerAnimator* animator, const PlayerConst* playerConst,
                         const IJudge* judge, PlayerEffect* effect, bool isMoon);

    void reset(f32 speed, bool forceReset);
    bool isAnimDashFast() const;
    void update(f32 speed, const sead::Vector3f& velocity);

private:
    PlayerAnimator* mAnimator;
    PlayerEffect* mEffect;
    const PlayerConst* mPlayerConst;
    const IJudge* mJudge;
    bool mIsRunStart;
    u8 _21[3];
    s32 mRunStartBlendRate;
    s32 mPrevSpeed;
    f32 mBlendWeight;
    bool mIsMoon;
    u8 _31[7];
    const char* mMoveAnimName;
};

static_assert(sizeof(PlayerAnimControlRun) == 0x40);
