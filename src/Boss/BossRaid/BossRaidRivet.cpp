#include "Boss/BossRaid/BossRaidRivet.h"

#include <math/seadMathCalcCommon.h>
#include <math/seadVector.h>

#include "Library/Collision/PartsConnectorUtil.h"
#include "Library/Effect/EffectSystemInfo.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Player/PlayerUtil.h"
#include "Library/Stage/StageSwitchUtil.h"

#include "Boss/BossRaid/BossRaidChainList.h"
#include "Enemy/Popn.h"
#include "Player/CapTargetInfo.h"
#include "Util/Hack.h"
#include "Util/ItemGenerator.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(BossRaidRivet, Wait);
NERVE_IMPL(BossRaidRivet, PullOut);
NERVE_IMPL(BossRaidRivet, Electric);
NERVE_IMPL(BossRaidRivet, Pull);
NERVE_IMPL(BossRaidRivet, ElectricSign);
NERVE_IMPL(BossRaidRivet, ElectricEnd);
NERVE_IMPL(BossRaidRivet, Demo);
NERVES_MAKE_NOSTRUCT(BossRaidRivet, Wait, PullOut, Electric, Pull, ElectricSign, ElectricEnd, Demo);
}  // namespace

const sead::Vector3f sCapPointPos = {15.0f, 0.0f, 5.0f};

BossRaidRivet::BossRaidRivet(const char* name) : al::LiveActor(name) {}

void BossRaidRivet::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, "BossRaidRivet", nullptr);
    al::initNerve(this, &Wait, 0);
    al::initJointControllerKeeper(this, 1);
    al::initJointLocalYRotator(this, &mJointAngle, "AllRoot");
    mCapTargetInfo = rs::createCapTargetInfo(this, nullptr);
    mMtxConnector = al::createMtxConnector(this);
    mChainRootMtxConnector = al::createMtxConnector(this);
    mItemGenerator = new ItemGenerator(this, info);
    makeActorAlive();
}

void BossRaidRivet::setConnect(al::LiveActor* actor, const char* jointName,
                               const sead::Vector3f& rotate, const sead::Vector3f& trans) {
    if (jointName)
        al::attachMtxConnectorToJoint(mMtxConnector, actor, jointName, rotate, trans);
    else
        al::attachMtxConnectorToActor(mMtxConnector, actor, rotate, trans);
}

void BossRaidRivet::setChainRootConnect(al::LiveActor* actor, const char* jointName,
                                        const sead::Vector3f& rotate, const sead::Vector3f& trans) {
    if (jointName)
        al::attachMtxConnectorToJoint(mChainRootMtxConnector, actor, jointName, rotate, trans);
    else
        al::attachMtxConnectorToActor(mChainRootMtxConnector, actor, rotate, trans);
}

// NON_MATCHING: regalloc (d8 saved for sqrtf result; operator new called before dist calc; vector
// literal stored via immediates vs global load)
void BossRaidRivet::createChainAndPopn(al::LiveActor* actor, const al::ActorInitInfo& info) {
    calcAnim();
    al::calcJointOffsetPos(&mCapPointPos, this, "CapPoint", sCapPointPos);
    al::calcConnectTrans(&mChainRootPos, mChainRootMtxConnector);

    f32 dist = (mCapPointPos - mChainRootPos).length();

    f32 countF = (dist + 150.0f) / 100.0f;
    s32 count = (s32)countF;
    if (countF >= 0.0f && (f32)count != countF)
        count += 1;

    mChainList = new BossRaidChainList("鎖リスト", "BossRaidChain", count, 20.0f, 60.0f);
    mChainList->setRootPosPtr(&mChainRootPos);
    mChainList->setTipPosPtr(&mCapPointPos);
    al::initCreateActorNoPlacementInfo(mChainList, info);
    mChainList->makeActorDead();
    mChainList->registerHostSubActorSyncClipping(actor);

    mPopn = new Popn("襲撃ポップン");
    mPopn->setArchiveName("PopnBossRaid");
    mPopn->setAngry(true);
    mPopn->init(info);
    mPopn->makeActorDead();
}

void BossRaidRivet::calcAnim() {
    if (!al::isNerve(this, &PullOut))
        al::connectPoseQT(this, mMtxConnector);
    al::LiveActor::calcAnim();
    al::calcJointOffsetPos(&mCapPointPos, this, "CapPoint", sCapPointPos);
    al::calcConnectTrans(&mChainRootPos, mChainRootMtxConnector);
}

bool BossRaidRivet::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                               al::HitSensor* self) {
    if (rs::tryReceiveMsgInitCapTargetAndSetCapTargetInfo(message, mCapTargetInfo))
        return true;
    if (al::isMsgPlayerDisregard(message))
        return al::isNerve(this, &PullOut);
    if (al::isMsgPlayerTouch(message) && al::isNerve(this, &Electric)) {
        al::sendMsgEnemyAttack(other, self);
        return true;
    }
    if (rs::isMsgCapReflect(message) && al::isNerve(this, &Electric))
        return true;
    if (rs::isMsgCapKeepLockOn(message))
        return isEnableCapKeepLockOn();
    if (rs::isMsgCapIgnoreCancelLockOn(message))
        return al::isNerve(this, &Pull);
    if (rs::isMsgCapCancelLockOn(message))
        return true;
    if (rs::isMsgCapStartLockOn(message) &&
        (al::isNerve(this, &Wait) || al::isNerve(this, &ElectricSign) ||
         al::isNerve(this, &ElectricEnd))) {
        al::setNerve(this, &Pull);
        return true;
    }
    return false;
}

bool BossRaidRivet::isElectric() const {
    return al::isNerve(this, &Electric);
}

bool BossRaidRivet::isEnableCapKeepLockOn() const {
    if (al::isNerve(this, &PullOut))
        return al::isLessEqualStep(this, 5);
    return true;
}

bool BossRaidRivet::isPullOut() const {
    if (al::isDead(this))
        return true;
    if (al::isNerve(this, &PullOut))
        return al::isGreaterEqualStep(this, 30);
    return false;
}

void BossRaidRivet::exeDemo() {}

void BossRaidRivet::exeWait() {
    if (al::isFirstStep(this))
        al::startAction(this, "Wait");
}

void BossRaidRivet::exeElectricSign() {
    if (al::isFirstStep(this))
        al::startAction(this, "ElectricSign");
}

void BossRaidRivet::exeElectric() {
    if (al::isFirstStep(this))
        al::startAction(this, "Electric");
}

void BossRaidRivet::exeElectricEnd() {
    if (al::isFirstStep(this))
        al::startAction(this, "ElectricEnd");
    if (al::isMclAnimEnd(this))
        al::setNerve(this, &Wait);
}

void BossRaidRivet::exePull() {
    if (al::isFirstStep(this))
        al::startAction(this, "Pull");

    sead::Vector3f upDir;
    sead::Vector3f frontDir;
    al::calcUpDir(&upDir, this);
    al::calcFrontDir(&frontDir, this);
    const sead::Vector3f& playerPos = al::getPlayerPos(this, 0);
    const sead::Vector3f& trans = al::getTrans(this);
    sead::Vector3f diff = {playerPos.x - trans.x, playerPos.y - trans.y, playerPos.z - trans.z};
    al::tryCalcAngleOnPlaneDegree(&mJointAngle, frontDir, diff, upDir);

    if (al::isActionEnd(this)) {
        al::invalidateCollisionParts(this);
        al::setVelocitySeparateUp(this, diff, 15.0f, 45.0f);
        al::setNerve(this, &PullOut);
    }
}

void BossRaidRivet::exePullOut() {
    if (al::isFirstStep(this)) {
        rs::requestLockOnCapHitReaction(this, mCapTargetInfo, "襲撃ボスの剣を引き抜いた");
        al::startAction(this, "PullOut");
        if (mChainList)
            mChainList->startBlowDown();
    }
    al::addVelocityToGravity(this, 1.5f);
    al::scaleVelocity(this, 0.997f);
    if (al::isGreaterEqualStep(this, 10))
        al::tryOnStageSwitch(this, "SwitchPullOutOn");

    if (al::isActionEnd(this)) {
        ItemGenerator* itemGenerator = mItemGenerator;
        const sead::Vector3f& trans = al::getTrans(this);
        const sead::Quatf& quat = al::getQuat(this);
        itemGenerator->generate(trans, quat);
        al::startHitReaction(this, "消滅");
        if (!mIsPopnKilled) {
            if (mPopn) {
                sead::Vector3f popnPos;
                al::calcConnectTrans(&popnPos, mMtxConnector);
                mPopn->appearByGenerater(popnPos, 0, true, false);
            }
        }
        kill();
    }
}

void BossRaidRivet::appearPopn() {
    if (mPopn) {
        sead::Vector3f pos;
        al::calcConnectTrans(&pos, mMtxConnector);
        mPopn->appearByGenerater(pos, 0, true, false);
    }
}

void BossRaidRivet::reset() {
    mJointAngle = 0.0f;
    mPopn->makeActorDead();
    al::validateCollisionParts(this);
    al::setVelocityZero(this);
    al::setNerve(this, &Wait);
    calcAnim();
    mIsPopnKilled = false;
    appear();
}

void BossRaidRivet::startAnim(const char* animName) {
    al::startAction(this, animName);
    al::setNerve(this, &Demo);
}

void BossRaidRivet::startElectricSign() {
    if (al::isNerve(this, &Pull) || al::isNerve(this, &PullOut) || al::isNerve(this, &ElectricSign))
        return;
    al::setNerve(this, &ElectricSign);
}

void BossRaidRivet::startElectric() {
    if (al::isNerve(this, &Pull) || al::isNerve(this, &PullOut) || al::isNerve(this, &Electric))
        return;
    al::setNerve(this, &Electric);
}

void BossRaidRivet::endElectric() {
    if (al::isNerve(this, &Pull) || al::isNerve(this, &PullOut) || al::isNerve(this, &ElectricEnd))
        return;
    al::setNerve(this, &ElectricEnd);
}

void BossRaidRivet::tryAppearPopn() {
    if (!al::isNerve(this, &PullOut))
        return;
    if (mPopn) {
        sead::Vector3f pos;
        al::calcConnectTrans(&pos, mMtxConnector);
        mPopn->appearByGenerater(pos, 0, true, false);
    }
}

void BossRaidRivet::tryKillPopn() {
    mIsPopnKilled = true;
    if (al::isAlive(mPopn)) {
        al::deleteEffectAll(mPopn);
        al::startHitReaction(mPopn, "死亡");
        mPopn->kill();
    }
}

void BossRaidRivet::killChain() {
    if (mChainList)
        mChainList->makeActorDead();
}

void BossRaidRivet::resetChain() {
    if (al::isAlive(this) && mChainList)
        mChainList->reset();
}
