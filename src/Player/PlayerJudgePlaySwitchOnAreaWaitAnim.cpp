#include "Player/PlayerJudgePlaySwitchOnAreaWaitAnim.h"

#include "Player/PlayerStateWait.h"

PlayerJudgePlaySwitchOnAreaWaitAnim::PlayerJudgePlaySwitchOnAreaWaitAnim(
    const PlayerStateWait* stateWait)
    : mStateWait(stateWait) {}

bool PlayerJudgePlaySwitchOnAreaWaitAnim::judge() const {
    return mStateWait->isPlaySwitchOnAreaAnim();
}
