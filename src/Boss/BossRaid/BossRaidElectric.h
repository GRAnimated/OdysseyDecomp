#pragma once

#include <math/seadVector.h>

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

private:
    al::DeriveActorGroup<BossRaidElectric>* mActorGroup = nullptr;  // active bullet list
    BossRaidElectric* mNextBullet = nullptr;
    BossRaidElectric* mPrevBullet = nullptr;
    sead::Vector3f mFrontDir = sead::Vector3f::ez;  // segment direction, normalised
    sead::Vector3f _12c = sead::Vector3f::zero;      // center position of segment
    sead::Vector3f mMoveDir = sead::Vector3f::zero; // per-frame translation delta
    f32 _144 = 0.0f;                                // segment length offset (length - 500)
    bool _148 = true;                               // true while bullet is in the active area
};

static_assert(sizeof(BossRaidElectric) == 0x150);
