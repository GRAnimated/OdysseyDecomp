#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}
class IJudge;
class IUsePlayerCollision;
class PlayerAnimator;
class PlayerActionAirMoveControl;
class PlayerActionGroundMoveControl;
class PlayerConst;
class PlayerInput;

class PlayerStateDamageFire : public al::ActorStateBase {
public:
    PlayerStateDamageFire(al::LiveActor* player, const PlayerConst* pConst,
                          const IUsePlayerCollision* collision, const PlayerInput* input,
                          PlayerAnimator* animator, IJudge* judgeSpeedCheckFall);
    void appear() override;

    void control() override;
    bool isEndFirstLand() const;
    bool isEnableJump() const;
    bool isEnablePeachAmiibo() const;
    s32 getEnableCancelCollisionSnapFrame() const;
    void exeJump();
    void exeJump2nd();
    void exeRun();
    void exeFall();
    void exeDead();
    ~PlayerStateDamageFire() override;

private:
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    const PlayerInput* mInput;
    PlayerActionAirMoveControl* mAirMoveControl;
    PlayerActionGroundMoveControl* mGroundMoveControl;
    PlayerAnimator* mAnimator;
    IJudge* mJudgeSpeedCheckFall;
    bool mIsDamageFireGround;
    bool mIsDamageFireWall;
    bool mIsDamageFireCeiling;
    u8 _5b;
    s32 mControlCounter;
    s32 _60;
    u8 _64[4];
};

static_assert(sizeof(PlayerStateDamageFire) == 0x68);
