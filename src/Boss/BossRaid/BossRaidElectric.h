#pragma once

#include "Library/LiveActor/LiveActor.h"

namespace al {
template <class T>
class DeriveActorGroup;
}

class BossRaidElectric : public al::LiveActor {
public:
    BossRaidElectric(const char* name);
    void init(const al::ActorInitInfo& info) override;
    void makeActorDead() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;

    void setPrevBullet(BossRaidElectric* bullet);
    void setNextBullet(BossRaidElectric* bullet);

    static void shot(BossRaidElectric* self, const sead::Vector3f& pos, const sead::Vector3f& dir,
                     BossRaidElectric* prevBullet,
                     al::DeriveActorGroup<BossRaidElectric>* actorGroup);
    void updatePosition();
    void exeWait();
    void updateAnimAndJoint();
    bool isAirAll() const;
    void exeDisappear();
    void exeHide();
    bool isHideAll() const;
    bool isHide() const;
    void updateEffectScale(f32 scale);
    void calcNearPos(sead::Vector3f* outPos, const sead::Vector3f& targetPos) const;
};
