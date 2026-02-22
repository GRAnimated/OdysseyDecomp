#include "Boss/BossRaid/BossRaidChain.h"

#include <math/seadMathCalcCommon.h>
#include <math/seadVector.h>

#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
NERVE_IMPL(BossRaidChain, Wait);
NERVE_IMPL(BossRaidChain, BlowDown);
NERVE_IMPL(BossRaidChain, Deactive);
NERVE_IMPL(BossRaidChain, Demo);
NERVES_MAKE_NOSTRUCT(BossRaidChain, Wait, BlowDown, Deactive, Demo);
}  // namespace

BossRaidChain::BossRaidChain(const char* name, const char* archiveName, const char* modelName,
                             f32 minDist, f32 maxDist)
    : al::LiveActor(name), mArchiveName(archiveName), mModelName(modelName), mMinDist(minDist),
      mMaxDist(maxDist) {}

void BossRaidChain::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, mArchiveName, mModelName);
    al::initNerve(this, &Wait, 0);
    makeActorAlive();
}

void BossRaidChain::setPrevChain(BossRaidChain* chain) {
    mPrevChain = chain;
}

void BossRaidChain::setNextChain(BossRaidChain* chain) {
    mNextChain = chain;
}

void BossRaidChain::setDemoFollowMtxPtr(const sead::Matrix34f* mtxPtr) {
    mDemoFollowMtxPtr = mtxPtr;
}

void BossRaidChain::setFix() {
    mIsFix = true;
}

void BossRaidChain::addVelocityChain(const sead::Vector3f& velocity) {
    if (!mIsFix)
        al::addVelocity(this, velocity);
}

void BossRaidChain::updateVelocity(BossRaidChain* other) {
    if (other == nullptr)
        return;

    const sead::Vector3f& otherTrans = al::getTrans(other);
    const sead::Vector3f& myTrans = al::getTrans(this);

    f32 dx = otherTrans.x - myTrans.x;
    f32 dy = otherTrans.y - myTrans.y;
    f32 dz = otherTrans.z - myTrans.z;
    f32 dist = sqrtf(dx * dx + dy * dy + dz * dz);

    sead::Vector3f vel = sead::Vector3f::zero;
    if (al::isNearZero(dist)) {
        al::getRandomDir(&vel);
    } else {
        if (dist < mMinDist) {
            f32 scale = (dist - mMinDist) * -0.2f / (dist * mMinDist);
            vel.x = dx * scale;
            vel.y = dy * scale;
            vel.z = dz * scale;
        }
        if (dist > mMaxDist) {
            f32 scale = (dist - mMaxDist) / dist * 0.4f;
            vel.x = dx * scale;
            vel.y = dy * scale;
            vel.z = dz * scale;
        }
    }

    if (!mIsFix)
        al::addVelocity(this, vel);

    sead::Vector3f negVel = {-vel.x, -vel.y, -vel.z};
    if (!other->mIsFix)
        al::addVelocity(other, negVel);
}

void BossRaidChain::updateDirection(f32 yRate, f32 zRate) {
    sead::Vector3f dir = sead::Vector3f::zero;

    if (mPrevChain != nullptr) {
        const sead::Vector3f& myTrans = al::getTrans(this);
        const sead::Vector3f& prevTrans = al::getTrans(mPrevChain);
        dir += myTrans - prevTrans;
    }

    if (mNextChain != nullptr) {
        const sead::Vector3f& nextTrans = al::getTrans(mNextChain);
        const sead::Vector3f& myTrans = al::getTrans(this);
        dir += nextTrans - myTrans;
    }

    sead::Vector3f up = sead::Vector3f::ex;
    if (mPrevChain != nullptr)
        al::calcUpDir(&up, mPrevChain);

    sead::Quatf* quat = al::getQuatPtr(this);
    al::turnQuatYDirRate(quat, *quat, up, yRate);

    if (al::tryNormalizeOrZero(&dir)) {
        sead::Quatf* quat2 = al::getQuatPtr(this);
        al::turnQuatZDirRate(quat2, *quat2, dir, zRate);
    }
}

void BossRaidChain::resetDirection() {
    updateDirection(1.0f, 1.0f);
}

void BossRaidChain::startBlowDown() {
    al::setNerve(this, &BlowDown);
}

void BossRaidChain::active() {
    if (!al::isNerve(this, &Deactive))
        return;
    al::setNerve(this, &Wait);
    al::onCalcAnim(this);
}

void BossRaidChain::deactive() {
    if (!al::isNerve(this, &Wait))
        return;
    al::setNerve(this, &Deactive);
    al::setVelocityZero(this);
    al::offCalcAnim(this);
}

void BossRaidChain::setUpDemo() {
    al::setNerve(this, &Demo);
    al::setVelocityZero(this);
}

void BossRaidChain::reset() {
    al::setNerve(this, &Wait);
    al::onCalcAnim(this);
    al::setVelocityZero(this);
    al::resetPosition(this);
    makeActorAlive();
}

void BossRaidChain::exeDeactive() {}

void BossRaidChain::exeDemo() {
    if (mDemoFollowMtxPtr != nullptr)
        al::updatePoseMtx(this, mDemoFollowMtxPtr);
}

void BossRaidChain::exeWait() {
    updateVelocity(mPrevChain);
    updateVelocity(mNextChain);
    updateDirection(0.15f, 0.4f);
    if (!mIsFix) {
        al::addVelocityToGravity(this, 2.0f);
        al::scaleVelocity(this, 0.9f);
    }
}

void BossRaidChain::exeBlowDown() {
    if (al::isFirstStep(this)) {
        const sead::Vector3f& gravity = al::getGravity(this);
        sead::Vector3f randomDir;
        al::getRandomDirH(&randomDir, gravity);
        const sead::Vector3f& gravity2 = al::getGravity(this);
        mBlowAxis.setCross(randomDir, gravity2);
        al::setVelocitySeparateHV(this, randomDir, 25.0f, 35.0f);
    }
    sead::Quatf* quat = al::getQuatPtr(this);
    al::rotateQuatRadian(quat, *quat, mBlowAxis, 0.40143f);
    al::addVelocityToGravity(this, 2.0f);
    al::scaleVelocity(this, 0.99f);
    if (al::isGreaterEqualStep(this, 180))
        kill();
}
