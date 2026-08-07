#include "Player/PlayerFunction.h"

#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/Player/PlayerUtil.h"

#include "Player/PlayerActorBase.h"
#include "Player/PlayerInfo.h"
#include "Player/PlayerJudgeDead.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/JudgeUtil.h"

namespace PlayerFunction {

bool isPlayerHitPointOne(const al::LiveActor* actor) {
    GameDataHolderAccessor accessor(actor);
    return GameDataFunction::getPlayerHitPoint(accessor) == 1;
}

bool isPlayerDeadStatus(const al::LiveActor* actor) {
    auto* player = static_cast<PlayerActorBase*>(al::getPlayerActor(actor, 0));
    PlayerInfo* info = player->getPlayerInfo();
    if (info)
        return rs::isJudge(info->getJudgeDead());

    player = static_cast<PlayerActorBase*>(al::getPlayerActor(actor, 0));
    return al::isDead(player);
}

}  // namespace PlayerFunction
