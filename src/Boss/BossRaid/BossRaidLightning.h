#pragma once

#include "Library/LiveActor/LiveActor.h"

namespace al {
template <class T>
class DeriveActorGroup;
}

class BossRaidElectric;
class BossRaidElectricLine;

class BossRaidLightning : public al::LiveActor {
public:
    BossRaidLightning(const char* name);
    void init(const al::ActorInitInfo& info) override;
    void setBulletList(al::DeriveActorGroup<BossRaidElectric>* bulletList);

    void exeSign();
    void exeFall();
    void exeLand();
    void shotBullet();
    void killForce();

    void shot(const sead::Vector3f& pos, bool isRight, f32 radius);

private:
    BossRaidElectricLine* mElectricLine = nullptr;
    sead::Vector3f mShotPos = sead::Vector3f::zero;
    f32 mAngleOffset = 0.0f;
    f32 mShotDir = 1.0f;
    f32 mRadius = 25.0f;
};

static_assert(sizeof(BossRaidLightning) == 0x128);
