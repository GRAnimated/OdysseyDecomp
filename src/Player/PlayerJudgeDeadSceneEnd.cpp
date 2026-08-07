#include "Player/PlayerJudgeDeadSceneEnd.h"

#include "Library/LiveActor/ActorFlagFunction.h"

#include "Util/JudgeUtil.h"

PlayerJudgeDeadSceneEnd::PlayerJudgeDeadSceneEnd(const al::LiveActor* player, const IJudge* judge)
    : mPlayer(player), mJudge(judge) {}

bool PlayerJudgeDeadSceneEnd::judge() const {
    return rs::isJudge(mJudge) || al::isDead(mPlayer);
}
