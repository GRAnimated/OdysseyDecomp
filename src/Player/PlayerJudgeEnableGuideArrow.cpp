#include "Player/PlayerJudgeEnableGuideArrow.h"

#include "Player/PlayerPuppet.h"

PlayerJudgeEnableGuideArrow::PlayerJudgeEnableGuideArrow(const PlayerPuppet* puppet)
    : mPuppet(puppet) {}

bool PlayerJudgeEnableGuideArrow::judge() const {
    if (!mPuppet->isBinding())
        return true;
    return mPuppet->isEnableGuideArrow();
}
