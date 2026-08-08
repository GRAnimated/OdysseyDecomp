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
    YoshiStateHackWallPopUp(al::LiveActor* actor, IUsePlayerHack** playerHack,
                            const PlayerConst* playerConst, const IUsePlayerCollision* collision,
                            PlayerAnimator* animator);

    void appear() override;
    void kill() override;
    void setupSnap(const al::CollisionParts* collisionParts, const sead::Vector3f& position,
                   const sead::Vector3f& front, const sead::Vector3f& up);
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
