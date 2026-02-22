#include "Boss/BossRaid/BossRaidStateLightning.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActorGroup.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Boss/BossRaid/BossRaid.h"
#include "Boss/BossRaid/BossRaidWheel.h"

namespace {
NERVE_IMPL(BossRaidStateLightning, Lightning);
NERVE_IMPL(BossRaidStateLightning, LightningEnd);
NERVES_MAKE_NOSTRUCT(BossRaidStateLightning, Lightning, LightningEnd);
}  // namespace

static const s32 sIntervalTable[3] = {70, 60, 35};

// NON_MATCHING: extra callee-saved register (x28) vs target (only x27); wrong function size
BossRaidStateLightning::BossRaidStateLightning(BossRaid* boss, const al::ActorInitInfo& info)
    : al::NerveStateBase("光輪攻撃"), mBossRaid(boss) {
    mWheelList = new al::DeriveActorGroup<BossRaidWheel>("光輪リスト", 16);
    for (s32 i = 0; i < mWheelList->getMaxActorCount(); i++) {
        BossRaidWheel* wheel = new BossRaidWheel("光輪");
        al::initCreateActorNoPlacementInfo(wheel, info);
        mWheelList->registerActor(wheel);
    }
    al::calcFrontDir(&mFrontDir, mBossRaid);
    al::calcUpDir(&mUpDir, mBossRaid);
    al::calcTransLocalOffset(&mLocalOffset, mBossRaid, sead::Vector3f::ez * 2500.0f);
    al::NerveExecutor::initNerve(&Lightning, 0);
}

BossRaidStateLightning::~BossRaidStateLightning() = default;

void BossRaidStateLightning::appear() {
    al::NerveStateBase::appear();
    al::setNerve(this, &Lightning);
}

// NON_MATCHING: sIntervalTable adrp hoisted before cmp; extra mov x2/x0 in target before shotGround
void BossRaidStateLightning::exeLightning() {
    if (al::isFirstStep(this)) {
        al::startAction(mBossRaid, "Lightning");
        mBossRaid->startElectricParts();
    }
    if (al::isStep(this, 1))
        mBossRaid->resetChainAll();

    s32 level = mBossRaid->getShotLevel() % 3;
    s32 interval = (level > 2) ? 35 : sIntervalTable[level];
    if (al::isIntervalStep(this, interval, 20)) {
        BossRaidWheel* wheel =
            (BossRaidWheel*)mWheelList->tryFindDeadActor();
        if (wheel) {
                f32 angle = al::getRandom(-15.0f, 15.0f);
            switch (mShotIndex) {
            case 0:
                angle += 30.0f;
                break;
            case 1:
                angle += -15.0f;
                break;
            case 2:
                angle += 15.0f;
                break;
            case 3:
                angle += -30.0f;
                break;
            default:
                break;
            }

            sead::Vector3f rotDir;
            sead::Vector3f jointPos;
            sead::Vector3f target;
            al::rotateVectorDegree(&rotDir, mFrontDir, mUpDir, angle);

            const char* jointName = (mShotIndex & 1) ? "HandL" : "HandR";
            al::calcJointPos(&jointPos, mBossRaid, jointName);

            target.x = mLocalOffset.x - rotDir.x * 4000.0f;
            target.y = mLocalOffset.y - rotDir.y * 4000.0f;
            target.z = mLocalOffset.z - rotDir.z * 4000.0f;
            wheel->shotGround(jointPos, target, 0.0f);

            mShotIndex = al::modi(mShotIndex + 5, 4);
        }
    }

    if (al::isGreaterEqualStep(this, 300))
        al::setNerve(this, &LightningEnd);
}

// NON_MATCHING: compiler eliminates the level > 2 branch (level = getShotLevel() % 3 is always 0-2)
s32 BossRaidStateLightning::getLightningInterval() const {
    s32 level = mBossRaid->getShotLevel() % 3;
    if (level > 2)
        return 35;
    return sIntervalTable[level];
}

void BossRaidStateLightning::exeLightningEnd() {
    if (al::isGreaterEqualStep(this, 180))
        kill();
}

void BossRaidStateLightning::killWheel() {
    mWheelList->makeActorDeadAll();
}
