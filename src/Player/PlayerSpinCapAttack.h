#pragma once

#include <math/seadVector.h>
#include <prim/seadSafeString.h>

class HackCap;
class PlayerAnimator;
class PlayerConst;
class PlayerCounterAfterCapCatch;
class PlayerInput;
class PlayerJudgePreInputCapThrow;
class PlayerTrigger;

namespace al {
class HitSensor;
}

class PlayerSpinCapAttack {
public:
    PlayerSpinCapAttack(HackCap*, const PlayerConst*, const PlayerTrigger*, const PlayerInput*,
                        const PlayerCounterAfterCapCatch*, const PlayerJudgePreInputCapThrow*);

    void clearAttackInfo();
    void setupAttackInfo();
    void startCapSpinAttack(PlayerAnimator*, const PlayerInput*);
    void startCapSpinAttackAir(PlayerAnimator*, const PlayerInput*);
    void startCapSpinAttackSwim(PlayerAnimator*, const PlayerInput*);
    void startCapThrow(const sead::Vector3f&, const sead::Vector3f&, f32, bool,
                       const sead::Vector3f&);
    void attackSpinMsg(al::HitSensor*, al::HitSensor*);
    bool tryCancelCapState(PlayerAnimator*);
    bool tryStartCapSpinGroundMiss(PlayerAnimator*);
    bool tryStartCapSpinAirMiss(PlayerAnimator*);
    void startSpinSeparate(PlayerAnimator*);
    void startSpinSeparateSwim(PlayerAnimator*);
    void startSpinSeparateSwimSurface(PlayerAnimator*);
    bool isCapSpinAttack() const;
    bool isValidAttackSensor(const PlayerAnimator*) const;
    bool isEnablePlaySpinCapMiss(const PlayerAnimator*) const;
    bool isSeparateSingleSpin() const;
    bool isThrowSwingRightDir() const;
    s32 getThrowFrameGround() const;
    s32 getThrowFrameAir() const;
    s32 getThrowFrameSwim() const;

private:
    HackCap* mHackCap;
    const PlayerConst* mConst;
    const PlayerTrigger* mTrigger;
    const PlayerInput* mInput;
    const PlayerCounterAfterCapCatch* mCounterAfterCapCatch;
    const PlayerJudgePreInputCapThrow* mJudgePreInputCapThrow;
    bool mIsCooperate;
    bool mIsSpinAttack;
    u8 _32[2];
    sead::Vector3f mThrowStartDir;
    s32 mAttackFrame;

    union {
        struct {
            bool isSwingLeft;
            bool isSwingRight;
        };

        u16 mSwingFlags;
    };

    u8 _46[2];
    sead::Vector2f mThrowDir;
    sead::Vector2f mDoubleThrowDir;
    f32 mThrowAngle;
    u8 _5c[4];
    sead::FixedSafeString<64> mSpinAnimName;
    sead::FixedSafeString<64> mSpinMissAnimName;
};

static_assert(sizeof(PlayerSpinCapAttack) == 0x110);
