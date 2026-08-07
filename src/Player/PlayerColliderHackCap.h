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
    explicit PlayerColliderHackCap(al::LiveActor*);

    void init();
    void updateCollider(const sead::Vector3f& velocity);
    f32 getColliderRadius() const;
    PlayerCollider* getPlayerCollider() const override;

private:
    al::LiveActor* mPlayer;
    PlayerCollider* mCollider;
    CollisionShapeKeeper* mShapeKeeper;
};

static_assert(sizeof(PlayerColliderHackCap) == 0x20);
