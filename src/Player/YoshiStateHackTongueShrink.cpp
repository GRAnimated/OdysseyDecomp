#include "Player/YoshiStateHackTongueShrink.h"

bool YoshiStateHackTongueShrink::isEnableAccelForceRun() const {
    return true;
}

u32 YoshiStateHackTongueShrink::getLoopRunCount() const {
    return mLoopRunCount;
}
