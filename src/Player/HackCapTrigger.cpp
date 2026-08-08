#include "Player/HackCapTrigger.h"

HackCapTrigger::HackCapTrigger() {}

void HackCapTrigger::clearAfterMovemetTrigger() {
    mAfterMovementTriggers = 0;
}

void HackCapTrigger::set(EAfterMovementTrigger trigger) {
    mAfterMovementTriggers |= 1u << trigger;
}

bool HackCapTrigger::isOn(EAfterMovementTrigger trigger) const {
    const u32 mask = 1u << trigger;
    return (mAfterMovementTriggers & mask) != 0;
}
