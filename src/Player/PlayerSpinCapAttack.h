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
    PlayerSpinCapAttack(HackCap* hackCap, const PlayerConst* playerConst,
                        const PlayerTrigger* trigger, const PlayerInput* input,
                        const PlayerCounterAfterCapCatch* counterAfterCapCatch,
                        const PlayerJudgePreInputCapThrow* judgePreInputCapThrow);

    void clearAttackInfo();
    void setupAttackInfo();
    void startCapSpinAttack(PlayerAnimator* animator, const PlayerInput*);
    void startCapSpinAttackAir(PlayerAnimator* animator, const PlayerInput*);
    void startCapSpinAttackSwim(PlayerAnimator* animator, const PlayerInput*);
    void startSpinSeparate(PlayerAnimator* animator);
    void startSpinSeparateSwim(PlayerAnimator* animator);
    void startSpinSeparateSwimSurface(PlayerAnimator* animator);
    void startCapThrow(const sead::Vector3f& startPos, const sead::Vector3f& throwDir, f32 power,
                       bool isAppend, const sead::Vector3f& targetPos);
    void attackSpinMsg(al::HitSensor* self, al::HitSensor* other);
    bool tryCancelCapState(PlayerAnimator* animator);
    bool tryStartCapSpinGroundMiss(PlayerAnimator* animator);
    bool tryStartCapSpinAirMiss(PlayerAnimator* animator);
    bool isCapSpinAttack() const;
    bool isValidAttackSensor(const PlayerAnimator* animator) const;
    bool isEnablePlaySpinCapMiss(const PlayerAnimator* animator) const;
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
