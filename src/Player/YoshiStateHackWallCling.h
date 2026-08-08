#pragma once

#include <math/seadVector.h>

#include "Player/HackerStateBase.h"

namespace al {
class CollisionParts;
class CollisionPartsConnector;
}

class IUsePlayerCollision;
class PlayerAnimator;
class PlayerConst;

class YoshiStateHackWallCling : public HackerStateBase {
public:
    YoshiStateHackWallCling(al::LiveActor* actor, IUsePlayerHack** playerHack,
                            const PlayerConst* playerConst, const IUsePlayerCollision* collision,
                            PlayerAnimator* animator);

    void appear() override;
    void setup(const al::CollisionParts* collisionParts, const sead::Vector3f& position,
               const sead::Vector3f& normal);
    void exeCling();
    ~YoshiStateHackWallCling() override;

private:
    const PlayerConst* mPlayerConst;
    const IUsePlayerCollision* mCollision;
    PlayerAnimator* mAnimator;
    const al::CollisionParts* mCollisionParts = nullptr;
    sead::Vector3f mPosition = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mNormal = {0.0f, 0.0f, 0.0f};
    al::CollisionPartsConnector* mConnector = nullptr;
};

static_assert(sizeof(YoshiStateHackWallCling) == 0x68);
