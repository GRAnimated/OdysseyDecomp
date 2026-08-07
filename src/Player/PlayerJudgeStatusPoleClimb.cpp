#include "Player/PlayerJudgeStatusPoleClimb.h"

#include "Player/PlayerStatePoleClimb.h"
#include "Util/JudgeUtil.h"

PlayerJudgeStatusPoleClimb::PlayerJudgeStatusPoleClimb(
    const IJudge* judge, const PlayerStatePoleClimb* statePoleClimb)
    : mJudge(judge), mStatePoleClimb(statePoleClimb) {}

bool PlayerJudgeStatusPoleClimb::judge() const {
    if (!rs::isJudge(mJudge))
        return false;
    return mStatePoleClimb->isFormPoleClimb();
}
