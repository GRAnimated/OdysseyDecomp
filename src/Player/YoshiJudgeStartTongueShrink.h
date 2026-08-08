#pragma once

#include <math/seadVector.h>

#include "Util/HackerJudge.h"

namespace al {
class CollisionParts;
class LiveActor;
}

class IUsePlayerCollision;
class IUsePlayerHack;
class PlayerConst;
class YoshiTongue;

class YoshiJudgeStartTongueShrink : public HackerJudge {
public:
    YoshiJudgeStartTongueShrink(IUsePlayerHack** hacker, const al::LiveActor* actor,
                                const IUsePlayerCollision* collision, const YoshiTongue* tongue,
                                const PlayerConst* playerConst);

    void update() override;
    bool updateJudge();
    bool isConnectGround() const;
    bool isConnectWall() const;
    void reset() override;
    bool judge() const override;

    const al::CollisionParts* getCollisionParts() const { return mCollisionParts; }
    const sead::Vector3f& getConnectPos() const { return mConnectPos; }
    const sead::Vector3f& getConnectDir() const { return mConnectDir; }
    const sead::Vector3f& getConnectNormal() const { return mConnectNormal; }
    const sead::Vector3f& getTongueTipPos() const { return mTongueTipPos; }
    bool isWallClimb() const { return mIsWallClimb; }

private:
    const al::LiveActor* mActor;
    const IUsePlayerCollision* mCollision;
    const YoshiTongue* mTongue;
    const PlayerConst* mPlayerConst;
    bool mIsJudge = false;
    bool mIsWallClimb = false;
    bool mIsConnected = false;
    const al::CollisionParts* mCollisionParts = nullptr;
    sead::Vector3f mConnectPos = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mConnectDir;
    sead::Vector3f mConnectNormal = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mTongueTipPos = {0.0f, 0.0f, 0.0f};
};

static_assert(sizeof(YoshiJudgeStartTongueShrink) == 0x70);
