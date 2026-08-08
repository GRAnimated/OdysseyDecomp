#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Util/HackerJudge.h"

namespace al {
class CollisionParts;
class LiveActor;
}
class IUsePlayerCollision;
class IUsePlayerHack;
class PlayerConst;
class PlayerWallActionHistory;

class YoshiJudgeWallCling : public HackerJudge {
public:
    YoshiJudgeWallCling(IUsePlayerHack** hacker, const al::LiveActor* player,
                        const IUsePlayerCollision* collision,
                        const PlayerWallActionHistory* wallActionHistory,
                        const PlayerConst* playerConst);

    void update() override;
    void reset() override;
    bool judge() const override;

    const al::CollisionParts* getCollisionParts() const { return mCollisionParts; }
    const sead::Vector3f& getWallPos() const { return mWallPos; }
    const sead::Vector3f& getWallNormal() const { return mWallNormal; }
    const sead::Vector3f& getPopUpDir() const { return mPopUpDir; }
    bool isWallPopUp() const { return mIsWallPopUp; }
    bool isDamageWall() const { return mIsDamageWall; }

private:
    const al::LiveActor* mPlayer;
    const IUsePlayerCollision* mCollision;
    const PlayerWallActionHistory* mWallActionHistory;
    const PlayerConst* mConst;
    bool mIsDamageWall;
    bool mIsJudge;
    bool mIsWallPopUp;
    u8 _33[5];
    const al::CollisionParts* mCollisionParts;
    sead::Vector3f mWallPos;
    sead::Vector3f mWallNormal;
    sead::Vector3f mPopUpDir;
    u8 _64[4];
};

static_assert(sizeof(YoshiJudgeWallCling) == 0x68);
