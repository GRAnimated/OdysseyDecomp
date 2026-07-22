#include "Item/CoinStackGroup.h"

#include <prim/seadStorageFor.h>
#include <random/seadRandom.h>

#include "Library/Clipping/ClippingActorHolder.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorPoseKeeper.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSceneInfo.h"
#include "Library/LiveActor/ActorSensorFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementId.h"
#include "Library/Stage/StageSwitchKeeper.h"
#include "Library/Stage/StageSwitchUtil.h"
#include "Library/Thread/FunctorV0M.h"

#include "Item/CoinStack.h"
#include "System/GameDataUtil.h"

constexpr f32 cStackDistH = 74.5f;

CoinStackGroup::CoinStackGroup(const char* name) : al::LiveActor(name) {}

void CoinStackGroup::init(const al::ActorInitInfo& initInfo) {
    using CoinStackGroupFunctor = al::FunctorV0M<CoinStackGroup*, void (CoinStackGroup::*)()>;

    al::initActorSceneInfo(this, initInfo);
    al::initActorPoseTFSV(this);
    al::initActorSRT(this, initInfo);
    al::initActorClipping(this, initInfo);
    initHitSensor(1);
    al::addHitSensor(this, initInfo, "Body", 13, 0.0f, 8, sead::Vector3f::zero);
    al::initExecutorMapObjMovement(this, initInfo);
    al::initStageSwitch(this, initInfo);
    mPlacementId = new al::PlacementId();
    al::tryGetArg(&mStackAmount, initInfo, "StacksAmount");
    al::tryGetArg(&mIsMustSave, initInfo, "MustSave");
    if (mIsMustSave && al::tryGetPlacementId(mPlacementId, initInfo))
        rs::tryFindCoinStackSave(&mStackAmount, this, mPlacementId);
    generateCoinStackGroup(initInfo, mStackAmount);
    if (al::isValidStageSwitch(this, "SwitchAppear")) {
        if (al::listenStageSwitchOn(
                this, "SwitchAppear",
                CoinStackGroupFunctor(this, &CoinStackGroup::makeStackAppear)) &&
            al::listenStageSwitchOff(
                this, "SwitchAppear",
                CoinStackGroupFunctor(this, &CoinStackGroup::makeStackDisappear)) &&
            al::trySyncStageSwitchAppear(this)) {
            return;
        }
    }
    if (mCoinStack)
        mCoinStack->makeStackAppear();
    makeActorAlive();
}

void CoinStackGroup::control() {
    for (CoinStack* stack = mCoinStack; stack != nullptr; stack = stack->getAbove())
        stack->setTransY(al::getTrans(stack).y - stack->getFallSpeed());
}

bool CoinStackGroup::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                                al::HitSensor* self) {
    if (al::isMsgChangeAlpha(message)) {
        mCoinStack->changeAlpha(al::getChangeAlphaValue(message));
        return true;
    }

    return false;
}

void CoinStackGroup::makeActorDead() {
    if (mCoinStack)
        mCoinStack->makeStackDisappear();
    al::LiveActor::makeActorDead();
}

void CoinStackGroup::makeActorAlive() {
    al::LiveActor::makeActorAlive();
    if (mCoinStack)
        mCoinStack->makeStackAppear();
}


inline f32 getRandomRange(sead::Random* random, f32 scale) {
    f32 value = random->getF32();
    return scale * value * ((value > 0.5f) ? 1.0f : -1.0f) + 0.0f;
}

void CoinStackGroup::generateCoinStackGroup(const al::ActorInitInfo& initInfo, s32 stackSize) {
    f32 clippingRadius = updateClippingInfo(stackSize);
    sead::Vector3f stackTrans = al::getTrans(this);

    CoinStack* previousStack = nullptr;
    for (u32 index = 0; index != (u32)stackSize; index++) {
        CoinStack* newStack = new CoinStack("CoinStack");
        newStack->init(initInfo);

        sead::StorageFor<sead::Random> random;
        sead::StorageFor<sead::Vector3f> resultTrans;
        if (index == 0) {
            resultTrans.construct(stackTrans);
            newStack->postInit(this, *resultTrans, previousStack, mClippingPos, clippingRadius,
                               &cStackDistH);
            mCoinStack = newStack;
            previousStack = newStack;
            continue;
        }

        random.constructDefault();
        sead::Vector3f coinTrans;
        coinTrans.x = getRandomRange(random.data(), 10.0f);
        coinTrans.y = index * cStackDistH;
        random->init();
        coinTrans.z = getRandomRange(random.data(), 10.0f);
        resultTrans.construct(stackTrans + coinTrans);
        newStack->postInit(this, *resultTrans, previousStack, mClippingPos, clippingRadius,
                           &cStackDistH);
        previousStack = newStack;
    }
}

void CoinStackGroup::makeStackAppear() {
    if (mCoinStack)
        mCoinStack->makeStackAppear();
}

void CoinStackGroup::makeStackDisappear() {
    if (mCoinStack)
        mCoinStack->makeStackDisappear();
}

f32 CoinStackGroup::setStackAsCollected(CoinStack* stack) {
    mStackAmount--;
    al::invalidateClipping(this);
    if (mIsMustSave)
        rs::saveCoinStack(this, mPlacementId, mStackAmount);

    if (mStackAmount == 0) {
        kill();
        return 0.0f;
    }

    if (mCoinStack == stack)
        mCoinStack = stack->getAbove();

    return updateClippingInfo(mStackAmount);
}

f32 CoinStackGroup::updateClippingInfo(u32 stackSize) {
    f32 clippingRadius = stackSize * 0.75f * cStackDistH;
    mClippingPos = al::getTrans(this) + sead::Vector3f(0.0f, clippingRadius * 0.75f, 0.0f);

    al::setClippingInfo(this, clippingRadius, &mClippingPos);
    return clippingRadius;
}

void CoinStackGroup::validateClipping() {
    al::validateClipping(this);
}
