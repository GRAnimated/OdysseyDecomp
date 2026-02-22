#include "Boss/BossRaid/BossRaid.h"

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Bgm/BgmLineFunction.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Collision/PartsMtxConnector.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Player/PlayerUtil.h"
#include "Library/LiveActor/ActorResourceFunction.h"
#include "Library/LiveActor/ActorSceneFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/LiveActor/LiveActorGroup.h"
#include "Library/Math/ParabolicPath.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Stage/StageSwitchUtil.h"
#include "Library/Thread/FunctorV0M.h"
#include "Library/Yaml/ByamlIter.h"
#include "Library/Yaml/ByamlUtil.h"

#include "Boss/BossRaid/BossRaidRivet.h"
#include "Boss/BossRaid/BossRaidStateBreathAttack.h"
#include "Boss/BossRaid/BossRaidStateGroundAttack.h"
#include "Boss/BossUtil/BossStateTalkDemo.h"
#include "Boss/BossUtil/BossUtil.h"
#include "MapObj/CapMessageShowInfo.h"
#include "Util/DemoUtil.h"
#include "Util/ItemUtil.h"
#include "Util/PlayerDemoUtil.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(BossRaid, EndTalk);
NERVE_IMPL(BossRaid, StartAttack);
NERVE_IMPL(BossRaid, PreDemoBattleStart);
NERVE_IMPL(BossRaid, BattleStartWait);
NERVE_IMPL(BossRaid, DemoBattleEnd);
NERVE_IMPL(BossRaid, GroundAttack);
NERVE_IMPL(BossRaid, BreathAttack);
NERVE_IMPL(BossRaid, Damage);
NERVE_IMPL(BossRaid, UpSign);
NERVE_IMPL(BossRaid, Tired);
NERVE_IMPL(BossRaid, DemoBattleStart);
NERVE_IMPL(BossRaid, Up);
NERVE_IMPL(BossRaid, PreDemoBattleEnd);
NERVE_IMPL(BossRaid, RoarSign);
NERVE_IMPL(BossRaid, Roar);
NERVES_MAKE_NOSTRUCT(BossRaid, EndTalk, StartAttack, PreDemoBattleStart, DemoBattleStart, Roar);
NERVES_MAKE_STRUCT(BossRaid, BattleStartWait, DemoBattleEnd, GroundAttack, BreathAttack, Damage,
                   UpSign, Tired, Up, PreDemoBattleEnd, RoarSign);
}  // namespace

BossRaid::BossRaid(const char* name) : al::LiveActor(name) {}

void BossRaid::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, "BossRaid", nullptr);
    al::initNerve(this, &NrvBossRaid.BattleStartWait, 4);
    al::initJointControllerKeeper(this, 4);
    mLevel = 1;
    if (al::isObjectNameSubStr(info, "Lv2"))
        mLevel = 2;
    mStateTalkDemo = BossStateTalkDemo::createWithEventFlow(this, info, "BossRaid", "BossRaid");
    mStateTalkDemo->setEventReceiver(this);
    al::initNerveState(this, mStateTalkDemo, &NrvBossRaid.DemoBattleEnd, "終了デモ");
    mStateGroundAttack = new BossRaidStateGroundAttack(this, info);
    al::initNerveState(this, mStateGroundAttack, &NrvBossRaid.GroundAttack, "地面攻撃");
    mStateBreathAttack = new BossRaidStateBreathAttack(this, info);
    al::initNerveState(this, mStateBreathAttack, &NrvBossRaid.BreathAttack, "ブレス攻撃");
    mMtxConnector = new al::MtxConnector();
    mInitTrans = al::getTrans(this);
    mInitQuat = al::getQuat(this);
    mParabolicPath = new al::ParabolicPath();
    mArmorActor = al::tryGetSubActor(this, "アーマー");
    mArmorBodyActor = al::tryGetSubActor(this, "弱点");
    mArmorBrokenActor = al::tryGetSubActor(this, "アーマー壊れモデル");
    calcAnim();
    mArmorActor->calcAnim();
    initRivetList(info);
    mDemoCamera = al::initDemoAnimCamera(this, info, "Anim");
    if (!al::tryGetLinksQT(&mInitQuat, &mInitTrans, info, "DemoAfterPlayerPos")) {
        sead::Vector3f offset = {0.0f, 0.0f, 3500.0f};
        al::multVecPose(&mInitTrans, this, offset);
        const sead::Quatf& actorQuat = al::getQuat(this);
        f32 w = actorQuat.w;
        f32 x = actorQuat.x;
        f32 y = actorQuat.y;
        f32 z = actorQuat.z;
        mInitQuat.x = (w * 0.0f + w * 0.0f + x * 0.0f) - y;
        mInitQuat.y = x * 0.0f + (y * 0.0f + z - w * 0.0f);
        mInitQuat.z = y * 0.0f + (x * 0.0f + (z * 0.0f - w * 0.0f));
        mInitQuat.w = (z * 0.0f - w * 0.0f) - x - y * 0.0f;
    }
    mShineActor = rs::tryInitLinkShine(info, "ShineActor", 0);
    al::registerAreaHostMtx(this, info);
    al::hideModelIfShow(this);
    s32 rivetCount = mRivetList->getActorCount();
    for (s32 i = 0; i < rivetCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->killChain();
        rivet->makeActorDead();
    }
    al::invalidateCollisionParts(this);
    al::tryInvalidateCollisionPartsSubActorAll(this);
    al::FunctorV0M<BossRaid*, void (BossRaid::*)()> functor(this,
                                                             &BossRaid::startDemoBattleStart);
    if (al::listenStageSwitchOnStart(this, functor))
        makeActorDead();
    else
        startBattle();
}

void BossRaid::initRivetList(const al::ActorInitInfo& info) {
    if (!al::isExistModelResourceYaml(this, "RivetList", nullptr))
        return;
    al::ByamlIter iter(al::getModelResourceYaml(this, "RivetList", nullptr));
    s32 size = iter.getSize();
    if (size < 1)
        return;
    al::LiveActorGroup* group = new al::LiveActorGroup("リベットリスト", size);
    mRivetList = group;
    for (s32 i = 0; i < group->getMaxActorCount(); i++) {
        BossRaidRivet* rivet = new BossRaidRivet("リベット");
        al::initCreateActorNoPlacementInfo(rivet, info);
        group->registerActor(rivet);
    }
    for (s32 i = 0; i < group->getActorCount(); i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(group->getActor(i));
        al::ByamlIter entryIter;
        iter.tryGetIterByIndex(&entryIter, i);
        sead::Vector3f rotate = sead::Vector3f::zero;
        sead::Vector3f trans = sead::Vector3f::zero;
        const char* joint = nullptr;
        al::tryGetByamlV3f(&rotate, entryIter, "Rotate");
        al::tryGetByamlV3f(&trans, entryIter, "Trans");
        al::tryGetByamlString(&joint, entryIter, "Joint");
        const char* followActorName = nullptr;
        al::LiveActor* followActor = this;
        if (al::tryGetByamlString(&followActorName, entryIter, "FollowActor")) {
            al::LiveActor* sub = al::tryGetSubActor(this, followActorName);
            if (sub)
                followActor = sub;
        }
        rivet->setConnect(followActor, joint, rotate, trans);
        al::ByamlIter chainIter;
        if (entryIter.tryGetIterByKey(&chainIter, "ChainRootPos")) {
            sead::Vector3f chainRotate = sead::Vector3f::zero;
            sead::Vector3f chainTrans = sead::Vector3f::zero;
            const char* chainJoint = nullptr;
            al::tryGetByamlV3f(&chainRotate, chainIter, "Rotate");
            al::tryGetByamlV3f(&chainTrans, chainIter, "Trans");
            al::tryGetByamlString(&chainJoint, chainIter, "Joint");
            const char* chainFollowActorName = nullptr;
            al::LiveActor* chainFollowActor = this;
            if (al::tryGetByamlString(&chainFollowActorName, chainIter, "FollowActor")) {
                al::LiveActor* sub = al::tryGetSubActor(this, chainFollowActorName);
                if (sub)
                    chainFollowActor = sub;
            }
            rivet->setChainRootConnect(chainFollowActor, chainJoint, chainRotate, chainTrans);
        }
        rivet->createChainAndPopn(this, info);
        al::registerSubActorSyncClippingAndHide(this, rivet);
    }
}

void BossRaid::killRivetAll() {
    for (s32 i = 0; i < mRivetList->getActorCount(); i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->killChain();
        rivet->makeActorDead();
    }
}

void BossRaid::invalidateCollisionAll() {
    al::invalidateCollisionParts(this);
    al::tryInvalidateCollisionPartsSubActorAll(this);
}

void BossRaid::startDemoBattleStart() {
    if (rs::isActiveDemo(this))
        return;
    al::invalidateCollisionParts(this);
    al::tryInvalidateCollisionPartsSubActorAll(this);
    al::invalidateClipping(this);
    al::LiveActor::makeActorAlive();
    s32 rivetCount = mRivetList->getActorCount();
    s32 enableCount = getEnableRivetCount();
    for (s32 i = 0; i < enableCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->reset();
    }
    for (s32 i = 0; i < rivetCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->killChain();
        rivet->makeActorDead();
    }
    mArmorActor->makeActorDead();
    mArmorBodyActor->makeActorDead();
    al::setNerve(this, &PreDemoBattleStart);
    mPhase = 3;
}

void BossRaid::startBattle() {
    al::requestCaptureScreenCover(this, 3);
    rs::startBossBattle(this, 9);
    s32 rivetCount = mRivetList->getActorCount();
    for (s32 i = 0; i < rivetCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->killChain();
    }
    al::resetPosition(this);
    al::tryOnStageSwitch(this, "SwitchBossBattleOn");
    al::setNerve(this, &StartAttack);
}

void BossRaid::tryGetFollowTargetInfo(al::LiveActor** actor, sead::Vector3f* rotate,
                                      sead::Vector3f* trans, const char** jointName,
                                      const al::ByamlIter& iter) {
    rotate->z = 0.0f;
    rotate->x = 0.0f;
    rotate->y = 0.0f;
    al::tryGetByamlV3f(rotate, iter, "Rotate");
    trans->z = 0.0f;
    trans->x = 0.0f;
    trans->y = 0.0f;
    al::tryGetByamlV3f(trans, iter, "Trans");
    *jointName = nullptr;
    al::tryGetByamlString(jointName, iter, "Joint");
    const char* followActorName = nullptr;
    al::LiveActor* result = this;
    if (al::tryGetByamlString(&followActorName, iter, "FollowActor"))
        result = al::tryGetSubActor(this, followActorName);
    if (result)
        *actor = result;
    else
        *actor = this;
}

void BossRaid::endBattleStartDemo() {
    f32 frameMax = al::getActionFrameMax(this);
    al::setActionFrame(this, frameMax);
    rs::showDemoPlayer(this);
    al::requestCancelCameraInterpole(this, 0);
    rs::replaceDemoPlayer(this, mInitTrans, mInitQuat);
    rs::requestEndDemoWithPlayerCinemaFrame(this);
    al::endCamera(this, mDemoCamera, 0, false);
    rs::saveShowDemoBossBattleStart(this, 9, mLevel);
    startBattle();
}

void BossRaid::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isNerve(this, &NrvBossRaid.BreathAttack) || al::isNerve(this, &NrvBossRaid.Up) ||
        (al::isNerve(this, &NrvBossRaid.GroundAttack) &&
         mStateGroundAttack->isElectric())) {
        if (al::isSensorEnemyAttack(self))
            al::sendMsgEnemyAttack(other, self);
    }
}

bool BossRaid::isElectric() const {
    if (al::isNerve(this, &NrvBossRaid.BreathAttack))
        return true;
    if (al::isNerve(this, &NrvBossRaid.Up))
        return true;
    if (al::isNerve(this, &NrvBossRaid.GroundAttack))
        return mStateGroundAttack->isElectric();
    return false;
}

bool BossRaid::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                          al::HitSensor* self) {
    if ((rs::isMsgPlayerHipDropDemoTrigger(message) || rs::isMsgCapHipDrop(message)) &&
        al::isSensorHost(self, mArmorBodyActor) && !al::isAlive(mArmorActor) &&
        (al::isNerve(this, &NrvBossRaid.Tired) || al::isNerve(this, &NrvBossRaid.UpSign))) {
        al::setNerve(this, &NrvBossRaid.Damage);
        return true;
    }
    if (rs::isMsgPlayerDisregardHomingAttack(message) && al::isSensorHost(self, mArmorBodyActor))
        return true;
    if (al::isMsgPlayerTouch(message) &&
        (al::isNerve(this, &NrvBossRaid.BreathAttack) ||
         al::isNerve(this, &NrvBossRaid.Up) ||
         (al::isNerve(this, &NrvBossRaid.GroundAttack) && mStateGroundAttack->isElectric()))) {
        al::sendMsgEnemyAttack(other, self);
        return true;
    }
    return false;
}

bool BossRaid::isEnableDamage() const {
    if (al::isAlive(mArmorActor))
        return false;
    if (al::isNerve(this, &NrvBossRaid.Tired))
        return true;
    return al::isNerve(this, &NrvBossRaid.UpSign);
}

bool BossRaid::receiveEvent(const al::EventFlowEventData* event) {
    if (!al::isEventName(event, "BossRaidAppearMoon"))
        return false;
    if (mShineActor) {
        rs::appearShineAndJoinBossDemo(mShineActor, "DemoBattleEndBossRaid", al::getQuat(this),
                                       al::getTrans(this));
    }
    return true;
}

void BossRaid::calcAnim() {
    al::LiveActor::calcAnim();
    if (rs::isActiveDemo(this) && mMtxConnector->isConnecting()) {
        sead::Quatf quat;
        sead::Vector3f trans;
        mMtxConnector->multQT(&quat, &trans, nullptr);
        rs::replaceDemoPlayer(this, trans, quat);
    }
    if (al::isDead(mArmorActor))
        mArmorActor->calcAnim();
}

void BossRaid::updatePlayerPose() {
    if (!rs::isActiveDemo(this))
        return;
    if (!mMtxConnector->isConnecting())
        return;
    sead::Quatf quat;
    sead::Vector3f trans;
    mMtxConnector->multQT(&quat, &trans, nullptr);
    rs::replaceDemoPlayer(this, trans, quat);
}

// NON_MATCHING: extra callee-saved register x23 (compiler pre-loads getEnableRivetCount constants
//               w21=4, w22=3 for two inline loops); branch direction at mHintWeakPointThreshold
void BossRaid::control() {
    mStateGroundAttack->updateBullet();
    s32 enableCount = getEnableRivetCount();
    s32 i = 0;
    for (; i < enableCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        if (!rivet->isPullOut())
            break;
    }
    if (i >= enableCount) {
        if (al::isAlive(mArmorActor)) {
            al::startHitReaction(this, "兜破壊");
            mArmorBrokenActor->appear();
            mArmorActor->kill();
        }
    }
    if (al::isNerve(this, &NrvBossRaid.UpSign)) {
        if (al::isStep(this, 40) && !mHasShownHintElectricSign) {
            mHasShownHintElectricSign = true;
            if (mLevel <= 1)
                rs::showCapMessageBossHint(this, "BossRaid_HintElectricSign", 90, 0);
        }
    }
    if (!mHasShownHintRivet) {
        if (mPhase == 3 && mDamageCount >= 2) {
            s32 j = 0;
            s32 enableCount2 = getEnableRivetCount();
            for (; j < enableCount2; j++) {
                BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(j));
                if (rivet->isPullOut())
                    break;
            }
            if (j >= enableCount2) {
                if (al::isNerve(this, &NrvBossRaid.Tired)) {
                    if (mLevel <= 1)
                        rs::showCapMessageBossHint(this, "BossRaid_HintRivet", 90, 0);
                    mHasShownHintRivet = true;
                }
            }
        }
    }
    if (mPhase == 3 && al::isDead(mArmorActor)) {
        sead::Vector3f weakPointPos = sead::Vector3f::zero;
        al::calcJointPos(&weakPointPos, this, "WeakPoint");
        sead::Vector3f diff = al::getPlayerPos(this, 0) - weakPointPos;
        if (diff.length() <= 500.0f) {
            if (al::isNerve(this, &NrvBossRaid.Tired)) {
                if (mHintWeakPointThreshold == -1) {
                    if (mLevel < 2) {
                        rs::showCapMessageBossHint(this, "BossRaid_HintWeakPoint", 90, 30);
                    }
                    mHintWeakPointThreshold = mDamageCount;
                }
                if (!mHasShownHintHipDrop && mHintWeakPointThreshold < mDamageCount) {
                    if (mLevel <= 1)
                        rs::showCapMessageBossHint(this, "BossRaid_HintHipDrop", 90, 30);
                    mHasShownHintHipDrop = true;
                }
            }
        }
    }
    if (!mHasShownHintLast && al::isDead(mArmorActor)) {
        sead::Vector3f weakPointPos2 = sead::Vector3f::zero;
        al::calcJointPos(&weakPointPos2, this, "WeakPoint");
        sead::Vector3f diff2 = al::getPlayerPos(this, 0) - weakPointPos2;
        if (diff2.length() <= 500.0f) {
            if (al::isNerve(this, &NrvBossRaid.Tired) ||
                al::isNerve(this, &NrvBossRaid.UpSign)) {
                if (mPhase == 1) {
                    if (mLevel <= 1)
                        rs::showCapMessageBossHint(this, "BossRaid_HintLast", 90, 0);
                    mHasShownHintLast = true;
                }
            }
        }
    }
}

bool BossRaid::isPullOutRivetAll() const {
    s32 enableCount = getEnableRivetCount();
    for (s32 i = 0; i < enableCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        if (!rivet->isPullOut())
            return false;
    }
    return true;
}

void BossRaid::hintCapMessage(const char* message, s32 unused) {
    if (mLevel <= 1)
        rs::showCapMessageBossHint(this, message, 90, unused);
}

bool BossRaid::isPullOutRivetAny() const {
    s32 enableCount = getEnableRivetCount();
    for (s32 i = 0; i < enableCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        if (rivet->isPullOut())
            return true;
    }
    return false;
}

bool BossRaid::isNearWeakPoint() const {
    sead::Vector3f weakPointPos = sead::Vector3f::zero;
    al::calcJointPos(&weakPointPos, this, "WeakPoint");
    sead::Vector3f diff = al::getPlayerPos(this, 0) - weakPointPos;
    return diff.length() <= 500.0f;
}

void BossRaid::damageCapMessage() {
    const char* msg;
    if (mPhase == 1) {
        msg = "BossRaid_Damage2";
    } else {
        if (mPhase != 2)
            return;
        msg = "BossRaid_Damage1";
    }
    rs::showCapMessageBossDamage(this, msg, 90, 30);
}

bool BossRaid::isFirstDemo() const {
    return !rs::isAlreadyShowDemoBossBattleStart(this, 9, mLevel);
}

bool BossRaid::isEnableSkipDemo() const {
    return al::isNerve(this, &DemoBattleStart);
}

void BossRaid::skipDemo() {
    f32 frameMax = al::getActionFrameMax(this);
    al::setActionFrame(this, frameMax);
    rs::showDemoPlayer(this);
    al::requestCancelCameraInterpole(this, 0);
    rs::replaceDemoPlayer(this, mInitTrans, mInitQuat);
    rs::requestEndDemoWithPlayerCinemaFrame(this);
    al::endCamera(this, mDemoCamera, 0, false);
    rs::saveShowDemoBossBattleStart(this, 9, mLevel);
    startBattle();
}

void BossRaid::exeBattleStartWait() {}

void BossRaid::exePreDemoBattleStart() {
    if (!al::isGreaterEqualStep(this, 60))
        return;
    if (rs::requestStartDemoWithPlayerCinemaFrame(this, false)) {
        rs::requestValidateDemoSkip(this, this);
        setUpDemoBattleStart();
        rs::forcePutOnDemoCap(this);
        al::setNerve(this, &DemoBattleStart);
    }
}

void BossRaid::setUpDemoBattleStart() {
    al::showModelIfHide(this);
    al::showModelIfHide(mArmorBodyActor);
    mArmorActor->makeActorAlive();
    mArmorBodyActor->makeActorAlive();
    s32 enableCount = getEnableRivetCount();
    for (s32 i = 0; i < enableCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->makeActorAlive();
        al::showModelIfHide(rivet);
    }
    mStateGroundAttack->killBulletAll();
    rs::addDemoSubActor(this);
    al::startAction(mArmorActor, "Wait");
}

void BossRaid::exeDemoBattleStart() {
    if (al::isFirstStep(this)) {
        al::requestCancelCameraInterpole(this, 0);
        al::tryOnStageSwitch(this, "SwitchDemoBattleStartOn");
        al::tryOffStageSwitch(this, "SwitchDemoBattleEndOn");
        al::startAnimCamera(this, mDemoCamera, "DemoBattleStart", 0);
        al::startAction(this, "DemoBattleStart");
        al::startAction(mArmorBodyActor, "DemoBattleStart");
        rs::startActionDemoPlayer(this, "BattleWait");
        al::startAction(mArmorActor, "DemoBattleStart");
        s32 enableCount = getEnableRivetCount();
        for (s32 i = 0; i < enableCount; i++) {
            BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
            rivet->startAnim("DemoBattleStart");
        }
        rs::setDemoInfoDemoName(this, "バトル開始デモ(襲撃ボス)");
        rs::hideDemoPlayer(this);
    }
    if (al::isStep(this, 1)) {
        s32 enableCount = getEnableRivetCount();
        for (s32 i = 0; i < enableCount; i++) {
            BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
            rivet->resetChain();
        }
    }
    if (!al::isActionEnd(this))
        return;
    f32 frameMax = al::getActionFrameMax(this);
    al::setActionFrame(this, frameMax);
    rs::showDemoPlayer(this);
    al::requestCancelCameraInterpole(this, 0);
    rs::replaceDemoPlayer(this, mInitTrans, mInitQuat);
    rs::requestEndDemoWithPlayerCinemaFrame(this);
    al::endCamera(this, mDemoCamera, 0, false);
    rs::saveShowDemoBossBattleStart(this, 9, mLevel);
    startBattle();
}

void BossRaid::startActionMain(const char* actionName) {
    al::startAction(this, actionName);
    al::startAction(mArmorBodyActor, actionName);
}

// NON_MATCHING: mPhase load scheduled after count loads; w9/w10 register swap
s32 BossRaid::getEnableRivetCount() const {
    s32 count = mRivetList->getActorCount();
    s32 count4 = count <= 4 ? count : 4;
    s32 count3 = count <= 3 ? count : 3;
    if (mPhase == 3)
        return count3;
    if (mPhase == 2)
        return count4;
    return count;
}

void BossRaid::resetChainAll() {
    s32 enableCount = getEnableRivetCount();
    for (s32 i = 0; i < enableCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->resetChain();
    }
}

// NON_MATCHING: w10/w11 register rename in loop condition
void BossRaid::exeStartAttack() {
    if (!al::isStep(this, 1))
        return;
    for (s32 i = 0; i < getEnableRivetCount(); i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->resetChain();
    }
    al::setNerve(this, &NrvBossRaid.BreathAttack);
}

void BossRaid::exeBreathAttack() {
    if (al::updateNerveState(this))
        al::setNerve(this, &NrvBossRaid.GroundAttack);
}

void BossRaid::exeGroundAttack() {
    if (al::updateNerveState(this))
        al::setNerve(this, &NrvBossRaid.Tired);
}

void BossRaid::exeTired() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Tired");
        al::startAction(mArmorBodyActor, "Tired");
        al::startAction(mArmorActor, "Wait");
        mDamageCount++;
    }
    if (al::isStep(this, 30)) {
        s32 enableCount = getEnableRivetCount();
        for (s32 i = 0; i < enableCount; i++) {
            BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
            rivet->tryAppearPopn();
        }
    }
    if (al::isGreaterEqualStep(this, 450))
        al::setNerve(this, &NrvBossRaid.UpSign);
}

void BossRaid::appearRivetPopnAll() {
    s32 enableCount = getEnableRivetCount();
    for (s32 i = 0; i < enableCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->tryAppearPopn();
    }
}

void BossRaid::exeUpSign() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "UpSign");
        al::startAction(mArmorBodyActor, "UpSign");
        al::startAction(mArmorActor, "ElectricSign");
        s32 enableCount = getEnableRivetCount();
        for (s32 i = 0; i < enableCount; i++) {
            BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
            rivet->startElectricSign();
        }
    }
    if (al::isActionEnd(this))
        al::setNerve(this, &NrvBossRaid.Up);
}

void BossRaid::startElectricSignParts() {
    al::startAction(mArmorActor, "ElectricSign");
    s32 enableCount = getEnableRivetCount();
    for (s32 i = 0; i < enableCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->startElectricSign();
    }
}

void BossRaid::exeUp() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Up");
        al::startAction(mArmorBodyActor, "Up");
        al::startAction(mArmorActor, "Electric");
        s32 enableCount = getEnableRivetCount();
        for (s32 i = 0; i < enableCount; i++) {
            BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
            rivet->tryKillPopn();
        }
    }
    if (al::isStep(this, 60)) {
        al::invalidateCollisionParts(this);
        al::tryInvalidateCollisionPartsSubActorAll(this);
    }
    if (al::isActionEnd(this))
        al::setNerve(this, &StartAttack);
}

void BossRaid::killRivetPopnAll() {
    s32 enableCount = getEnableRivetCount();
    for (s32 i = 0; i < enableCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->tryKillPopn();
    }
}

void BossRaid::startAttack() {
    al::setNerve(this, &StartAttack);
}

void BossRaid::exeDamage() {
    if (al::isFirstStep(this)) {
        const char* actionName = mPhase >= 2 ? "Damage" : "DamageLast";
        mPhase--;
        al::startAction(this, actionName);
        al::startAction(mArmorBodyActor, actionName);
        if (mPhase == 1)
            rs::showCapMessageBossDamage(this, "BossRaid_Damage2", 90, 0);
        else if (mPhase == 2)
            rs::showCapMessageBossDamage(this, "BossRaid_Damage1", 90, 0);
        s32 enableCount = getEnableRivetCount();
        for (s32 i = 0; i < enableCount; i++) {
            BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
            rivet->tryKillPopn();
        }
        mMtxConnector->clear();
        if (mPhase <= 0)
            al::stopAllBgm(this, -1);
    }
    if (al::isStep(this, 7))
        al::startHitReaction(this, "ダメージ");
    if (mPhase <= 0) {
        if (!al::isActionEnd(this))
            return;
        al::setNerve(this, &NrvBossRaid.PreDemoBattleEnd);
        return;
    }
    if (al::isGreaterEqualStep(this, 11) && !rs::isActiveDemo(this) &&
        rs::requestStartDemoWithPlayer(this, false)) {
        if (rs::isPlayerOnActor(this)) {
            const sead::Matrix34f* jointMtx = al::getJointMtxPtr(this, "ArmorPos");
            mMtxConnector->init(jointMtx);
            mMtxConnector->setBaseQuatTrans(rs::getDemoPlayerQuat(this),
                                            rs::getDemoPlayerTrans(this));
        } else {
            mMtxConnector->clear();
        }
        rs::addDemoSubActor(this);
    }
    if (rs::isActiveDemo(this)) {
        if (al::isActionEnd(this))
            al::setNerve(this, &NrvBossRaid.RoarSign);
    }
}

void BossRaid::exeRoarSign() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "RoarSign");
        al::startAction(mArmorBodyActor, "RoarSign");
        rs::startActionDemoPlayer(this, "AreaWaitSearch");
    }
    if (al::isActionEnd(this))
        al::setNerve(this, &Roar);
}

// NON_MATCHING: extra callee-saved register x23 (compiler pre-loads getEnableRivetCount constants
//               w21=4, w22=3 for multiple inline loops); loop body structural differences
void BossRaid::exeRoar() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Roar");
        al::startAction(mArmorBodyActor, "Roar");
    }
    if (!al::isLessStep(this, 0x26)) {
        if (al::isStep(this, 38)) {
            rs::startActionDemoPlayer(this, "JumpBack");
            mParabolicPath->initFromUpVectorAddHeight(al::getPlayerPos(this, 0), mInitTrans,
                                                      sead::Vector3f::ey, 2000.0f);
        } else if (al::isLessStep(this, 0xA8)) {
            mMtxConnector->clear();
            f32 rate = al::calcNerveRate(this, 0x26, 168);
            sead::Vector3f pos;
            mParabolicPath->calcPosition(&pos, rate);
            rs::replaceDemoPlayer(this, pos, mInitQuat);
        } else if (rs::isActiveDemo(this)) {
            rs::requestEndDemoWithPlayer(this);
        }
    }
    if (al::isStep(this, 60)) {
        mArmorActor->appear();
        s32 enableCount = getEnableRivetCount();
        for (s32 i = 0; i < enableCount; i++) {
            BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
            rivet->reset();
        }
    }
    if (al::isStep(this, 61)) {
        s32 enableCount = getEnableRivetCount();
        for (s32 i = 0; i < enableCount; i++) {
            BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
            rivet->resetChain();
        }
        al::startAction(mArmorActor, "Roar");
        for (s32 i = 0; i < enableCount; i++) {
            BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
            rivet->startAnim("Roar");
        }
    }
    if (al::isStep(this, 60)) {
        al::invalidateCollisionParts(this);
        al::tryInvalidateCollisionPartsSubActorAll(this);
    }
    if (!rs::isActiveDemo(this)) {
        if (al::isActionEnd(this) && al::isAlive(mArmorActor))
            al::setNerve(this, &NrvBossRaid.BreathAttack);
    }
}

void BossRaid::resetRivetAll() {
    s32 enableCount = getEnableRivetCount();
    for (s32 i = 0; i < enableCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->reset();
    }
}

void BossRaid::startRoarAnimParts() {
    al::startAction(mArmorActor, "Roar");
    s32 enableCount = getEnableRivetCount();
    for (s32 i = 0; i < enableCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->startAnim("Roar");
    }
}

void BossRaid::exePreDemoBattleEnd() {
    if (mStateTalkDemo->tryStart("BattleEnd")) {
        setUpDemoBattleEnd();
        al::tryOnStageSwitch(this, "SwitchDemoBattleEndOn");
        al::tryOffStageSwitch(this, "SwitchBossBattleOn");
        al::setNerve(this, &NrvBossRaid.DemoBattleEnd);
    }
}

void BossRaid::setUpDemoBattleEnd() {
    al::showModelIfHide(this);
    al::showModelIfHide(mArmorBodyActor);
    mArmorActor->makeActorDead();
    mArmorBodyActor->makeActorAlive();
    s32 rivetCount = mRivetList->getActorCount();
    for (s32 i = 0; i < rivetCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->killChain();
        rivet->makeActorDead();
    }
    mStateGroundAttack->killBulletAll();
    al::invalidateCollisionParts(this);
    al::tryInvalidateCollisionPartsSubActorAll(this);
    rs::replaceDemoPlayer(this, mInitTrans, mInitQuat);
}

void BossRaid::exeDemoBattleEnd() {
    if (!al::updateNerveState(this))
        return;
    al::tryOnSwitchDeadOn(this);
    al::hideModelIfShow(mArmorBodyActor);
    al::invalidateCollisionParts(this);
    al::tryInvalidateCollisionPartsSubActorAll(this);
    al::LiveActor* battleCollision = al::tryGetSubActor(this, "戦闘後コリジョン");
    if (battleCollision)
        al::validateCollisionParts(battleCollision);
    if (mShineActor)
        rs::endShineBossDemoAndStartFall(mShineActor, 1000.0f);
    rs::endBossBattle(this, 9);
    al::setNerve(this, &EndTalk);
}

void BossRaid::exeEndTalk() {
    if (!al::isFirstStep(this))
        return;
    al::startAction(this, "BattleEndWait");
    al::startAction(mArmorBodyActor, "BattleEndWait");
}

// NON_MATCHING: instruction scheduling differs; target defers csel after sub
s32 BossRaid::getShotLevel() const {
    s32 levelOffset = mLevel == 2 ? 3 : 0;
    return 3 - mPhase + levelOffset;
}

void BossRaid::startElectricParts() {
    al::startAction(mArmorActor, "Electric");
    s32 enableCount = getEnableRivetCount();
    for (s32 i = 0; i < enableCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->startElectric();
    }
}

void BossRaid::endElectricParts() {
    al::startAction(mArmorActor, "ElectricEnd");
    s32 enableCount = getEnableRivetCount();
    for (s32 i = 0; i < enableCount; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->endElectric();
    }
}

void BossRaid::validateCollisionAll() {
    al::validateCollisionParts(this);
    al::tryValidateCollisionPartsSubActorAll(this);
    al::LiveActor* battleCollision = al::tryGetSubActor(this, "戦闘後コリジョン");
    if (battleCollision)
        al::invalidateCollisionParts(battleCollision);
}

void BossRaid::killElectoricAll() {
    mStateGroundAttack->killBulletAll();
}

void BossRaid::killChainAll() {
    s32 count = mRivetList->getActorCount();
    for (s32 i = 0; i < count; i++) {
        BossRaidRivet* rivet = static_cast<BossRaidRivet*>(mRivetList->getActor(i));
        rivet->killChain();
    }
}

void BossRaid::appearAndHideRivetAll() {
    s32 count = mRivetList->getActorCount();
    for (s32 i = 0; i < count; i++) {
        al::hideModelIfShow(mRivetList->getActor(i));
    }
}

void BossRaid::showRivetAll() {
    s32 enableCount = getEnableRivetCount();
    for (s32 i = 0; i < enableCount; i++) {
        al::LiveActor* rivet = mRivetList->getActor(i);
        rivet->makeActorAlive();
        al::showModelIfHide(rivet);
    }
}

void BossRaid::startDemoBattleEnd() {
    if (rs::isActiveDemo(this))
        return;
    al::invalidateClipping(this);
    al::LiveActor::makeActorAlive();
    al::stopAllBgm(this, -1);
    al::setNerve(this, &NrvBossRaid.PreDemoBattleEnd);
}
