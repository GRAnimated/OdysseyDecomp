#include "Player/PlayerJudgeEnablePeachAmiibo.h"

#include "Player/PlayerStateDamageFire.h"

PlayerJudgeEnablePeachAmiibo::PlayerJudgeEnablePeachAmiibo(
    const PlayerStateDamageFire* stateDamageFire)
    : mStateDamageFire(stateDamageFire) {}

bool PlayerJudgeEnablePeachAmiibo::judge() const {
    return mStateDamageFire->isEnablePeachAmiibo();
}
