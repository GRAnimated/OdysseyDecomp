#pragma once

#include <math/seadVector.h>

#include "Player/IUsePlayerCollision.h"

namespace al {
class LiveActor;
}

class CollisionShapeKeeper;
class PlayerCollider;

class YoshiTongueCollider : public IUsePlayerCollision {
public:
    explicit YoshiTongueCollider(al::LiveActor* actor);

    void collide(const sead::Vector3f& position, const sead::Vector3f& direction);
    void resetCollision(const sead::Vector3f& position);

    PlayerCollider* getPlayerCollider() const override { return mPlayerCollider; }

private:
    al::LiveActor* mActor;
    PlayerCollider* mPlayerCollider = nullptr;
    CollisionShapeKeeper* mShapeKeeper = nullptr;
    sead::Vector3f mPosition = sead::Vector3f::zero;
};

static_assert(sizeof(YoshiTongueCollider) == 0x30);
