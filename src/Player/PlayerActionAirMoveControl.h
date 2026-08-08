#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
}
class PlayerConst;
class PlayerInput;
class PlayerActionTurnControl;
class IUsePlayerCollision;
class IUsePlayerHack;

class PlayerActionAirMoveControl {
public:
    PlayerActionAirMoveControl(al::LiveActor* actor, const PlayerConst* playerConst,
                               const PlayerInput* input, const IUsePlayerCollision* collision,
                               bool isSlerpGravity);
    void setup(f32 speedMax, f32 inertiaAdd, s32 extendFrame, f32 velocityV, f32 gravityAccel,
               s32 noInputFrame, f32 inertiaRate);
    void setupTurn(f32 angleStart, f32 angleFast, f32 angleLimit, f32 angleFastLimit,
                   s32 accelFrame, s32 accelFrameFast, s32 brakeFrame);
    void setExtendFrame(s32 frame);
    void setupCollideWallScaleVelocity(f32 velocityScaleH, f32 maxVelocityH, f32 maxScaledVelocityH);
    void verticalizeStartMoveDir(const sead::Vector3f& vertical);
    void update();
    bool isHoldJumpExtend() const;
    void calcMoveInput(sead::Vector3f* moveInput, const sead::Vector3f& up) const;
    const sead::Vector3f& getStartMoveDir() const { return mStartMoveDir; }
    void setIsPlayer2D(bool value) { _33 = value; }
    void setPlayerHack(IUsePlayerHack** playerHack) { _20 = playerHack; }
    void setUseGroundNormalForStartMove(bool value) { _40 = value; }
    void setIsInertiaWall(bool value) { _41 = value; }
    void setStartMoveSpeedClamp(bool value, f32 minSpeed, f32 maxSpeed) {
        _42 = value;
        _44 = minSpeed;
        _48 = maxSpeed;
    }

private:
    al::LiveActor* mActor;
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    const IUsePlayerCollision* mCollision;
    const IUsePlayerHack* const* _20;
    PlayerActionTurnControl* mTurnControl;
    bool _30;
    bool mIsHoldJumpExtend;
    bool _32;
    bool _33;
    s32 mExtendFrame;
    s32 _38;
    s32 _3c;
    bool _40;
    bool _41;
    bool _42;
    u8 _43;
    f32 _44;
    f32 _48;
    sead::Vector3f mStartMoveDir;
    sead::Vector3f _58;
    f32 _64;
    f32 _68;
    f32 _6c;
    bool _70;
    u8 _71[3];
    f32 _74;
    f32 _78;
    f32 _7c;
    f32 _80;
    sead::Vector3f _84;
};

static_assert(sizeof(PlayerActionAirMoveControl) == 0x90);
