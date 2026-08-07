#include "Player/YoshiStateHackWallJump.h"

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionAirMoveControl.h"

namespace {
class PlayerActionAirMoveControlOverlay {
public:
    u8 _0[0x20];
    IUsePlayerHack** mPlayerHack;
};

NERVE_IMPL(YoshiStateHackWallJump, Jump)
NERVES_MAKE_NOSTRUCT(YoshiStateHackWallJump, Jump)
}  // namespace

YoshiStateHackWallJump::YoshiStateHackWallJump(
    al::LiveActor* actor, IUsePlayerHack** playerHack, const PlayerConst* playerConst,
    const IUsePlayerCollision* collision, const PlayerTrigger* trigger, const YoshiTongue* tongue,
    PlayerAnimator* animator)
    : HackerStateBase("壁ジャンプ", actor, playerHack), mPlayerConst(playerConst),
      mCollision(collision), mTrigger(trigger), mTongue(tongue), mAnimator(animator),
      mAirMoveControl(nullptr) {
    mAirMoveControl = new PlayerActionAirMoveControl(actor, playerConst, nullptr, collision, false);
    reinterpret_cast<PlayerActionAirMoveControlOverlay*>(mAirMoveControl)->mPlayerHack = playerHack;
    initNerve(&Jump, 0);
}

void YoshiStateHackWallJump::appear() {
    HackerStateBase::appear();
    al::setNerve(this, &Jump);
}
