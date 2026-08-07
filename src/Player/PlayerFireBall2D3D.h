#pragma once

#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

class Collider2D3D;

class PlayerFireBall2D3D : public al::LiveActor {
public:
    PlayerFireBall2D3D(const al::LiveActor* player);

    void init(const al::ActorInitInfo& info) override;
    void shoot(bool is2D);
    void exeMove();
    void boundWall(const sead::Vector3f& normal, const sead::Vector3f& position);
    void applyGravity();
    void turn(const sead::Vector3f& velocity);
    void exeDead();
    void updateCollider() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                    al::HitSensor* self) override;

private:
    const al::LiveActor* mPlayer;
    bool mIsBound;
    bool mIsIn2D;
    bool mIsShoot2D;
    u8 _113;
    sead::Vector3f mAreaUp;
    sead::Vector3f mAreaLockDir;
    u8 _12c[4];
    Collider2D3D* mCollider2D3D;
};

static_assert(sizeof(PlayerFireBall2D3D) == 0x138);
