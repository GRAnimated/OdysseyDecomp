#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class AreaObj;
class LiveActor;
}
class IUseDimension;
class IUsePlayerCollision;
class PlayerAnimator;
class PlayerAreaChecker;
class PlayerConst;
class PlayerInput;
class PlayerJudgeInvalidateInputFall;
class PlayerActionAirMoveControl;
class PlayerTrigger;

class PlayerStateFallHakoniwa : public al::ActorStateBase {
public:
    PlayerStateFallHakoniwa(al::LiveActor* player, const PlayerConst* pConst,
                            const IUsePlayerCollision* collision, const PlayerInput* input,
                            const PlayerTrigger* trigger, const PlayerAreaChecker* areaChecker,
                            const IUseDimension* dimension, PlayerAnimator* animator);
    ~PlayerStateFallHakoniwa() override;

    void appear() override;
    void updateFallTargetArea();
    bool isValidWallCatch() const;
    bool isInvalidInputFall() const { return mIsInvalidInputFall; }
    bool isRunFall() const { return mIsRunFall; }
    bool hasFallTargetArea() const { return mFallTargetArea != nullptr; }
    void exeFall();

private:
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    const PlayerTrigger* mTrigger;
    const IUseDimension* mDimension;
    PlayerAnimator* mAnimator;
    PlayerJudgeInvalidateInputFall* mJudgeInvalidateInputFall;
    PlayerActionAirMoveControl* mAirMoveControl;
    s32 _58;
    s32 _5c;
    sead::Vector3f _60;
    bool mIsInvalidInputFall;
    bool mIsRunFall;
    u8 _6e[2];
    const al::AreaObj* mFallTargetArea;
    f32 mFallSpeed;
    u8 _7c[4];
};

static_assert(sizeof(PlayerStateFallHakoniwa) == 0x80);
