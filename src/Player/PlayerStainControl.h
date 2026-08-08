#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <gfx/seadColor.h>
#include <prim/seadSafeString.h>

namespace al {
class LiveActor;
}

class HackCap;
class PlayerEffect;
class PlayerModelChangerHakoniwa;

struct PlayerStainControlParam {
    s32 type;
    sead::Color4f color;
    f32 setupRate;
    s32 _18;
    s32 _1c;
    s32 _20;
    s32 blizzardCount;
};


class PlayerStainControl {
public:
    enum StainTypeValue {
        StainTypeNone = 0,
        StainTypePoison = 1,
        StainTypeFire = 2,
        StainTypeIce = 3,
        StainTypeSandDesert = 4,
        StainTypeSand = 5,
        StainTypeSandMoon = 6,
    };

    class StainType {
    public:
        StainType() : mValue(StainTypeNone) {}

        StainType(StainTypeValue value) : mValue(value) {}

        StainType(s32 value) : mValue(value) {}

        const StainType& operator=(StainTypeValue value) {
            mValue = value;
            return *this;
        }

        bool operator==(StainTypeValue value) const { return mValue == value; }

        bool operator!=(StainTypeValue value) const { return mValue != value; }

        operator s32() const volatile { return mValue; }

    private:
        s32 mValue;
    };

    PlayerStainControl(const al::LiveActor*, al::LiveActor*, const PlayerModelChangerHakoniwa*,
                       const HackCap*, PlayerEffect*);

    void recordExplosion();
    void recordDamageFire();
    void recordDamageFireDead();
    void recordBlackSmoke();
    void recordPoison();
    void recordIceWater();
    void recordBlizzard();
    void recordSandMove(const char* materialCode);
    bool isEnableLowPriorityStain() const;
    u64 getSandType(const char* materialCode) const;
    void recordSnowMove(const char* materialCode);
    void recordSnowBySensor();
    void recordSandHeavyLand(const char* materialCode);
    void clearCurrentStain(bool emitReaction);
    void recordSandMoonBySensor();
    void recordInWater();
    void recordInWet();
    void noticeStartHack();
    void tryDeleteStainEffect(StainType stainType, f32 rate);
    void noticeEndHack();
    void clearStain();
    void noticeMainShineGet();
    void update();
    void clearStainRequest();
    void tryEmitStainEffect(StainType stainType);
    bool isEnableValidateStain() const;
    bool isEnableInvalidateStain() const;
    void tryEmitClearStainEffect(StainType stainType, f32 rate);

private:
    const al::LiveActor* mPlayer;
    al::LiveActor* mModelActor;
    const PlayerModelChangerHakoniwa* mModelChanger;
    const HackCap* mHackCap;
    PlayerEffect* mEffect;
    sead::PtrArray<sead::SafeStringBase<char>> mStainParts;
    StainType mCurrentStainType;
    u8 _3c[4];
    sead::PtrArray<PlayerStainControlParam> mStainParams;
    StainType mRequestStainType;
    bool mIsInWater;
    bool mIsInWet;
    bool mIsBlizzard;
    u8 _57;
    f32 mBlizzardValueA;
    s32 mBlizzardValueB;
    s32 mBlizzardCounter;
    f32 mBlizzardRate;
    f32 mRequestRate;
    f32 mCurrentRate;
    s32 _70;
    s32 _74;
    s32 _78;
    s32 _7c;
    bool mIsStainInvalid;
    bool mIsHack;
    u8 _82[6];
};

static_assert(sizeof(PlayerStainControl) == 0x88);
