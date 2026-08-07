#include "Player/PlayerJudgeCapCatchPop.h"

#include "Player/IPlayerModelChanger.h"
#include "Player/PlayerCapActionHistory.h"
#include "Player/PlayerCounterAfterCapCatch.h"
#include "Player/PlayerInput.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerUtil.h"

PlayerJudgeCapCatchPop::PlayerJudgeCapCatchPop(
    const al::LiveActor* player, const PlayerInput* input, const IUsePlayerCollision* collision,
    const IPlayerModelChanger* modelChanger, const PlayerCapActionHistory* capActionHistory,
    const PlayerCounterAfterCapCatch* counterAfterCapCatch)
    : mPlayer(player), mInput(input), mCollision(collision), mModelChanger(modelChanger),
      mCapActionHistory(capActionHistory), mCounterAfterCapCatch(counterAfterCapCatch) {
    return;
}

bool PlayerJudgeCapCatchPop::judge() const {
    if (mModelChanger->is2DModel() || !mCounterAfterCapCatch->isCapCatch() ||
        !mCapActionHistory->isCapCatchPopEnabled() || rs::isCollidedGround(mCollision) ||
        !rs::isEquipCapCatched(mPlayer))
        return false;

    return mInput->isTriggerJump();
}
