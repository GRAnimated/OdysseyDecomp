#include "Player/YoshiJudgeWallCling.h"

void YoshiJudgeWallCling::reset() {
    mIsDamageWall = false;
}

bool YoshiJudgeWallCling::judge() const {
    return mIsJudge;
}
