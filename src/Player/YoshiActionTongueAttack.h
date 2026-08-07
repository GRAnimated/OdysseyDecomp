#pragma once

#include <basis/seadTypes.h>

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

    void initStartTongue(bool useSubParts);
    void syncEatVis();
    void startShrink();
    void endShrink();
    void adjustTongueShrinkRestRange(f32 range);

private:
    const al::LiveActor* mPlayer;
    IUsePlayerHack** mPlayerHack;
    PlayerAnimator* mAnimator;
    YoshiTongue* mTongue;
    bool mIsUseSubParts;
    bool mIsUpperBodyAnimAttached;
    unsigned char _22[6];
};

static_assert(sizeof(YoshiActionTongueAttack) == 0x28);
