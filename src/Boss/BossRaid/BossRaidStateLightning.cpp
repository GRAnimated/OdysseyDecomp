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

BossRaidStateLightning::BossRaidStateLightning(BossRaid* boss, const al::ActorInitInfo& info)
    : al::NerveStateBase("光輪攻撃"), mBossRaid(boss) {
    al::calcFrontDir(&mFrontDir, mBossRaid);
    al::calcUpDir(&mUpDir, mBossRaid);
    al::DeriveActorGroup<BossRaidWheel>* group =
        new al::DeriveActorGroup<BossRaidWheel>("光輪リスト", 16);

    mWheelList = group;
    for (s32 i = 0; i < group->getMaxActorCount(); i++) {
        BossRaidWheel* wheel = new BossRaidWheel("光輪");
        al::initCreateActorNoPlacementInfo(wheel, info);
        group->registerActor(wheel);
    }

    al::calcTransLocalOffset(&mLocalOffset, mBossRaid, sead::Vector3f::ez * 2500.0f);
    al::NerveExecutor::initNerve(&Lightning, 0);
}

BossRaidStateLightning::~BossRaidStateLightning() = default;

void BossRaidStateLightning::appear() {
    al::NerveStateBase::appear();
    al::setNerve(this, &Lightning);
}

void BossRaidStateLightning::exeLightning() {
    if (al::isFirstStep(this)) {
        al::startAction(mBossRaid, "Lightning");
        mBossRaid->startElectricParts();
    }
    if (al::isStep(this, 1))
        mBossRaid->resetChainAll();

    if (al::isIntervalStep(this, getLightningInterval(), 20)) {
        BossRaidWheel* wheel = (BossRaidWheel*)mWheelList->tryFindDeadActor();
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
            al::calcJointPos(&jointPos, mBossRaid, (mShotIndex & 1) ? "HandL" : "HandR");

            target = mLocalOffset - rotDir * 4000.0f;
            wheel->shotGround(jointPos, target, 0.0f);

            mShotIndex = al::modi(mShotIndex + 5, 4);
        }
    }

    if (al::isGreaterEqualStep(this, 300))
        al::setNerve(this, &LightningEnd);
}

s32 BossRaidStateLightning::getLightningInterval() const {
    s32 level = mBossRaid->getShotLevel() % 3;
    if ((u32)level < 3)
        return sIntervalTable[level];
    return sIntervalTable[2];
}

void BossRaidStateLightning::exeLightningEnd() {
    if (al::isGreaterEqualStep(this, 180))
        kill();
}

void BossRaidStateLightning::killWheel() {
    mWheelList->makeActorDeadAll();
}
