#include "Player/PlayerJudgeCapCatch.h"

#include "Player/PlayerCounterAfterCapCatch.h"
#include "Util/PlayerUtil.h"

PlayerJudgeCapCatch::PlayerJudgeCapCatch(
    const al::LiveActor* player, const PlayerCounterAfterCapCatch* counterAfterCapCatch)
    : mPlayer(player), mCounterAfterCapCatch(counterAfterCapCatch) {}

bool PlayerJudgeCapCatch::judge() const {
    if (!mCounterAfterCapCatch->isCapCatch())
        return false;
    return rs::isEquipCapCatched(mPlayer);
}
