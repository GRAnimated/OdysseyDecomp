#include "Player/YoshiStateHackRun.h"

void YoshiStateHackRun::invalidateTurn() {
    mTurnControl->turnInvalid = true;
}

void YoshiStateHackRun::validateTurn() {
    mTurnControl->turnInvalid = false;
}
