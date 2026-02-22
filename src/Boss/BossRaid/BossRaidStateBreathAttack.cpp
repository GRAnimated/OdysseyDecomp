#include "Boss/BossRaid/BossRaidStateBreathAttack.h"

#include <math/seadVector.h>

#include "Library/Controller/PadRumbleFunction.h"
#include "Library/Effect/EffectSystemInfo.h"
#include "Library/Joint/JointAimInfo.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/LiveActor/LiveActorGroup.h"
#include "Library/Math/MathUtil.h"
#include "Library/Matrix/MatrixUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Boss/BossRaid/BossRaid.h"
#include "Boss/BossRaid/BossRaidWheel.h"
#include "Util/PlayerUtil.h"

namespace {
NERVE_IMPL(BossRaidStateBreathAttack, AttackSign);
NERVE_IMPL(BossRaidStateBreathAttack, AttackStart);
NERVE_IMPL(BossRaidStateBreathAttack, Attack);
NERVE_IMPL(BossRaidStateBreathAttack, AttackWait);
NERVE_IMPL(BossRaidStateBreathAttack, AttackEnd);
NERVES_MAKE_NOSTRUCT(BossRaidStateBreathAttack, AttackSign, AttackStart, Attack, AttackWait,
                     AttackEnd);
}  // namespace

static const f32 sAngleTable0[2] = {135.0f, -135.0f};
static const f32 sAngleTable1[2] = {45.0f, -45.0f};
static const f32 sAngleTable2[2] = {-45.0f, 45.0f};
static const f32 sAngleTable3[2] = {-135.0f, 135.0f};

BossRaidStateBreathAttack::BossRaidStateBreathAttack(BossRaid* boss, const al::ActorInitInfo& info)
    : al::NerveStateBase("状態名"), mBossRaid(boss) {
    al::multVecPose(&mBreathCenter, mBossRaid, sead::Vector3f::ez * -7800.0f);

    al::DeriveActorGroup<BossRaidWheel>* group =
        new al::DeriveActorGroup<BossRaidWheel>("光輪リスト", 20);
    mWheelList = group;
    for (s32 i = 0; i < group->getMaxActorCount(); i++) {
        BossRaidWheel* wheel = new BossRaidWheel("光輪");
        al::initCreateActorNoPlacementInfo(wheel, info);
        group->registerActor(wheel);
    }

    mBreathActor = al::tryGetSubActor(mBossRaid, "ブレス");
    al::setEffectNamedMtxPtr(mBreathActor, "LandingPoint", &mLandingPointMtx);
    al::setEffectNamedMtxPtr(mBreathActor, "ToMouth", &mToMouthMtx);

    mJointAimRoot = new al::JointAimInfo();
    mJointAimRoot->setPowerRate(0.0f);
    mJointAimRoot->setInterpoleRate(1.0f);
    mJointAimRoot->setBaseAimLocalDir(sead::Vector3f::ez);
    mJointAimRoot->setBaseUpLocalDir(sead::Vector3f::ex);
    mJointAimRoot->setLimitDegreeRect(30.0f, 30.0f, 0.0f, 0.0f);
    al::initJointAimController(mBossRaid, mJointAimRoot, "JointRoot");

    mJointAimHead = new al::JointAimInfo();
    mJointAimHead->setPowerRate(0.0f);
    mJointAimHead->setInterpoleRate(1.0f);
    mJointAimHead->setBaseAimLocalDir(sead::Vector3f::ex);
    mJointAimHead->setBaseUpLocalDir({-sead::Vector3f::ez});
    mJointAimHead->setBaseSideLocalDir({-sead::Vector3f::ey});
    mJointAimHead->setBaseOffsetLocal({900.0f, 0.0f, 400.0f});
    mJointAimHead->setLimitDegreeRect(20.0f, 20.0f, 50.0f, 50.0f);
    al::initJointAimController(mBossRaid, mJointAimHead, "Head");

    al::NerveExecutor::initNerve(&AttackSign, 0);
}

BossRaidStateBreathAttack::~BossRaidStateBreathAttack() = default;

// NON_MATCHING: switch generates bitmask pattern instead of jump table; minor instruction count
// difference
void BossRaidStateBreathAttack::appear() {
    al::NerveStateBase::appear();
    mBreathActor->kill();
    mAttackCount = 0;
    switch (mBossRaid->getShotLevel()) {
    case 0:
    case 1:
    case 3:
    case 4:
        mAttackMax = 2;
        break;
    case 2:
    case 5:
        mAttackMax = 1;
        break;
    default:
        break;
    }
    al::setNerve(this, &AttackSign);
}

void BossRaidStateBreathAttack::kill() {
    al::NerveStateBase::kill();
    mBreathActor->kill();
    setAimRate(0.0f);
}

void BossRaidStateBreathAttack::control() {
    mJointAimRoot->setTargetPos(mTargetPos);
    mJointAimHead->setTargetPos(mTargetPos);

    mLandingPointMtx.setTranslation(mTargetPos);

    al::LiveActor* breathActor = mBreathActor;
    sead::Matrix34f* toMouthMtx = &mToMouthMtx;
    sead::Vector3f dir = al::getTrans(breathActor) - mTargetPos;
    al::makeMtxUpNoSupportPos(toMouthMtx, dir, mTargetPos);
}

void BossRaidStateBreathAttack::setAimRate(f32 rate) {
    mJointAimRoot->setPowerRate(rate);
    mJointAimHead->setPowerRate(rate);
}

void BossRaidStateBreathAttack::updateAttackSign() {
    switch (mBossRaid->getShotLevel()) {
    case 0:
        updateAttackSignLv1();
        break;
    case 1:
        updateAttackSignLv1();
        break;
    case 2:
        updateAttackSignLv3();
        break;
    case 3:
        updateAttackSignLv1();
        break;
    case 4:
        updateAttackSignLv1();
        break;
    case 5:
        updateAttackSignLv3();
        break;
    default:
        break;
    }
}

// NON_MATCHING: dx/dz assigned to swapped f32 registers; fmax vs fmaxnm; stp store ordering differs
void BossRaidStateBreathAttack::updateAttackSignLv1() {
    if (al::isFirstStep(this)) {
        const sead::Vector3f& playerPos = rs::getPlayerPos(mBossRaid);
        limitBreathPos(&mTargetPos, playerPos, 6500.0f, 8000.0f);
    }

    const sead::Vector3f& playerPos = rs::getPlayerPos(mBossRaid);
    sead::Vector3f target;
    limitBreathPos(&target, playerPos, 6500.0f, 8000.0f);
    al::convergeVec(&mTargetPos, mTargetPos, target, 10.0f);
}

void BossRaidStateBreathAttack::updateAttackSignLv2() {
    updateAttackSignLv1();
}

// NON_MATCHING: extra callee-saved register x20 for mBossRaid; frame size differs
void BossRaidStateBreathAttack::updateAttackSignLv3() {
    if (al::isFirstStep(this)) {
        const sead::Vector3f& playerPos = rs::getPlayerPos(mBossRaid);
        bool isLeft = al::isLeftTarget(mBossRaid, playerPos);
        mIsLeftTarget = isLeft;
        sead::Vector3f frontDir = sead::Vector3f::ez;
        f32 degree = isLeft ? -15.0f : 15.0f;
        al::calcFrontDir(&frontDir, mBossRaid);
        sead::Vector3f shotDir;
        shotDir = frontDir * 9000.0f;
        al::rotateVectorDegree(&frontDir, shotDir, sead::Vector3f::ey, degree);
        mTargetPos = mBreathCenter + frontDir;
    }
}

bool BossRaidStateBreathAttack::updateAttack() {
    switch (mBossRaid->getShotLevel()) {
    case 0:
        return updateAttackLv1();
    case 1:
        return updateAttackLv2();
    case 2:
        return updateAttackLv3();
    case 3:
        return updateAttackLv1();
    case 4:
        return updateAttackLv2();
    case 5:
        return updateAttackLv3();
    default:
        return true;
    }
}

bool BossRaidStateBreathAttack::updateAttackLv1() {
    if (al::isStep(this, 20)) {
        shotWheel(0.0f);
        shotWheel(120.0f);
        shotWheel(240.0f);
    }
    return al::isGreaterEqualStep(this, 70);
}

bool BossRaidStateBreathAttack::updateAttackLv2() {
    if (al::isStep(this, 20)) {
        shotWheel(0.0f);
        shotWheel(72.0f);
        shotWheel(144.0f);
        shotWheel(216.0f);
        shotWheel(288.0f);
    }
    return al::isGreaterEqualStep(this, 70);
}

// NON_MATCHING: frame size differs; register allocation and instruction ordering differ
// significantly
bool BossRaidStateBreathAttack::updateAttackLv3() {
    if (al::isFirstStep(this))
        mShotIndex = 0;

    f32 sweepDeg = al::calcNerveValue(this, 250, -15.0f, 15.0f);
    f32 rotDeg = sweepDeg;
    if (!mIsLeftTarget)
        rotDeg = -rotDeg;
    sead::Vector3f frontDir = sead::Vector3f::ez;
    al::calcFrontDir(&frontDir, mBossRaid);
    al::rotateVectorDegree(&frontDir, frontDir * 9000.0f, sead::Vector3f::ey, rotDeg);
    mTargetPos = mBreathCenter + frontDir;

    if (al::isIntervalStep(this, 62, 0)) {
        shotWheel(sAngleTable0[(mShotIndex & 1) == 0]);
        shotWheel(sAngleTable1[(mShotIndex & 1) == 0]);
        shotWheel(sAngleTable2[(mShotIndex & 1) == 0]);
        shotWheel(sAngleTable3[(mShotIndex & 1) == 0]);
        mShotIndex++;
    }

    return al::isGreaterEqualStep(this, 250);
}

// NON_MATCHING: s9 and s10 regswap
void BossRaidStateBreathAttack::limitBreathPos(sead::Vector3f* result,
                                               const sead::Vector3f& targetPos, f32 minDist,
                                               f32 maxDist) {
    sead::Vector3f diff = targetPos - mBreathCenter;
    diff.y = 0.0f;
    f32 distSq = diff.squaredLength();
    f32 dist = sead::Mathf::sqrt(distSq);

    if (dist < minDist || (minDist = sead::Mathf::max(maxDist, dist - 1500.0f), dist > minDist)) {
        f32 dist2 = sead::Mathf::sqrt(distSq);
        if (dist2 > 0.0f) {
            f32 scale = minDist / dist2;
            diff.x = diff.x * scale;
            diff.y = scale * 0.0f;
            diff.z = diff.z * scale;
        }
    }
    *result = diff + mBreathCenter;
}

void BossRaidStateBreathAttack::shotWheel(f32 degree) {
    BossRaidWheel* wheel = (BossRaidWheel*)mWheelList->tryFindDeadActor();
    if (wheel) {
        sead::Vector3f frontDir = sead::Vector3f::ez;
        al::calcJointFrontDir(&frontDir, mBossRaid, "JointRoot");
        al::rotateVectorDegree(&frontDir, sead::Vector3f::ey, degree);
        wheel->shotGround(mTargetPos, frontDir, 0.0f);
    }
}

void BossRaidStateBreathAttack::calcBreathPosDistanceDegree(sead::Vector3f* pos, f32 distance,
                                                            f32 degree) {
    sead::Vector3f frontDir = sead::Vector3f::ez;
    al::calcFrontDir(&frontDir, mBossRaid);
    sead::Vector3f shotDir = frontDir * distance;
    al::rotateVectorDegree(&frontDir, shotDir, sead::Vector3f::ey, degree);
    *pos = mBreathCenter + frontDir;
}

void BossRaidStateBreathAttack::exeAttackSign() {
    if (al::isFirstStep(this)) {
        mBossRaid->startActionMain("BreathAttackSign");
        mBossRaid->startElectricParts();
        mBreathActor->appear();
        al::startAction(mBreathActor, "Sign");
    }
    updateAttackSign();
    f32 frame = al::getActionFrame(mBossRaid);
    f32 frameMax = al::getActionFrameMax(mBossRaid);
    f32 rate = al::easeInOut(frame / frameMax);
    setAimRate(rate);
    if (al::isActionEnd(mBossRaid))
        al::setNerve(this, &AttackStart);
}

void BossRaidStateBreathAttack::exeAttackStart() {
    if (al::isFirstStep(this)) {
        mBossRaid->startActionMain("BreathAttack");
        mBossRaid->startElectricParts();
        al::startAction(mBreathActor, "Appear");
        setAimRate(1.0f);
        alPadRumbleFunction::startPadRumblePos(mBreathActor, mTargetPos, "ドーン（強）", 3000.0f,
                                               8500.0f, -1);
        alPadRumbleFunction::startPadRumbleLoop(mBreathActor, "【ループ】ジリジリ（弱）",
                                                &mTargetPos, 3000.0f, 8500.0f, -1);
    }
    if (al::isActionEnd(mBreathActor))
        al::setNerve(this, &Attack);
}

void BossRaidStateBreathAttack::exeAttack() {
    if (al::isFirstStep(this))
        al::startAction(mBreathActor, "Wait");
    if (updateAttack())
        al::setNerve(this, &AttackWait);
}

void BossRaidStateBreathAttack::exeAttackWait() {
    if (al::isGreaterEqualStep(this, 0))
        al::setNerve(this, &AttackEnd);
}

// NON_MATCHING: extra callee-saved register x20; frame size differs
void BossRaidStateBreathAttack::exeAttackEnd() {
    if (al::isFirstStep(this)) {
        alPadRumbleFunction::stopPadRumbleLoop(mBreathActor, "【ループ】ジリジリ（弱）",
                                               &mTargetPos, -1);
        mBossRaid->startActionMain("BreathAttackEnd");
        al::startAction(mBreathActor, "End");
        mBossRaid->startElectricParts();
    }
    f32 frame = al::getActionFrame(mBossRaid);
    f32 frameMax = al::getActionFrameMax(mBossRaid);
    f32 rate = al::easeInOut(1.0f - (frame / frameMax));
    setAimRate(rate);
    if (al::isActionEnd(mBossRaid)) {
        mAttackCount++;
        mIsLeftTarget = !mIsLeftTarget;
        if (mAttackCount >= mAttackMax)
            kill();
        else
            al::setNerve(this, &AttackSign);
    }
}
