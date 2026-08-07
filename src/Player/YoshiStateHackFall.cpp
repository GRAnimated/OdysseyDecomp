#include "Player/YoshiStateHackFall.h"

#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
NERVE_IMPL(YoshiStateHackFall, Fall)
NERVES_MAKE_NOSTRUCT(YoshiStateHackFall, Fall)
}  // namespace

void YoshiStateHackFall::appear() {
    HackerStateBase::appear();
    al::setNerve(this, &Fall);
}
