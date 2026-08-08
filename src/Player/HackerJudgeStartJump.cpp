#include "Player/HackerJudgeStartJump.h"

#include "Util/PlayerHackInputFunction.h"

HackerJudgeStartJump::HackerJudgeStartJump(IUsePlayerHack** hacker) : HackerJudge(hacker) {}

bool HackerJudgeStartJump::judge() const {
    return rs::isTriggerHackJump(*getHacker());
}

void HackerJudgeStartJump::reset() {}

void HackerJudgeStartJump::update() {}

