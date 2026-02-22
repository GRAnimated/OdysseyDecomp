#pragma once

#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
template <class T>
class DeriveActorGroup;
struct ActorInitInfo;
}  // namespace al

class BossRaid;
class BossRaidElectric;
class BossRaidElectricLine;

class BossRaidStateGroundAttack : public al::NerveStateBase {
public:
    BossRaidStateGroundAttack(BossRaid* boss, const al::ActorInitInfo& info);
    ~BossRaidStateGroundAttack() override;

    void appear() override;

    void exeFallSign();
    void exeFall();
    void exeAttack();
    s32 getGroundAttackTime() const;
    void shotBulletLv1();
    void shotBulletLv2();
    void shotBulletLv3();
    void shotBulletLv4();
    void shotBulletLv5();
    void shotBulletLv6();
    void exeAttackEnd();
    bool isElectric() const;
    void updateBullet();
    void killBulletAll();

private:
    BossRaid* mBossRaid = nullptr;
    al::DeriveActorGroup<BossRaidElectric>* mElectricList = nullptr;
    al::DeriveActorGroup<BossRaidElectricLine>* mElectricLineList = nullptr;
    sead::Vector3f mUpDir = sead::Vector3f::ey;
    sead::Vector3f mFrontDir = sead::Vector3f::ez;
};

static_assert(sizeof(BossRaidStateGroundAttack) == 0x48);
