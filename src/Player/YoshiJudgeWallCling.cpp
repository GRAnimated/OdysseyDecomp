#include "Player/YoshiJudgeWallCling.h"

YoshiJudgeWallCling::YoshiJudgeWallCling(IUsePlayerHack** hacker, const al::LiveActor* player,
                                         const IUsePlayerCollision* collision,
                                         const PlayerWallActionHistory* wallActionHistory,
                                         const PlayerConst* playerConst)
    : HackerJudge(hacker), mPlayer(player), mCollision(collision),
      mWallActionHistory(wallActionHistory), mConst(playerConst), mIsDamageWall(false),
      mIsJudge(false), mIsWallPopUp(false), mCollisionParts(nullptr),
      mWallPos{0.0f, 0.0f, 0.0f}, mWallNormal{0.0f, 0.0f, 0.0f},
      mPopUpDir{0.0f, 0.0f, 0.0f} {}

void YoshiJudgeWallCling::reset() {
    mIsDamageWall = false;
}

bool YoshiJudgeWallCling::judge() const {
    return mIsJudge;
}
