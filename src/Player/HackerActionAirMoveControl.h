#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
}

class IUsePlayerCollision;
class IUsePlayerHack;
class PlayerActionTurnControl;

class HackerActionAirMoveControl {
public:
    HackerActionAirMoveControl(al::LiveActor* actor, bool isSlerpGravity, bool isUseAnyButton);

    void setup(f32 speedMax, f32 inertiaAdd, s32 extendFrame, f32 velocityV, f32 gravityAccel, s32 noInputFrame, f32 inertiaRate, f32 fallSpeedMax, f32 normalMinSpeed, f32 slerpQuatRate, f32 accelFront, f32 accelBack, f32 accelTurn);
    void setupTurn(f32 turnAngleStart, f32 turnAngleFast, f32 turnAngleLimit, f32 turnAngleFastLimit, s32 turnAccelFrame, s32 turnAccelFrameFast, s32 turnBrakeFrame);
    void setExtendFrame(s32 frame);
    void setupCollideWallScaleVelocity(f32 velocityScaleH, f32 maxVelocityH, f32 maxScaledVelocityH);
    void verticalizeStartMoveDir(const sead::Vector3f& vertical);
    void update();
    bool isHoldJumpExtend() const;
    void calcMoveInput(sead::Vector3f* moveInput, const sead::Vector3f& up) const;

    void setPlayerHack(IUsePlayerHack** playerHack) { mPlayerHack = playerHack; }

    void setForceJumpExtend(bool value) { mIsForceJumpExtend = value; }

    void set_28(bool value) { mIsGroundUp = value; }

    void set_2a(bool value) { mIsClampSpeed = value; }

    void set_2c_30(s32 value2c, s32 value30) {
        _2cbits = value2c;
        _30bits = value30;
    }

    f32 get_4c() const { return mSpeedH; }

private:
    al::LiveActor* mActor;
    IUsePlayerHack** mPlayerHack;
    PlayerActionTurnControl* mTurnControl;
    bool mIsSlerpGravity;
    bool mIsUseAnyButton;
    bool mIsHoldJumpExtend;
    bool mIsForceJumpExtend;
    s32 mExtendFrame;
    s32 mFrame;
    s32 mNoInputFrame;
    bool mIsGroundUp;
    bool mIsInertiaWall;
    bool mIsClampSpeed;
    u8 _2b;

    union {
        f32 mClampSpeedMin;
        s32 _2cbits;
    };

    union {
        f32 mClampSpeedMax;
        s32 _30bits;
    };

    sead::Vector3f mStartMoveDir;
    sead::Vector3f mSideDir;
    f32 mSpeedH;
    f32 mGravityAccel;
    f32 mFallSpeedMax;
    bool mIsScaleWallVelocity;
    u8 _59[3];
    f32 mVelocityScaleWallH;
    f32 mMaxVelocityWallH;
    f32 mMaxScaledVelocityWallH;
    f32 mSlerpQuatRate;
    sead::Vector3f mPrevInput;
    f32 mAccelFront;
    f32 mAccelBack;
    f32 mAccelTurn;
    u8 _84[4];
    const IUsePlayerCollision* mCollision;
};

static_assert(sizeof(HackerActionAirMoveControl) == 0x90);
