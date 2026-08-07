#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

namespace al {
class LiveActor;
class MeInfo;
}
class BgmAnimeSynchronizer;
class BgmSyncTargetActionInfo;
class HackCap;
class PlayerAnimator;
class PlayerExternalVelocity;
class PlayerModelChangerHakoniwa;

class PlayerSeCtrl {
public:
    PlayerSeCtrl(const al::LiveActor* player, const PlayerAnimator* animator,
                 const HackCap* hackCap, const PlayerModelChangerHakoniwa* modelChanger,
                 const al::LiveActor* modelActor,
                 const PlayerExternalVelocity* externalVelocity);

    void update();
    bool isPassAnimFrame(s32 index) const;

private:
    const al::LiveActor* mPlayer;
    const PlayerAnimator* mAnimator;
    const PlayerExternalVelocity* mExternalVelocity;
    f32 _18;
    u8 _1c[4];
    const HackCap* mHackCap;
    sead::FixedSafeString<256> mShakeNoiseName;
    const PlayerModelChangerHakoniwa* mModelChanger;
    al::MeInfo* mMeInfo;
    s32 _150;
    f32 _154;
    s32 _158;
    s32 _15c;
    bool _160;
    bool _161;
    bool _162;
    u8 _163;
    f32 _164;
    bool _168;
    u8 _169[3];
    s32 _16c;
    f32 _170;
    f32 _174;
    BgmSyncTargetActionInfo* mBgmSyncTargetActionInfo;
    BgmAnimeSynchronizer* mBgmAnimeSynchronizer;
    bool _188;
    u8 _189[3];
    s32 _18c;
    s32 _190;
    u8 _194[4];
};

static_assert(sizeof(PlayerSeCtrl) == 0x198);
