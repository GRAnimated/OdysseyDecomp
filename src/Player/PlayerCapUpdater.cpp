#include "Player/PlayerCapUpdater.h"

#include "Library/Execute/ExecuteUtil.h"

#include "Player/HackCap.h"
#include "Player/PlayerModelChangerHakoniwa.h"

PlayerCapUpdater::PlayerCapUpdater(HackCap* hackCap, PlayerModelChangerHakoniwa* modelChanger)
    : mHackCap(hackCap), mModelChanger(modelChanger) {}

void PlayerCapUpdater::init(al::ExecuteDirector* executeDirector) {
    al::registerExecutorUser(this, executeDirector, "帽子装着位置更新");
}

void PlayerCapUpdater::execute() {
    mHackCap->updateCapPose();
    mModelChanger->syncModelBoneVisibility();
}
