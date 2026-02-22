#include "Boss/BossRaid/BossRaidChainList.h"

#include <math/seadVector.h>

#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Boss/BossRaid/BossRaidChain.h"

namespace {
NERVE_IMPL(BossRaidChainList, BlowDown);
NERVE_IMPL(BossRaidChainList, Demo);
NERVE_IMPL(BossRaidChainList, Wait);
NERVE_IMPL(BossRaidChainList, Deactive);
NERVES_MAKE_NOSTRUCT(BossRaidChainList, BlowDown, Demo, Wait, Deactive);
}  // namespace

BossRaidChainList::BossRaidChainList(const char* name, const char* modelName, s32 count,
                                     f32 minDist, f32 maxDist)
    : al::LiveActor(name), mModelName(modelName), mChainCount(count), mMinDist(minDist),
      mMaxDist(maxDist) {}

void BossRaidChainList::init(const al::ActorInitInfo& info) {
    al::initActorSceneInfo(this, info);
    al::initExecutorEnemyMovement(this, info);
    al::initActorPoseTQSV(this);
    al::initActorClipping(this, info);
    al::invalidateClipping(this);

    mChains = new BossRaidChain*[mChainCount];

    for (s32 i = 0; i < mChainCount; i++) {
        BossRaidChain* chain = new BossRaidChain("鎖", mModelName,
                                                  i == 0 || i == mChainCount - 1 ? "NoModel"
                                                                                  : nullptr,
                                                  mMinDist, mMaxDist);
        mChains[i] = chain;
        al::initCreateActorWithPlacementInfo(mChains[i], info);
    }

    if (mChainCount > 1) {
        for (s32 i = 0; i < mChainCount - 1; i++) {
            mChains[i]->setNextChain(mChains[i + 1]);
            mChains[i + 1]->setPrevChain(mChains[i]);
        }
    }

    if (mChainCount >= 1) {
        mChains[0]->setFix();
        mChains[mChainCount - 1]->setFix();
    }

    al::initNerve(this, &Wait, 0);

    const sead::Vector3f* rootPtr = mRootPosPtr;
    if (!rootPtr)
        rootPtr = &al::getTrans(this);
    sead::Vector3f rootPos;
    rootPos.set(*rootPtr);
    const sead::Vector3f* tipPtr = mTipPosPtr;
    sead::Vector3f tipPos;
    if (!tipPtr)
        tipPtr = &al::getTrans(this);
    tipPos.set(*tipPtr);
    resetChain(rootPos, tipPos);

    makeActorAlive();
}

void BossRaidChainList::makeActorAlive() {
    al::setNerve(this, &Wait);
    al::LiveActor::makeActorAlive();
    for (s32 i = 0; i < mChainCount; i++)
        mChains[i]->makeActorAlive();
}

void BossRaidChainList::makeActorDead() {
    al::LiveActor::makeActorDead();
    for (s32 i = 0; i < mChainCount; i++)
        mChains[i]->makeActorDead();
}

void BossRaidChainList::setRootPosPtr(const sead::Vector3f* pos) {
    mRootPosPtr = pos;
}

void BossRaidChainList::setTipPosPtr(const sead::Vector3f* pos) {
    mTipPosPtr = pos;
}

void BossRaidChainList::calcRootAndTipPos(sead::Vector3f* outRoot, sead::Vector3f* outTip) {
    const sead::Vector3f* rootPos = mRootPosPtr;
    if (!rootPos)
        rootPos = &al::getTrans(this);
    outRoot->set(*rootPos);

    const sead::Vector3f* tipPos = mTipPosPtr;
    if (!tipPos)
        tipPos = &al::getTrans(this);
    outTip->set(*tipPos);
}

BossRaidChain* BossRaidChainList::getChain(s32 index) {
    return mChains[index];
}

void BossRaidChainList::registerHostSubActorSyncClipping(al::LiveActor* actor) {
    al::registerSubActorSyncClipping(actor, this);
    for (s32 i = 0; i < mChainCount; i++)
        al::registerSubActorSyncClipping(actor, mChains[i]);
}

void BossRaidChainList::resetChain() {
    const sead::Vector3f* rootPtr = mRootPosPtr;
    if (!rootPtr)
        rootPtr = &al::getTrans(this);
    sead::Vector3f rootPos;
    rootPos.set(*rootPtr);
    const sead::Vector3f* tipPtr = mTipPosPtr;
    sead::Vector3f tipPos;
    if (!tipPtr)
        tipPtr = &al::getTrans(this);
    tipPos.set(*tipPtr);
    resetChain(rootPos, tipPos);
}

void BossRaidChainList::resetChain(const sead::Vector3f& rootPos, const sead::Vector3f& tipPos) {
    for (s32 i = 0; i < mChainCount; i++) {
        sead::Vector3f* transPtr = al::getTransPtr(mChains[i]);
        al::lerpVec(transPtr, rootPos, tipPos, (f32)i / (f32)(mChainCount - 1));
        sead::Vector3f* transPtr2 = al::getTransPtr(mChains[i]);
        const sead::Vector3f& trans = al::getTrans(mChains[i]);
        al::addRandomVector(transPtr2, trans, 10.0f);
    }
    for (s32 i = 0; i < mChainCount; i++) {
        mChains[i]->resetDirection();
        mChains[i]->reset();
    }
}

void BossRaidChainList::startBlowDown() {
    al::setNerve(this, &BlowDown);
}

void BossRaidChainList::active() {
    if (!al::isNerve(this, &Deactive))
        return;
    al::setNerve(this, &Wait);
    for (s32 i = 0; i < mChainCount; i++)
        mChains[i]->active();
}

void BossRaidChainList::deactive() {
    if (!al::isNerve(this, &Wait))
        return;
    al::setNerve(this, &Deactive);
    for (s32 i = 0; i < mChainCount; i++)
        mChains[i]->deactive();
}

void BossRaidChainList::setUpDemo() {
    al::setNerve(this, &Demo);
    for (s32 i = 0; i < mChainCount; i++)
        mChains[i]->setUpDemo();
}

void BossRaidChainList::reset() {
    const sead::Vector3f* rootPtr = mRootPosPtr;
    if (!rootPtr)
        rootPtr = &al::getTrans(this);
    sead::Vector3f rootPos;
    rootPos.set(*rootPtr);
    const sead::Vector3f* tipPtr = mTipPosPtr;
    sead::Vector3f tipPos;
    if (!tipPtr)
        tipPtr = &al::getTrans(this);
    tipPos.set(*tipPtr);
    resetChain(rootPos, tipPos);
    al::setNerve(this, &Wait);
    makeActorAlive();
}

void BossRaidChainList::exeDemo() {}

void BossRaidChainList::exeDeactive() {}

// NON_MATCHING: instruction scheduling (vel stores before ldr; fadd operand order)
void BossRaidChainList::exeWait() {
    const sead::Vector3f* rootPtr = mRootPosPtr;
    if (!rootPtr)
        rootPtr = &al::getTrans(this);
    sead::Vector3f rootPos;
    rootPos.set(*rootPtr);
    const sead::Vector3f* tipPtr = mTipPosPtr;
    sead::Vector3f tipPos;
    if (!tipPtr)
        tipPtr = &al::getTrans(this);
    tipPos.set(*tipPtr);

    al::resetPosition(mChains[0], rootPos);
    al::resetPosition(mChains[mChainCount - 1], tipPos);

    if (mChainCount - 1 >= 2) {
        for (s32 i = 1; i < mChainCount - 1; i++) {
            sead::Vector3f lerpPos = sead::Vector3f::zero;
            al::lerpVec(&lerpPos, rootPos, tipPos, (f32)i / (f32)(mChainCount - 1));
            const sead::Vector3f& trans = al::getTrans(mChains[i]);
            sead::Vector3f diff = {lerpPos.x - trans.x, lerpPos.y - trans.y,
                                   lerpPos.z - trans.z};
            f32 dist = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
            if (dist > 50.0f) {
                f32 scale = (dist - 50.0f) / dist * 0.05f;
                sead::Vector3f vel = {diff.x * scale, diff.y * scale, diff.z * scale};
                al::addVelocity(mChains[i], vel);
            }
        }
    }

    if (mChainCount >= 2) {
        for (s32 i = 0; i < mChainCount - 1; i++) {
            mChains[i]->exeWait();
            sead::Vector3f* transPtr = al::getTransPtr(mChains[i]);
            const sead::Vector3f& vel = al::getVelocity(mChains[i]);
            transPtr->x += vel.x;
            transPtr->y += vel.y;
            transPtr->z += vel.z;
        }
    }
}

// NON_MATCHING: cmp w8 hoisted before tbz; vtable offset difference for makeActorDead
void BossRaidChainList::exeBlowDown() {
    if (al::isFirstStep(this)) {
        for (s32 i = 0; i < mChainCount; i++)
            mChains[i]->startBlowDown();
        if (mChainCount < 1) {
            makeActorDead();
            return;
        }
    }
    if (mChainCount < 1) {
        makeActorDead();
        return;
    }
    for (s32 i = 0; i < mChainCount; i++) {
        if (al::isAlive(mChains[i]))
            return;
    }
    makeActorDead();
}
