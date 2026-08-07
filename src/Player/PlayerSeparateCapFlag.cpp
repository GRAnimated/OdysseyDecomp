#include "Player/PlayerSeparateCapFlag.h"

PlayerSeparateCapFlag::PlayerSeparateCapFlag()
    : separateCap(false), separateCapLocal(false), puppetable(false) {
    mSeparateCapLocalOffset.set(0.0f, 0.0f, 0.0f);
}
