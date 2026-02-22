#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
template <class T>
class DeriveActorGroup;
class JointAimInfo;
class LiveActor;
struct ActorInitInfo;
}  // namespace al

class BossRaid;
class BossRaidWheel;

class BossRaidStateBreathAttack : public al::NerveStateBase {
public:
    BossRaidStateBreathAttack(BossRaid* boss, const al::ActorInitInfo& info);
    ~BossRaidStateBreathAttack() override;

    void appear() override;
    void kill() override;
    void control() override;

    void setAimRate(f32 rate);
    void updateAttackSign();
    void updateAttackSignLv1();
    void updateAttackSignLv2();
    void updateAttackSignLv3();
    bool updateAttack();
    bool updateAttackLv1();
    bool updateAttackLv2();
    bool updateAttackLv3();
    void limitBreathPos(sead::Vector3f* result, const sead::Vector3f& targetPos, f32 minDist,
                        f32 maxDist);
    void shotWheel(f32 degree);
    void calcBreathPosDistanceDegree(sead::Vector3f* pos, f32 distance, f32 degree);

    void exeAttackSign();
    void exeAttackStart();
    void exeAttack();
    void exeAttackWait();
    void exeAttackEnd();

private:
    BossRaid* mBossRaid = nullptr;
    al::DeriveActorGroup<BossRaidWheel>* mWheelList = nullptr;
    al::JointAimInfo* mJointAimRoot = nullptr;
    al::JointAimInfo* mJointAimHead = nullptr;
    al::LiveActor* mBreathActor = nullptr;
    sead::Matrix34f mLandingPointMtx = sead::Matrix34f::ident;
    sead::Matrix34f mToMouthMtx = sead::Matrix34f::ident;
    sead::Vector3f mTargetPos = sead::Vector3f::zero;
    sead::Vector3f _0xac = sead::Vector3f::zero;
    sead::Vector3f mBreathCenter = sead::Vector3f::zero;
    s32 mShotIndex = 0;
    s32 mAttackCount = 0;
    s32 mAttackMax = 2;
    bool mIsLeftTarget = true;
};

static_assert(sizeof(BossRaidStateBreathAttack) == 0xD8);
