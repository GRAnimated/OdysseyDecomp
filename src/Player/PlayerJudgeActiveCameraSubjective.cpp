#include "Player/PlayerJudgeActiveCameraSubjective.h"

#include "Player/PlayerStateCameraSubjective.h"

PlayerJudgeActiveCameraSubjective::PlayerJudgeActiveCameraSubjective(
    const PlayerStateCameraSubjective* stateCameraSubjective)
    : mStateCameraSubjective(stateCameraSubjective) {}

bool PlayerJudgeActiveCameraSubjective::judge() const {
    return mStateCameraSubjective->isStateActiveCamera();
}
