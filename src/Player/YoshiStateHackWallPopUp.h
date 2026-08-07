#pragma once

#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Player/HackerStateBase.h"

namespace al {
class CollisionParts;
class CollisionPartsConnector;
class ParabolicPath;
}

class IUsePlayerCollision;
class PlayerAnimator;
class PlayerConst;

class YoshiStateHackWallPopUp : public HackerStateBase {
public:
    YoshiStateHackWallPopUp(al::LiveActor*, IUsePlayerHack**, const PlayerConst*,
                            const IUsePlayerCollision*, PlayerAnimator*);

    void appear() override;
    void kill() override;
    void setupSnap(const al::CollisionParts*, const sead::Vector3f&, const sead::Vector3f&,
                   const sead::Vector3f&);
    void exePopUp();

private:
    const PlayerConst* mPlayerConst;
    const IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    sead::Vector3f mStartTrans;
    sead::Quatf mStartQuat;
    al::CollisionParts* mCollisionParts;
    al::CollisionPartsConnector* mConnector;
    sead::Matrix34f mSnapMtx;
    sead::Matrix34f mConnectedMtx;
    al::ParabolicPath* mPath;
};

static_assert(sizeof(YoshiStateHackWallPopUp) == 0xD8);
