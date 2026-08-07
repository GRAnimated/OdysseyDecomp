#include "Player/YoshiJudgeStartTongueClingFix.h"

#include "Player/YoshiTongue.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

YoshiJudgeStartTongueClingFix::YoshiJudgeStartTongueClingFix(
    const YoshiTongue* tongue, const IUsePlayerCollision* collision,
    const PlayerWallActionHistory* wallActionHistory)
    : mTongue(tongue), mCollision(collision), mWallActionHistory(wallActionHistory) {}

void YoshiJudgeStartTongueClingFix::update() {
    mIsJudge = false;
    if (mIsCheckWall) {
        if (!rs::isCollidedWall(mCollision))
            return;

        const sead::Vector3f& wallNormal = rs::getCollidedWallNormal(mCollision);
        sead::Vector3f tongueTipPos(0.0f, 0.0f, 0.0f);
        mTongue->calcTongueTipPos(&tongueTipPos);
        if (!rs::judgeEnableWallKeepHistory(mTongue, mWallActionHistory, tongueTipPos,
                                            wallNormal, -200.0f, false))
            return;
        mIsJudge = true;
    }

    if (mIsCheckGround && rs::isCollidedGround(mCollision))
        mIsJudge = true;
}

void YoshiJudgeStartTongueClingFix::reset() {
    mIsJudge = false;
    mIsCheckWall = false;
    mIsCheckGround = false;
}

bool YoshiJudgeStartTongueClingFix::judge() const {
    return mIsJudge;
}
