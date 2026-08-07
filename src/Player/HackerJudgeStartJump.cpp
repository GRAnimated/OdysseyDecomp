#include "Player/HackerJudgeStartJump.h"

#include "Util/PlayerHackInputFunction.h"

HackerJudgeStartJump::HackerJudgeStartJump(IUsePlayerHack** hacker) : HackerJudge(hacker) {}

void HackerJudgeStartJump::reset() {}

void HackerJudgeStartJump::update() {}

bool HackerJudgeStartJump::judge() const {
    return rs::isTriggerHackJump(*getHacker());
}
