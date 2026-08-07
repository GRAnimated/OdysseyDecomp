#pragma once

#include "Player/IJudge.h"

class IUsePlayerCollision;
class PlayerWallActionHistory;
class YoshiTongue;

class YoshiJudgeStartTongueClingFix : public IJudge {
public:
    YoshiJudgeStartTongueClingFix(const YoshiTongue* tongue,
                                  const IUsePlayerCollision* collision,
                                  const PlayerWallActionHistory* wallActionHistory);

    void reset() override;
    void update() override;
    bool judge() const override;

    void setCheckWall() { mIsCheckWall = true; }
    void setCheckGround() { mIsCheckGround = true; }

private:
    bool mIsJudge = false;
    const YoshiTongue* mTongue;
    const IUsePlayerCollision* mCollision;
    const PlayerWallActionHistory* mWallActionHistory;
    bool mIsCheckWall = false;
    bool mIsCheckGround = false;
};

static_assert(sizeof(YoshiJudgeStartTongueClingFix) == 0x30);
