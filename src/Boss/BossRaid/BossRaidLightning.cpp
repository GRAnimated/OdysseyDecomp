#include "Boss/BossRaid/BossRaidLightning.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Boss/BossRaid/BossRaidElectricLine.h"

namespace {
NERVE_IMPL(BossRaidLightning, Sign);
NERVE_IMPL(BossRaidLightning, Fall);
NERVE_IMPL(BossRaidLightning, Land);
NERVES_MAKE_NOSTRUCT(BossRaidLightning, Sign, Fall, Land);
}  // namespace

BossRaidLightning::BossRaidLightning(const char* name) : al::LiveActor(name) {}

void BossRaidLightning::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, "BossRaidLightning", nullptr);
    al::initNerve(this, &Sign, 0);
    al::invalidateClipping(this);
    mElectricLine = new BossRaidElectricLine("電撃ライン[ライトニング用]");
    al::initCreateActorNoPlacementInfo(mElectricLine, info);
    makeActorDead();
}

void BossRaidLightning::setBulletList(al::DeriveActorGroup<BossRaidElectric>* bulletList) {
    mElectricLine->setBulletList(bulletList);
}

void BossRaidLightning::exeSign() {
    if (al::isFirstStep(this))
        al::startAction(this, "FallSign");
    if (al::isActionEnd(this))
        al::setNerve(this, &Fall);
}

void BossRaidLightning::exeFall() {
    if (al::isFirstStep(this))
        al::startAction(this, "Fall");
    if (al::isActionEnd(this)) {
        al::startHitReaction(this, "着弾");
        al::setNerve(this, &Land);
    }
}

void BossRaidLightning::exeLand() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Land");
        mAngleOffset = mShotDir * -60.0f;
    }
    shotBullet();
    if (al::isGreaterEqualStep(this, 120))
        kill();
}

void BossRaidLightning::shotBullet() {
    if (!al::isIntervalStep(this, 8, 0))
        return;

    f32 angle = mAngleOffset + mShotDir * al::calcNerveRepeatDegree(this, 0x78);

    sead::Vector3f shotDir;
    sead::Vector3f frontDir;
    al::calcFrontDir(&frontDir, this);
    al::rotateVectorDegree(&shotDir, frontDir, sead::Vector3f::ey, angle);

    BossRaidElectricLine* electricLine = mElectricLine;
    const sead::Vector3f& trans = al::getTrans(this);
    sead::Vector3f offset = mRadius * shotDir;
    electricLine->shot(trans, offset);
}

void BossRaidLightning::killForce() {
    mElectricLine->killForce();
    makeActorDead();
}

void BossRaidLightning::shot(const sead::Vector3f& pos, bool isRight, f32 radius) {
    mShotDir = isRight ? 1.0f : -1.0f;
    mRadius = radius;
    mShotPos.set(pos);
    al::setTrans(this, mShotPos);
    al::setNerve(this, &Sign);
    makeActorAlive();
}
