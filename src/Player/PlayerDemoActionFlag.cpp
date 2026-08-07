#include "Player/PlayerDemoActionFlag.h"

PlayerDemoActionFlag::PlayerDemoActionFlag() {
    reset();
}

void PlayerDemoActionFlag::reset() {
    mIsDemoAction = false;
    mFlags = 0;
    actionData = 0;
    actionParam = 0;
}
