#include "Player/PlayerJudgeSafetyPointRecovery.h"

#include "Player/PlayerStateAbyss.h"
#include "Util/JudgeUtil.h"

PlayerJudgeSafetyPointRecovery::PlayerJudgeSafetyPointRecovery(IJudge* judge,
                                                               const PlayerStateAbyss* stateAbyss)
    : mJudge(judge), mStateAbyss(stateAbyss) {}

bool PlayerJudgeSafetyPointRecovery::judge() const {
    if (!rs::updateJudgeAndResult(mJudge))
        return false;
    return mStateAbyss->isRecovery();
}
