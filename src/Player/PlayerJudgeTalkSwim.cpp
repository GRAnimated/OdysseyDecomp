#include "Player/PlayerJudgeTalkSwim.h"

#include "Player/PlayerCarryKeeper.h"
#include "Player/PlayerHackKeeper.h"
#include "Player/PlayerStateSwim.h"
#include "Util/PlayerUtil.h"

PlayerJudgeTalkSwim::PlayerJudgeTalkSwim(const PlayerHackKeeper* hackKeeper,
                                         const PlayerCarryKeeper* carryKeeper,
                                         const PlayerStateSwim* stateSwim)
    : mHackKeeper(hackKeeper), mCarryKeeper(carryKeeper), mStateSwim(stateSwim) {}

void PlayerJudgeTalkSwim::reset() {}

void PlayerJudgeTalkSwim::update() {}

bool PlayerJudgeTalkSwim::judge() const {
    if (mHackKeeper->getHackSensor())
        return rs::isPlayerInWater(mHackKeeper->getHack());
    if (mCarryKeeper->isThrowHold())
        return false;
    return mStateSwim->isEnableTalkSwim();
}
