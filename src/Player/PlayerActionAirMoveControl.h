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
    PlayerActionAirMoveControl(al::LiveActor*, const PlayerConst*, const PlayerInput*,
                               const IUsePlayerCollision*, bool);
    void setup(f32, f32, s32, f32, f32, s32, f32);
    void setupTurn(f32, f32, f32, f32, s32, s32, s32);
    void setExtendFrame(s32);
    void setupCollideWallScaleVelocity(f32, f32, f32);
    void verticalizeStartMoveDir(const sead::Vector3f&);
    void update();
    bool isHoldJumpExtend() const;
    void calcMoveInput(sead::Vector3f*, const sead::Vector3f&) const;
    const sead::Vector3f& getStartMoveDir() const { return mStartMoveDir; }
    void setIsPlayer2D(bool value) { _33 = value; }

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
