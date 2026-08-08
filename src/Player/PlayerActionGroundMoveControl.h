#pragma once

#include <math/seadVector.h>

namespace al {
class LiveActor;
}

class PlayerConst;
class PlayerInput;
class IUsePlayerCollision;
class IUsePlayerHack;
class IJudge;
class PlayerActionTurnControl;

class PlayerActionGroundMoveControl {
public:
    friend class PlayerStateDamageFire;
    friend class PlayerStateSpinCap;

    PlayerActionGroundMoveControl(al::LiveActor* parent, const PlayerConst* playerConst,
                                  const PlayerInput* playerInput,
                                  const IUsePlayerCollision* collision);

    void initDash(IJudge* judge, f32 speed, s32 frame);
    void setupDash(f32 speed, s32 frame);
    void setup(f32 maxSpeed, f32 minSpeed, s32 runFrame, s32 stickOnBrakeFrame, s32 brakeFrame, f32 gravityMove, f32 brakeSpeed, s32 counterBorder);
    void appear();
    void reset(const sead::Vector3f& groundNormal);
    void calcInitBrakeOnCounter();
    f32 update();
    f32 updateSkateMove();
    f32 updateNormalMove();
    void updateNormalAndSnap(sead::Vector3f* velocity);
    f32 calcTurnTiltRate() const;
    void calcMoveInput(sead::Vector3f* moveInput, const sead::Vector3f& up);
    bool isActiveSquatBrake() const;
    void updateHillAffect(const sead::Vector3f& groundNormal, const sead::Vector3f& moveInput, bool hasMoveInput);
    f32 calcMaxSpeed(f32 speed) const;
    f32 calcAccelRate(f32 speed) const;
    void updatePoseUpFront(const sead::Vector3f& up, const sead::Vector3f& front, f32 speed);

    const sead::Vector3f& getGroundNormal() const { return mGroundNormal; }
    bool isBrake2D() const { return _64; }
    bool isStopped() const { return mHasStopped; }
    void setIs2D(bool is2D) { _d5 = is2D; }

    void set_c4(bool c4) { _c4 = c4; }
    void setupHackRunFlags() {
        _bb = false;
        _78 = true;
        _b8 = true;
        _bd = true;
    }
    void setPlayerHack(IUsePlayerHack** playerHack) { mHack = playerHack; }
    void setTurnInvalid(bool isInvalid) { _ba = isInvalid; }

private:
    al::LiveActor* mParent;
    const PlayerConst* mPlayerConst;
    const PlayerInput* mPlayerInput;
    const IUsePlayerCollision* mCollision;
    IUsePlayerHack** mHack = nullptr;
    bool mIsSetup = false;
    sead::Vector3f mGroundNormal;
    f32 mMaxSpeed;
    f32 mMinSpeed;
    f32 _40;
    f32 _44;
    s32 mRunFrame;
    s32 mStickOnBrakeFrame;
    s32 mBrakeFrame;
    f32 mGravityMove;
    f32 mBrakeSpeed;
    s32 mCounterBorder;
    s32 _60;
    bool _64;
    IJudge* mJudge;
    f32 _70;
    s32 _74;
    bool _78;
    f32 _7c;
    bool mHasStopped;
    sead::Vector3f _84;
    sead::Vector3f _90;
    bool mIsForceRunCtrlActive;
    f32 _a0;
    bool mAlwaysFalse;
    f32 _a8;
    f32 _ac;
    PlayerActionTurnControl* mTurnCtrl;
    bool _b8;
    bool _b9;
    bool _ba;
    bool _bb;
    bool _bc;
    bool _bd;
    f32 _c0;
    bool _c4;
    sead::Vector3f _c8;
    bool _d4;
    bool _d5;
};

static_assert(sizeof(PlayerActionGroundMoveControl) == 0xD8);
