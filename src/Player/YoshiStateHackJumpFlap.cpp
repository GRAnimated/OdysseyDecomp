#include "Player/YoshiStateHackJumpFlap.h"

#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
NERVE_IMPL(YoshiStateHackJumpFlap, Rise)
NERVE_IMPL(YoshiStateHackJumpFlap, Fall)
NERVES_MAKE_STRUCT(YoshiStateHackJumpFlap, Rise, Fall)
}  // namespace

void YoshiStateHackJumpFlap::appear() {
    HackerStateBase::appear();
    al::setNerve(this, &NrvYoshiStateHackJumpFlap.Rise);
}

void YoshiStateHackJumpFlap::kill() {
    if (*mPlayerHack)
        al::startMtpAnim(mHackActor, "HackOn");
    HackerStateBase::kill();
}
