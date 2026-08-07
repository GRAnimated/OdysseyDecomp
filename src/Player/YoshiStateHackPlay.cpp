#include "Player/YoshiStateHackPlay.h"

#include "Player/PlayerEyeSensorHitHolder.h"
#include "Player/YoshiTongue.h"

void YoshiStateHackPlay::updatePrevMovement() {
    mEyeSensorHitHolder->clear();
}

void YoshiStateHackPlay::prepareEndHack() {
    mTongue->endHack();
}
