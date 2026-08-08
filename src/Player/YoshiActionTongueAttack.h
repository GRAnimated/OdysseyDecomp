#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
}
class IUsePlayerHack;
class PlayerAnimator;
class YoshiTongue;

class YoshiActionTongueAttack {
public:
    YoshiActionTongueAttack(const al::LiveActor* player, IUsePlayerHack** playerHack,
                            PlayerAnimator* animator, YoshiTongue* tongue);

    bool tryStartAttack(const sead::Vector3f& up);
    void initStartTongue(bool useSubParts);
    void startTongueAnimStart();
    bool tryStartAttackGround(const sead::Vector3f& groundUp);
    bool tryStartAttackFromWall(const sead::Vector3f& up, const sead::Vector3f& wallDir);
    bool tryStartAttackFromWallAir(const sead::Vector3f& up, const sead::Vector3f& wallDir);
    bool tryStartEatFinish();
    void startTongueAnimEnd();
    bool tryEatDirect();
    void startShrink();
    void endShrink();
    void adjustTongueShrinkRestRange(f32 range);
    void update();
    void startTongueAnimLoop();
    void endTongueAnim();
    void syncEatVis();
    bool isUseSubParts() const { return mIsUseSubParts; }

private:
    const al::LiveActor* mPlayer;
    IUsePlayerHack** mPlayerHack;
    PlayerAnimator* mAnimator;
    YoshiTongue* mTongue;
    bool mIsUseSubParts = false;
    bool mIsUpperBodyAnimAttached = false;
    unsigned char _22[6];
};

static_assert(sizeof(YoshiActionTongueAttack) == 0x28);
