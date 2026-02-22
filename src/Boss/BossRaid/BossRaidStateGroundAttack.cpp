#include "Boss/BossRaid/BossRaidStateGroundAttack.h"

#include <math/seadMathCalcCommon.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActorGroup.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Boss/BossRaid/BossRaid.h"
#include "Boss/BossRaid/BossRaidElectric.h"
#include "Boss/BossRaid/BossRaidElectricLine.h"

namespace {
NERVE_IMPL(BossRaidStateGroundAttack, FallSign);
NERVE_IMPL(BossRaidStateGroundAttack, Fall);
NERVE_IMPL(BossRaidStateGroundAttack, Attack);
NERVE_IMPL(BossRaidStateGroundAttack, AttackEnd);
NERVES_MAKE_NOSTRUCT(BossRaidStateGroundAttack, FallSign, Fall, Attack, AttackEnd);
}  // namespace

static const s32 sGroundAttackTimeTable[3] = {240, 240, 290};

// NON_MATCHING: initialization of last loop
BossRaidStateGroundAttack::BossRaidStateGroundAttack(BossRaid* boss, const al::ActorInitInfo& info)
    : al::NerveStateBase("状態名"), mBossRaid(boss) {
    al::NerveExecutor::initNerve(&FallSign, 0);
    al::calcFrontDir(&mFrontDir, mBossRaid);
    al::calcUpDir(&mUpDir, mBossRaid);

    al::DeriveActorGroup<BossRaidElectric>* group =
        new al::DeriveActorGroup<BossRaidElectric>("電撃リスト", 230);
    mElectricList = group;

    for (s32 i = 0; i < group->getMaxActorCount(); i++) {
        BossRaidElectric* electric = new BossRaidElectric("電撃");
        al::initCreateActorNoPlacementInfo(electric, info);
        group->registerActor(electric);
    }

    al::DeriveActorGroup<BossRaidElectricLine>* lineGroup =
        new al::DeriveActorGroup<BossRaidElectricLine>("電撃ラインリスト", 8);
    mElectricLineList = lineGroup;

    for (s32 i = 0; i < lineGroup->getMaxActorCount(); i++) {
        BossRaidElectricLine* line = new BossRaidElectricLine("電撃ライン本体用");
        al::initCreateActorNoPlacementInfo(line, info);
        lineGroup->registerActor(line);
    }

    for (s32 i = 0; i < lineGroup->getActorCount(); i++)
        lineGroup->getDeriveActor(i)->setBulletList(group);
}

BossRaidStateGroundAttack::~BossRaidStateGroundAttack() = default;

void BossRaidStateGroundAttack::appear() {
    al::NerveStateBase::appear();
    al::setNerve(this, &FallSign);
}

void BossRaidStateGroundAttack::exeFallSign() {
    if (al::isFirstStep(this)) {
        mBossRaid->startActionMain("FallSign");
        al::startAction(mBossRaid->mArmorActor, "Electric");
        mBossRaid->validateCollisionAll();
    }
    if (al::isGreaterEqualStep(this, 150))
        al::setNerve(this, &Fall);
}

void BossRaidStateGroundAttack::exeFall() {
    if (al::isFirstStep(this))
        mBossRaid->startActionMain("Fall");
    if (al::isGreaterEqualStep(this, 120)) {
        al::startHitReaction(mBossRaid, "落下衝撃");
        al::setNerve(this, &Attack);
    }
}

void BossRaidStateGroundAttack::exeAttack() {
    if (al::isFirstStep(this)) {
        mBossRaid->startActionMain("GroundAttack");
        mBossRaid->startElectricParts();
        for (s32 i = 0; i < mElectricLineList->getActorCount(); i++)
            mElectricLineList->getDeriveActor(i)->resetPrevBullet();
    }
    if (al::isActionPlaying(mBossRaid, "GroundAttack") && al::isActionEnd(mBossRaid))
        mBossRaid->startActionMain("GroundAttackLoop");

    s32 time = getGroundAttackTime();
    if (al::isLessStep(this, time)) {
        switch (mBossRaid->getShotLevel()) {
        case 0:
            shotBulletLv1();
            break;
        case 1:
            shotBulletLv2();
            break;
        case 2:
            shotBulletLv3();
            break;
        case 3:
            shotBulletLv4();
            break;
        case 4:
        case 5:
            shotBulletLv5();
            break;
        default:
            break;
        }
    }

    s32 level = mBossRaid->getShotLevel() % 3;
    if ((u32)level < 3) {
        if (al::isGreaterEqualStep(this, sGroundAttackTimeTable[level] + 60))
            al::setNerve(this, &AttackEnd);
    } else if (al::isGreaterEqualStep(this, 350))
        al::setNerve(this, &AttackEnd);
}

s32 BossRaidStateGroundAttack::getGroundAttackTime() const {
    s32 level = mBossRaid->getShotLevel() % 3;
    if ((u32)level < 3)
        return sGroundAttackTimeTable[level];

    return sGroundAttackTimeTable[2];
}

void BossRaidStateGroundAttack::shotBulletLv1() {
    sead::Vector3f upDir = mUpDir;

    if (!al::isIntervalStep(this, 8, 0))
        return;

    sead::Vector3f rotDir;
    sead::Vector3f upOffset = upDir * 50.0f;
    f32 baseAngle = al::calcNerveCosCycle(this, 160) * -45.0f;

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, baseAngle);
    mElectricLineList->getDeriveActor(0)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 36.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, baseAngle + 90.0f);
    mElectricLineList->getDeriveActor(1)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 36.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, baseAngle + -90.0f);
    mElectricLineList->getDeriveActor(2)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 36.0f);
}

void BossRaidStateGroundAttack::shotBulletLv2() {
    sead::Vector3f upDir = mUpDir;

    if (!al::isIntervalStep(this, 8, 0))
        return;

    sead::Vector3f rotDir;
    sead::Vector3f upOffset = upDir * 50.0f;
    f32 phase = al::calcNerveCosCycle(this, 115) * 24.0f;

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, -120.0f - phase);
    mElectricLineList->getDeriveActor(0)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 36.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, phase + -72.0f);
    mElectricLineList->getDeriveActor(1)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 36.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, -24.0f - phase);
    mElectricLineList->getDeriveActor(2)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 36.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, phase + 24.0f);
    mElectricLineList->getDeriveActor(3)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 36.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, 72.0f - phase);
    mElectricLineList->getDeriveActor(4)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 36.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, phase + 120.0f);
    mElectricLineList->getDeriveActor(5)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 36.0f);
}

void BossRaidStateGroundAttack::shotBulletLv3() {
    sead::Vector3f upDir = mUpDir;
    if (!al::isIntervalStep(this, 7, 0))
        return;

    sead::Vector3f rotDir;
    sead::Vector3f upOffset = upDir * 50.0f;
    f32 phase = sead::Mathf::sin(((f32)al::getNerveStep(this) / 115.0f - 1.0f / 12.0f) *
                                 sead::Mathf::pi2()) *
                30.0f;

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, -105.0f - phase);
    mElectricLineList->getDeriveActor(6)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 40.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, phase + -75.0f);
    mElectricLineList->getDeriveActor(0)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 40.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, -45.0f - phase);
    mElectricLineList->getDeriveActor(1)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 40.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, phase + -15.0f);
    mElectricLineList->getDeriveActor(2)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 40.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, 15.0f - phase);
    mElectricLineList->getDeriveActor(3)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 40.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, phase + 45.0f);
    mElectricLineList->getDeriveActor(4)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 40.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, 75.0f - phase);
    mElectricLineList->getDeriveActor(5)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 40.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, phase + 105.0f);
    mElectricLineList->getDeriveActor(7)->shot(upOffset + al::getTrans(mBossRaid), rotDir * 40.0f);
}

void BossRaidStateGroundAttack::shotBulletLv4() {
    if (!al::isIntervalStep(this, 8, 0))
        return;

    sead::Vector3f rotDir;
    f32 baseAngle = al::calcNerveCosCycle(this, 160) * -45.0f;

    f32 dist1 = al::isIntervalOnOffStep(this, 8, 0) ? 50.0f : 350.0f;
    sead::Vector3f nearOffset = mUpDir * dist1;

    f32 dist2 = al::isIntervalOnOffStep(this, 8, 0) ? 350.0f : 50.0f;
    sead::Vector3f farOffset = mUpDir * dist2;

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, baseAngle);
    mElectricLineList->getDeriveActor(0)->shot(nearOffset + al::getTrans(mBossRaid),
                                               rotDir * 36.0f);
    mElectricLineList->getDeriveActor(0)->shot(farOffset + al::getTrans(mBossRaid), rotDir * 36.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, baseAngle + 90.0f);
    mElectricLineList->getDeriveActor(1)->shot(nearOffset + al::getTrans(mBossRaid),
                                               rotDir * 36.0f);
    mElectricLineList->getDeriveActor(1)->shot(farOffset + al::getTrans(mBossRaid), rotDir * 36.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, baseAngle + -90.0f);
    mElectricLineList->getDeriveActor(2)->shot(nearOffset + al::getTrans(mBossRaid),
                                               rotDir * 36.0f);
    mElectricLineList->getDeriveActor(2)->shot(farOffset + al::getTrans(mBossRaid), rotDir * 36.0f);
}

void BossRaidStateGroundAttack::shotBulletLv5() {
    if (!al::isIntervalStep(this, 12, 0))
        return;

    sead::Vector3f rotDir;
    f32 phase = al::calcNerveCosCycle(this, 115) * 24.0f;

    f32 dist1 = al::isIntervalOnOffStep(this, 12, 0) ? 50.0f : 350.0f;
    sead::Vector3f nearOffset = mUpDir * dist1;

    f32 dist2 = al::isIntervalOnOffStep(this, 12, 0) ? 350.0f : 50.0f;
    sead::Vector3f farOffset = mUpDir * dist2;

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, -120.0f - phase);
    mElectricLineList->getDeriveActor(0)->shot(nearOffset + al::getTrans(mBossRaid),
                                               rotDir * 36.0f);
    mElectricLineList->getDeriveActor(0)->shot(farOffset + al::getTrans(mBossRaid), rotDir * 36.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, phase + -72.0f);
    mElectricLineList->getDeriveActor(1)->shot(nearOffset + al::getTrans(mBossRaid),
                                               rotDir * 36.0f);
    mElectricLineList->getDeriveActor(1)->shot(farOffset + al::getTrans(mBossRaid), rotDir * 36.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, -24.0f - phase);
    mElectricLineList->getDeriveActor(2)->shot(nearOffset + al::getTrans(mBossRaid),
                                               rotDir * 36.0f);
    mElectricLineList->getDeriveActor(2)->shot(farOffset + al::getTrans(mBossRaid), rotDir * 36.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, phase + 24.0f);
    mElectricLineList->getDeriveActor(3)->shot(nearOffset + al::getTrans(mBossRaid),
                                               rotDir * 36.0f);
    mElectricLineList->getDeriveActor(3)->shot(farOffset + al::getTrans(mBossRaid), rotDir * 36.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, 72.0f - phase);
    mElectricLineList->getDeriveActor(4)->shot(nearOffset + al::getTrans(mBossRaid),
                                               rotDir * 36.0f);
    mElectricLineList->getDeriveActor(4)->shot(farOffset + al::getTrans(mBossRaid), rotDir * 36.0f);

    al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, phase + 120.0f);
    mElectricLineList->getDeriveActor(5)->shot(nearOffset + al::getTrans(mBossRaid),
                                               rotDir * 36.0f);
    mElectricLineList->getDeriveActor(5)->shot(farOffset + al::getTrans(mBossRaid), rotDir * 36.0f);
}

void BossRaidStateGroundAttack::shotBulletLv6() {
    shotBulletLv5();
}

void BossRaidStateGroundAttack::exeAttackEnd() {
    if (al::isFirstStep(this)) {
        mBossRaid->startActionMain("GroundAttackEnd");
        mBossRaid->endElectricParts();
    }
    if (al::isGreaterEqualStep(this, 135))
        kill();
}

bool BossRaidStateGroundAttack::isElectric() const {
    return !al::isNerve(this, &AttackEnd);
}

void BossRaidStateGroundAttack::updateBullet() {
    s32 count = mElectricList->getActorCount();
    for (s32 i = 0; i < count; i++) {
        BossRaidElectric* electric = mElectricList->getDeriveActor(i);
        if (al::isAlive(electric))
            electric->updatePosition();
    }
}

void BossRaidStateGroundAttack::killBulletAll() {
    mElectricList->makeActorDeadAll();
    for (s32 i = 0; i < mElectricLineList->getActorCount(); i++)
        mElectricLineList->getDeriveActor(i)->killForce();
}
