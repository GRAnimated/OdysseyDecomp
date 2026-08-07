#pragma once

class PlayerCollider;

class IUsePlayerCollision {
public:
    virtual PlayerCollider* getPlayerCollider() const = 0;
};

static_assert(sizeof(IUsePlayerCollision) == 0x8);
