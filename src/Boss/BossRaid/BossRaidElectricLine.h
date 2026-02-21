#pragma once

#include "Library/LiveActor/LiveActor.h"

namespace al {
template <class T>
class DeriveActorGroup;
}

class BossRaidElectric;

class BossRaidElectricLine : public al::LiveActor {
public:
    BossRaidElectricLine(const char* name);
    void init(const al::ActorInitInfo& info) override;
    void setBulletList(al::DeriveActorGroup<BossRaidElectric>* bulletList);
    void shot(const sead::Vector3f& pos, const sead::Vector3f& dir);
    void killForce();

private:
    void* _108 = nullptr;
    void* _110 = nullptr;
    void* _118 = nullptr;
};

static_assert(sizeof(BossRaidElectricLine) == 0x120);
