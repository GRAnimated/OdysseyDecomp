#include "Player/YoshiJudgeStartTongueShrink.h"

#include "Player/PlayerConst.h"
#include "Player/YoshiTongue.h"
#include "Util/ObjUtil.h"

YoshiJudgeStartTongueShrink::YoshiJudgeStartTongueShrink(
    IUsePlayerHack** hacker, const al::LiveActor* actor, const IUsePlayerCollision* collision,
    const YoshiTongue* tongue, const PlayerConst* playerConst)
    : HackerJudge(hacker), mActor(actor), mCollision(collision), mTongue(tongue),
      mPlayerConst(playerConst) {}

void YoshiJudgeStartTongueShrink::update() {
    mIsWallClimb = false;
    mIsConnected = false;
    mIsJudge = updateJudge();
}

bool YoshiJudgeStartTongueShrink::updateJudge() {
    if (!mTongue->isEnableShrinkStart())
        return false;

    sead::Vector3f connectPos(0.0f, 0.0f, 0.0f);
    sead::Vector3f connectNormal(0.0f, 0.0f, 0.0f);
    sead::Vector3f direction(0.0f, 0.0f, 0.0f);
    const al::CollisionParts* collisionParts = nullptr;
    if (!mTongue->tryCalcTongueConnect(&collisionParts, &connectPos, &connectNormal, &direction,
                                       &mTongueTipPos))
        return false;

    mIsWallClimb = false;
    mIsConnected = true;
    if (mTongue->isConnectWall()) {
        mIsWallClimb = rs::findWallClimbPos(
            &mCollisionParts, &mConnectPos, &mConnectNormal, mActor, -mTongueTipPos, connectPos,
            mTongueTipPos, mPlayerConst->getWallKeepDegree(), 60.0f, 300.0f, 0.0f, -1.0f,
            mPlayerConst->getCollisionRadius(), mPlayerConst->getCollisionRadiusStand());
        if (!mIsWallClimb) {
            mCollisionParts = collisionParts;
            mConnectPos = connectPos;
            mConnectDir = direction;
            mConnectNormal = connectNormal;
        }
    } else if (mTongue->isConnectGround()) {
        mCollisionParts = collisionParts;
        mConnectPos = connectPos;
        mConnectDir = connectNormal;
        mConnectNormal = direction;
    }
    return true;
}

bool YoshiJudgeStartTongueShrink::isConnectGround() const {
    return mTongue->isConnectGround();
}

bool YoshiJudgeStartTongueShrink::isConnectWall() const {
    return mTongue->isConnectWall();
}

void YoshiJudgeStartTongueShrink::reset() {
    mIsJudge = false;
}

bool YoshiJudgeStartTongueShrink::judge() const {
    return mIsJudge;
}
