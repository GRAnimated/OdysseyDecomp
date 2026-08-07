#include "Player/YoshiActionTongueAttack.h"

#include "Player/PlayerAnimator.h"
#include "Player/YoshiTongue.h"

YoshiActionTongueAttack::YoshiActionTongueAttack(const al::LiveActor* player,
                                                 IUsePlayerHack** playerHack,
                                                 PlayerAnimator* animator, YoshiTongue* tongue)
    : mPlayer(player), mPlayerHack(playerHack), mAnimator(animator), mTongue(tongue),
      mIsUseSubParts(false), mIsUpperBodyAnimAttached(false) {}

void YoshiActionTongueAttack::initStartTongue(bool useSubParts) {
    mIsUseSubParts = useSubParts;
}

void YoshiActionTongueAttack::syncEatVis() {
    if (!mIsUseSubParts && mIsUpperBodyAnimAttached)
        mAnimator->overwrideYoshiEatVis();
}

void YoshiActionTongueAttack::startShrink() {
    mTongue->startShrink();
}

void YoshiActionTongueAttack::endShrink() {
    if (mTongue->isShrinkMove())
        mTongue->endShrink();
}

void YoshiActionTongueAttack::adjustTongueShrinkRestRange(f32 range) {
    mTongue->adjustShrinkRestRange(range);
}
