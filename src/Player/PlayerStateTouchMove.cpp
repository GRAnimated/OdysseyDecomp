#include "Player/PlayerStateTouchMove.h"

#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
NERVE_IMPL(PlayerStateTouchMove, Jump)
NERVES_MAKE_STRUCT(PlayerStateTouchMove, Jump)
}  // namespace

PlayerStateTouchMove::PlayerStateTouchMove(al::LiveActor* player, const TouchTargetKeeper*)
    : al::ActorStateBase("", player) {
    initNerve(&NrvPlayerStateTouchMove.Jump, 0);
}

void PlayerStateTouchMove::appear() {
    al::NerveStateBase::appear();
    al::setNerve(this, &NrvPlayerStateTouchMove.Jump);
}

void PlayerStateTouchMove::exeJump() {}

PlayerStateTouchMove::~PlayerStateTouchMove() = default;
