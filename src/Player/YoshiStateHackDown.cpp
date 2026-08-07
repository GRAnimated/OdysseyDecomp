#include "Player/YoshiStateHackDown.h"

#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerConst.h"

namespace {
NERVE_IMPL(YoshiStateHackDown, Down)
NERVE_IMPL(YoshiStateHackDown, Land)
NERVES_MAKE_NOSTRUCT(YoshiStateHackDown, Down, Land)
}  // namespace

bool YoshiStateHackDown::isLand() const {
    return al::isNerve(this, &Land);
}

bool YoshiStateHackDown::isEnableCancel() const {
    if (!al::isNerve(this, &Land))
        return false;
    return al::isGreaterEqualStep(this, mPlayerConst->getDamageCancelFrame());
}
