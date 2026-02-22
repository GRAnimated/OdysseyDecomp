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
    mJointAimHead->setBaseUpLocalDir(
        {-sead::Vector3f::ez.x, -sead::Vector3f::ez.y, -sead::Vector3f::ez.z});
    mJointAimHead->setBaseSideLocalDir(
        {-sead::Vector3f::ey.x, -sead::Vector3f::ey.y, -sead::Vector3f::ey.z});
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
    mJointAimRoot->setPowerRate(0.0f);
    mJointAimHead->setPowerRate(0.0f);
}

void BossRaidStateBreathAttack::control() {
    mJointAimRoot->setTargetPos(mTargetPos);
    mJointAimHead->setTargetPos(mTargetPos);

    mLandingPointMtx.setTranslation(mTargetPos);

    al::LiveActor* breathActor = mBreathActor;
    sead::Matrix34f* toMouthMtx = &mToMouthMtx;
    const sead::Vector3f& trans = al::getTrans(breathActor);
    sead::Vector3f dir = trans - mTargetPos;
    al::makeMtxUpNoSupportPos(toMouthMtx, dir, mTargetPos);
}

void BossRaidStateBreathAttack::setAimRate(f32 rate) {
    mJointAimRoot->setPowerRate(rate);
    mJointAimHead->setPowerRate(rate);
}

// NON_MATCHING: switch generates bitmask pattern instead of jump table
void BossRaidStateBreathAttack::updateAttackSign() {
    switch (mBossRaid->getShotLevel()) {
    case 0:
    case 1:
    case 3:
    case 4:
        updateAttackSignLv1();
        break;
    case 2:
    case 5:
        updateAttackSignLv3();
        break;
    }
}

// NON_MATCHING: dx/dz assigned to swapped f32 registers; fmax vs fmaxnm; stp store ordering differs
void BossRaidStateBreathAttack::updateAttackSignLv1() {
    if (al::isFirstStep(this)) {
        const sead::Vector3f& playerPos = rs::getPlayerPos(mBossRaid);
        f32 dx = playerPos.x - mBreathCenter.x;
        f32 dz = playerPos.z - mBreathCenter.z;
        f32 dy = 0.0f;
        f32 distSq = dx * dx + dy * dy + dz * dz;
        f32 dist = sead::Mathf::sqrt(distSq);
        f32 clampDist = 6500.0f;
        if (dist < 6500.0f ||
            (clampDist = sead::Mathf::max(dist - 1500.0f, 8000.0f), dist > clampDist)) {
            f32 d2 = sead::Mathf::sqrt(distSq);
            if (d2 > 0.0f) {
                f32 scale = clampDist / d2;
                dx = dx * scale;
                dy = dy * scale;
                dz = dz * scale;
            }
        }
        mTargetPos.x = dx + mBreathCenter.x;
        mTargetPos.y = dy + mBreathCenter.y;
        mTargetPos.z = dz + mBreathCenter.z;
    }

    const sead::Vector3f& playerPos = rs::getPlayerPos(mBossRaid);
    f32 dx = playerPos.x - mBreathCenter.x;
    f32 dz = playerPos.z - mBreathCenter.z;
    f32 dy = 0.0f;
    f32 distSq = dx * dx + dy * dy + dz * dz;
    f32 dist = sead::Mathf::sqrt(distSq);
    f32 clampDist = 6500.0f;
    if (dist < 6500.0f ||
        (clampDist = sead::Mathf::max(dist - 1500.0f, 8000.0f), dist > clampDist)) {
        f32 d2 = sead::Mathf::sqrt(distSq);
        if (d2 > 0.0f) {
            f32 scale = clampDist / d2;
            dx = dx * scale;
            dy = dy * scale;
            dz = dz * scale;
        }
    }
    sead::Vector3f target;
    target.x = dx + mBreathCenter.x;
    target.y = dy + mBreathCenter.y;
    target.z = dz + mBreathCenter.z;
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
        shotDir.x = frontDir.x * 9000.0f;
        shotDir.y = frontDir.y * 9000.0f;
        shotDir.z = frontDir.z * 9000.0f;
        al::rotateVectorDegree(&frontDir, shotDir, sead::Vector3f::ey, degree);
        mTargetPos.x = mBreathCenter.x + frontDir.x;
        mTargetPos.y = mBreathCenter.y + frontDir.y;
        mTargetPos.z = mBreathCenter.z + frontDir.z;
    }
}

// NON_MATCHING: switch generates bitmask pattern instead of jump table
bool BossRaidStateBreathAttack::updateAttack() {
    switch (mBossRaid->getShotLevel()) {
    case 0:
    case 3:
        return updateAttackLv1();
    case 1:
    case 4:
        return updateAttackLv2();
    case 2:
    case 5:
        return updateAttackLv3();
    default:
        return true;
    }
}

bool BossRaidStateBreathAttack::updateAttackLv1() {
    if (al::isStep(this, 20)) {
        BossRaidWheel* wheel = (BossRaidWheel*)mWheelList->tryFindDeadActor();
        if (wheel) {
            sead::Vector3f frontDir = sead::Vector3f::ez;
            al::calcJointFrontDir(&frontDir, mBossRaid, "JointRoot");
            al::rotateVectorDegree(&frontDir, sead::Vector3f::ey, 0.0f);
            wheel->shotGround(mTargetPos, frontDir, 0.0f);
        }
        BossRaidWheel* wheel2 = (BossRaidWheel*)mWheelList->tryFindDeadActor();
        if (wheel2) {
            sead::Vector3f frontDir = sead::Vector3f::ez;
            al::calcJointFrontDir(&frontDir, mBossRaid, "JointRoot");
            al::rotateVectorDegree(&frontDir, sead::Vector3f::ey, 120.0f);
            wheel2->shotGround(mTargetPos, frontDir, 0.0f);
        }
        BossRaidWheel* wheel3 = (BossRaidWheel*)mWheelList->tryFindDeadActor();
        if (wheel3) {
            sead::Vector3f frontDir = sead::Vector3f::ez;
            al::calcJointFrontDir(&frontDir, mBossRaid, "JointRoot");
            al::rotateVectorDegree(&frontDir, sead::Vector3f::ey, 240.0f);
            wheel3->shotGround(mTargetPos, frontDir, 0.0f);
        }
    }
    return al::isGreaterEqualStep(this, 70);
}

bool BossRaidStateBreathAttack::updateAttackLv2() {
    if (al::isStep(this, 20)) {
        BossRaidWheel* wheel = (BossRaidWheel*)mWheelList->tryFindDeadActor();
        if (wheel) {
            sead::Vector3f frontDir = sead::Vector3f::ez;
            al::calcJointFrontDir(&frontDir, mBossRaid, "JointRoot");
            al::rotateVectorDegree(&frontDir, sead::Vector3f::ey, 0.0f);
            wheel->shotGround(mTargetPos, frontDir, 0.0f);
        }
        BossRaidWheel* wheel2 = (BossRaidWheel*)mWheelList->tryFindDeadActor();
        if (wheel2) {
            sead::Vector3f frontDir = sead::Vector3f::ez;
            al::calcJointFrontDir(&frontDir, mBossRaid, "JointRoot");
            al::rotateVectorDegree(&frontDir, sead::Vector3f::ey, 72.0f);
            wheel2->shotGround(mTargetPos, frontDir, 0.0f);
        }
        BossRaidWheel* wheel3 = (BossRaidWheel*)mWheelList->tryFindDeadActor();
        if (wheel3) {
            sead::Vector3f frontDir = sead::Vector3f::ez;
            al::calcJointFrontDir(&frontDir, mBossRaid, "JointRoot");
            al::rotateVectorDegree(&frontDir, sead::Vector3f::ey, 144.0f);
            wheel3->shotGround(mTargetPos, frontDir, 0.0f);
        }
        BossRaidWheel* wheel4 = (BossRaidWheel*)mWheelList->tryFindDeadActor();
        if (wheel4) {
            sead::Vector3f frontDir = sead::Vector3f::ez;
            al::calcJointFrontDir(&frontDir, mBossRaid, "JointRoot");
            al::rotateVectorDegree(&frontDir, sead::Vector3f::ey, 216.0f);
            wheel4->shotGround(mTargetPos, frontDir, 0.0f);
        }
        BossRaidWheel* wheel5 = (BossRaidWheel*)mWheelList->tryFindDeadActor();
        if (wheel5) {
            sead::Vector3f frontDir = sead::Vector3f::ez;
            al::calcJointFrontDir(&frontDir, mBossRaid, "JointRoot");
            al::rotateVectorDegree(&frontDir, sead::Vector3f::ey, 288.0f);
            wheel5->shotGround(mTargetPos, frontDir, 0.0f);
        }
    }
    return al::isGreaterEqualStep(this, 70);
}

// NON_MATCHING: frame size differs; register allocation and instruction ordering differ
// significantly
bool BossRaidStateBreathAttack::updateAttackLv3() {
    if (al::isFirstStep(this))
        mShotIndex = 0;

    f32 sweepDeg = al::calcNerveValue(this, 250, -15.0f, 15.0f);
    sead::Vector3f frontDir = sead::Vector3f::ez;
    f32 rotDeg = mIsLeftTarget ? -sweepDeg : sweepDeg;
    al::calcFrontDir(&frontDir, mBossRaid);
    sead::Vector3f shotDir;
    shotDir.x = frontDir.x * 9000.0f;
    shotDir.y = frontDir.y * 9000.0f;
    shotDir.z = frontDir.z * 9000.0f;
    al::rotateVectorDegree(&frontDir, shotDir, sead::Vector3f::ey, rotDeg);
    mTargetPos.x = mBreathCenter.x + frontDir.x;
    mTargetPos.y = mBreathCenter.y + frontDir.y;
    mTargetPos.z = mBreathCenter.z + frontDir.z;

    if (al::isIntervalStep(this, 62, 0)) {
        BossRaidWheel* wheel = (BossRaidWheel*)mWheelList->tryFindDeadActor();
        if (wheel) {
            f32 angle = sAngleTable0[(mShotIndex & 1) == 0];
            sead::Vector3f dir = sead::Vector3f::ez;
            al::calcJointFrontDir(&dir, mBossRaid, "JointRoot");
            al::rotateVectorDegree(&dir, sead::Vector3f::ey, angle);
            wheel->shotGround(mTargetPos, dir, 0.0f);
        }
        BossRaidWheel* wheel2 = (BossRaidWheel*)mWheelList->tryFindDeadActor();
        if (wheel2) {
            f32 angle = sAngleTable1[(mShotIndex & 1) == 0];
            sead::Vector3f dir = sead::Vector3f::ez;
            al::calcJointFrontDir(&dir, mBossRaid, "JointRoot");
            al::rotateVectorDegree(&dir, sead::Vector3f::ey, angle);
            wheel2->shotGround(mTargetPos, dir, 0.0f);
        }
        BossRaidWheel* wheel3 = (BossRaidWheel*)mWheelList->tryFindDeadActor();
        if (wheel3) {
            f32 angle = sAngleTable2[(mShotIndex & 1) == 0];
            sead::Vector3f dir = sead::Vector3f::ez;
            al::calcJointFrontDir(&dir, mBossRaid, "JointRoot");
            al::rotateVectorDegree(&dir, sead::Vector3f::ey, angle);
            wheel3->shotGround(mTargetPos, dir, 0.0f);
        }
        BossRaidWheel* wheel4 = (BossRaidWheel*)mWheelList->tryFindDeadActor();
        if (wheel4) {
            f32 angle = sAngleTable3[(mShotIndex & 1) == 0];
            sead::Vector3f dir = sead::Vector3f::ez;
            al::calcJointFrontDir(&dir, mBossRaid, "JointRoot");
            al::rotateVectorDegree(&dir, sead::Vector3f::ey, angle);
            wheel4->shotGround(mTargetPos, dir, 0.0f);
        }
        mShotIndex++;
    }

    return al::isGreaterEqualStep(this, 250);
}

// NON_MATCHING: dx/dz assigned to swapped f32 registers; register allocation differs
void BossRaidStateBreathAttack::limitBreathPos(sead::Vector3f* result,
                                               const sead::Vector3f& targetPos, f32 minDist,
                                               f32 maxDist) {
    f32 dx = targetPos.x - mBreathCenter.x;
    f32 dz = targetPos.z - mBreathCenter.z;
    f32 dy = 0.0f;
    f32 distSq = dx * dx + dy * dy + dz * dz;
    f32 dist = sead::Mathf::sqrt(distSq);
    f32 clampDist = minDist;
    if (dist < minDist ||
        ((dist - 1500.0f >= maxDist) ? (clampDist = dist - 1500.0f) : (clampDist = maxDist),
         dist > clampDist)) {
        f32 d2 = sead::Mathf::sqrt(distSq);
        if (d2 > 0.0f) {
            f32 scale = clampDist / d2;
            dx = dx * scale;
            dy = dy * scale;
            dz = dz * scale;
        }
    }
    result->x = dx + mBreathCenter.x;
    result->y = dy + mBreathCenter.y;
    result->z = dz + mBreathCenter.z;
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

// NON_MATCHING: register allocation differs
void BossRaidStateBreathAttack::calcBreathPosDistanceDegree(sead::Vector3f* pos, f32 distance,
                                                            f32 degree) {
    sead::Vector3f frontDir = sead::Vector3f::ez;
    al::calcFrontDir(&frontDir, mBossRaid);
    sead::Vector3f shotDir;
    shotDir.x = frontDir.x * distance;
    shotDir.y = frontDir.y * distance;
    shotDir.z = frontDir.z * distance;
    al::rotateVectorDegree(&frontDir, shotDir, sead::Vector3f::ey, degree);
    pos->x = mBreathCenter.x + frontDir.x;
    pos->y = mBreathCenter.y + frontDir.y;
    pos->z = mBreathCenter.z + frontDir.z;
}

// NON_MATCHING: updateAttackSign() inlined; register differences
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
    mJointAimRoot->setPowerRate(rate);
    mJointAimHead->setPowerRate(rate);
    if (al::isActionEnd(mBossRaid))
        al::setNerve(this, &AttackStart);
}

void BossRaidStateBreathAttack::exeAttackStart() {
    if (al::isFirstStep(this)) {
        mBossRaid->startActionMain("BreathAttack");
        mBossRaid->startElectricParts();
        al::startAction(mBreathActor, "Appear");
        mJointAimRoot->setPowerRate(1.0f);
        mJointAimHead->setPowerRate(1.0f);
        alPadRumbleFunction::startPadRumblePos(mBreathActor, mTargetPos, "ドーン（強）", 3000.0f,
                                               8500.0f, -1);
        alPadRumbleFunction::startPadRumbleLoop(mBreathActor, "【ループ】ジリジリ（弱）",
                                                &mTargetPos, 3000.0f, 8500.0f, -1);
    }
    if (al::isActionEnd(mBreathActor))
        al::setNerve(this, &Attack);
}

// NON_MATCHING: updateAttack() inlined
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
    mJointAimRoot->setPowerRate(rate);
    mJointAimHead->setPowerRate(rate);
    if (al::isActionEnd(mBossRaid)) {
        mAttackCount++;
        mIsLeftTarget ^= true;
        if (mAttackCount >= mAttackMax)
            kill();
        else
            al::setNerve(this, &AttackSign);
    }
}
