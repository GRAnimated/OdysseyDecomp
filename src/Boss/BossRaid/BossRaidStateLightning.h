#pragma once

#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
template <class T>
class DeriveActorGroup;
struct ActorInitInfo;
}  // namespace al

class BossRaid;
class BossRaidWheel;

class BossRaidStateLightning : public al::NerveStateBase {
public:
    BossRaidStateLightning(BossRaid* boss, const al::ActorInitInfo& info);
    ~BossRaidStateLightning() override;

    void appear() override;

    void exeLightning();
    s32 getLightningInterval() const;
    void exeLightningEnd();
    void killWheel();

private:
    BossRaid* mBossRaid = nullptr;
    al::DeriveActorGroup<BossRaidWheel>* mWheelList = nullptr;
    sead::Vector3f mUpDir = sead::Vector3f::ey;
    sead::Vector3f mFrontDir = sead::Vector3f::ez;
    sead::Vector3f mLocalOffset = sead::Vector3f::zero;
    s32 mShotIndex = 0;
};

static_assert(sizeof(BossRaidStateLightning) == 0x50);
