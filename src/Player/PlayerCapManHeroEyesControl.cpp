#include "Player/PlayerCapManHeroEyesControl.h"

void PlayerCapManHeroEyesControl::update() {
    updateNerve();
}

al::LiveActor* PlayerCapManHeroEyesControl::getPuppetEye() const {
    return mPuppetEye;
}

void PlayerCapManHeroEyesControl::exeDemo() {}
