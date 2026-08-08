#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Player/IUsePlayerCollision.h"

namespace al {
class LiveActor;
}
class CollisionShapeKeeper;
class PlayerCollider;

class PlayerColliderHackCap : public IUsePlayerCollision {
public:
    PlayerColliderHackCap(al::LiveActor* player);

    void init();
    void updateCollider(const sead::Vector3f& velocity);
    f32 getColliderRadius() const;
    PlayerCollider* getPlayerCollider() const override;

private:
    al::LiveActor* mPlayer;
    PlayerCollider* mCollider = nullptr;
    CollisionShapeKeeper* mShapeKeeper = nullptr;
};

static_assert(sizeof(PlayerColliderHackCap) == 0x20);
