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
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorResourceFunction.h"
#include "Library/LiveActor/ActorSceneFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/LiveActor/LiveActorGroup.h"
#include "Library/Math/ParabolicPath.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Player/PlayerUtil.h"
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
    mStateTalkDemo->setEnableSkipDemo(false);

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
        al::multVecPose(&mInitTrans, this, sead::Vector3f::ez * 3500.0f);
        mInitQuat.setMul(al::getQuat(this), sead::Quatf(0.0f, 0.0f, 1.0f, 0.0f));
    }

    mShineActor = rs::tryInitLinkShine(info, "ShineActor", 0);
    al::registerAreaHostMtx(this, info);
    al::hideModelIfShow(this);

    killRivetAll();

    al::invalidateCollisionParts(this);
    al::tryInvalidateCollisionPartsSubActorAll(this);

    al::FunctorV0M<BossRaid*, void (BossRaid::*)()> functor(this, &BossRaid::startDemoBattleStart);
    if (al::listenStageSwitchOnStart(this, functor))
        makeActorDead();
    else
        startBattle();
}

// NON_MATCHING
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

    for (s32 i = 0; i < mRivetList->getActorCount(); i++) {
        BossRaidRivet* rivet = (BossRaidRivet*)mRivetList->getActor(i);

        al::ByamlIter entryIter;
        iter.tryGetIterByIndex(&entryIter, i);

        sead::Vector3f rotate = sead::Vector3f::zero;
        sead::Vector3f trans = sead::Vector3f::zero;
        const char* joint = nullptr;
        al::LiveActor* followActor = nullptr;

        tryGetFollowTargetInfo(&followActor, &trans, &rotate, &joint, entryIter);
        rivet->setConnect(followActor, joint, rotate, trans);

        al::ByamlIter chainIter;
        if (entryIter.tryGetIterByKey(&chainIter, "ChainRootPos")) {
            sead::Vector3f chainRotate = sead::Vector3f::zero;
            sead::Vector3f chainTrans = sead::Vector3f::zero;
            const char* chainJoint = nullptr;
            al::LiveActor* chainFollowActor = nullptr;

            tryGetFollowTargetInfo(&chainFollowActor, &chainTrans, &chainRotate, &chainJoint,
                                   chainIter);

            rivet->setChainRootConnect(chainFollowActor, chainJoint, chainRotate, chainTrans);
        }

        rivet->createChainAndPopn(this, info);
        al::registerSubActorSyncClippingAndHide(this, rivet);
    }
}

void BossRaid::killRivetAll() {
    for (s32 i = 0; i < mRivetList->getActorCount(); i++) {
        BossRaidRivet* rivet = (BossRaidRivet*)mRivetList->getActor(i);
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
    resetRivetAll();
    killRivetAll();
    mArmorActor->makeActorDead();
    mArmorBodyActor->makeActorDead();
    al::setNerve(this, &PreDemoBattleStart);
    mPhase = 3;
}

void BossRaid::startBattle() {
    al::requestCaptureScreenCover(this, 3);
    rs::startBossBattle(this, 9);
    killChainAll();
    al::resetPosition(this);
    al::tryOnStageSwitch(this, "SwitchBossBattleOn");
    startAttack();
}

al::LiveActor* BossRaid::tryGetFollowTargetInfo(al::LiveActor** actor, sead::Vector3f* trans,
                                                sead::Vector3f* rotate, const char** jointName,
                                                const al::ByamlIter& iter) {
    rotate->set(sead::Vector3f::zero);
    al::tryGetByamlV3f(rotate, iter, "Rotate");

    trans->set(sead::Vector3f::zero);
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

    return result;
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
    if (isElectric()) {
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

bool BossRaid::receiveMsg(const al::SensorMsg* message, al::HitSensor* other, al::HitSensor* self) {
    if ((rs::isMsgPlayerHipDropDemoTrigger(message) || rs::isMsgCapHipDrop(message)) &&
        al::isSensorHost(self, mArmorBodyActor) && isEnableDamage()) {
        al::setNerve(this, &NrvBossRaid.Damage);
        return true;
    }

    if (rs::isMsgPlayerDisregardHomingAttack(message) && al::isSensorHost(self, mArmorBodyActor))
        return true;

    if (al::isMsgPlayerTouch(message) && isElectric()) {
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

// NON_MATCHING: still needs cleanup
void BossRaid::control() {
    mStateGroundAttack->updateBullet();

    const s32 enableCount = getEnableRivetCount();
    if (enableCount > 0 && isPullOutRivetAll()) {
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
                rs::showCapMessageBossHint(this, "BossRaid_HintElectricSign", 90, 30);
        }
    }

    if (!mHasShownHintRivet && mPhase == 3 && mDamageCount >= 2) {
        if (!isPullOutRivetAny() && al::isNerve(this, &NrvBossRaid.Tired)) {
            if (mLevel <= 1)
                rs::showCapMessageBossHint(this, "BossRaid_HintRivet", 90, 30);

            mHasShownHintRivet = true;
        }
    }

    if (mPhase == 3 && al::isDead(mArmorActor) && isNearWeakPoint()) {
        if (al::isNerve(this, &NrvBossRaid.Tired)) {
            if (mHintWeakPointThreshold == -1) {
                if (mLevel < 2)
                    rs::showCapMessageBossHint(this, "BossRaid_HintWeakPoint", 90, 30);

                mHintWeakPointThreshold = mDamageCount;
            }

            if (!mHasShownHintHipDrop && mHintWeakPointThreshold < mDamageCount) {
                if (mLevel <= 1)
                    rs::showCapMessageBossHint(this, "BossRaid_HintHipDrop", 90, 30);

                mHasShownHintHipDrop = true;
            }
        }
    }

    if (!mHasShownHintLast && al::isDead(mArmorActor) && isNearWeakPoint()) {
        if (mPhase == 1 &&
            (al::isNerve(this, &NrvBossRaid.Tired) || al::isNerve(this, &NrvBossRaid.UpSign))) {
            if (mLevel <= 1)
                rs::showCapMessageBossHint(this, "BossRaid_HintLast", 90, 0);

            mHasShownHintLast = true;
        }
    }
}

bool BossRaid::isPullOutRivetAll() const {
    for (s32 i = 0; i < getEnableRivetCount(); i++)
        if (!((BossRaidRivet*)mRivetList->getActor(i))->isPullOut())
            return false;
    return true;
}

void BossRaid::hintCapMessage(const char* message, s32 unused) {
    if (mLevel > 1)
        return;

    rs::showCapMessageBossHint(this, message, 90, unused);
}

bool BossRaid::isPullOutRivetAny() const {
    for (s32 i = 0; i < getEnableRivetCount(); i++)
        if (((BossRaidRivet*)mRivetList->getActor(i))->isPullOut())
            return true;
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

    showRivetAll();

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
        for (s32 i = 0; i < getEnableRivetCount(); i++)
            ((BossRaidRivet*)mRivetList->getActor(i))->startAnim("DemoBattleStart");
        rs::setDemoInfoDemoName(this, "バトル開始デモ(襲撃ボス)");
        rs::hideDemoPlayer(this);
    }

    if (al::isStep(this, 1))
        resetChainAll();

    if (al::isActionEnd(this)) {
        // Even though this is exactly skipDemo(), it doesn't inline if called here
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
}

void BossRaid::startActionMain(const char* actionName) {
    al::startAction(this, actionName);
    al::startAction(mArmorBodyActor, actionName);
}

s32 BossRaid::getEnableRivetCount() const {
    s32 count = mRivetList->getActorCount();

    if (mPhase == 3)
        return std::min(count, 3);  // sead::Mathi::min does not match

    if (mPhase == 2)
        return std::min(count, 4);

    return count;
}

void BossRaid::resetChainAll() {
    for (s32 i = 0; i < getEnableRivetCount(); i++) {
        BossRaidRivet* rivet = (BossRaidRivet*)mRivetList->getActor(i);
        rivet->resetChain();
    }
}

void BossRaid::exeStartAttack() {
    if (!al::isStep(this, 1))
        return;
    resetChainAll();
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
        for (s32 i = 0; i < getEnableRivetCount(); i++) {
            BossRaidRivet* rivet = (BossRaidRivet*)mRivetList->getActor(i);
            rivet->tryAppearPopn();
        }
    }
    if (al::isGreaterEqualStep(this, 450))
        al::setNerve(this, &NrvBossRaid.UpSign);
}

void BossRaid::appearRivetPopnAll() {
    for (s32 i = 0; i < getEnableRivetCount(); i++) {
        BossRaidRivet* rivet = (BossRaidRivet*)mRivetList->getActor(i);
        rivet->tryAppearPopn();
    }
}

void BossRaid::exeUpSign() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "UpSign");
        al::startAction(mArmorBodyActor, "UpSign");
        al::startAction(mArmorActor, "ElectricSign");
        for (s32 i = 0; i < getEnableRivetCount(); i++) {
            BossRaidRivet* rivet = (BossRaidRivet*)mRivetList->getActor(i);
            rivet->startElectricSign();
        }
    }
    if (al::isActionEnd(this))
        al::setNerve(this, &NrvBossRaid.Up);
}

void BossRaid::startElectricSignParts() {
    al::startAction(mArmorActor, "ElectricSign");
    for (s32 i = 0; i < getEnableRivetCount(); i++) {
        BossRaidRivet* rivet = (BossRaidRivet*)mRivetList->getActor(i);
        rivet->startElectricSign();
    }
}

void BossRaid::exeUp() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Up");
        al::startAction(mArmorBodyActor, "Up");
        al::startAction(mArmorActor, "Electric");
        killRivetPopnAll();
    }
    if (al::isStep(this, 60)) {
        al::invalidateCollisionParts(this);
        al::tryInvalidateCollisionPartsSubActorAll(this);
    }
    if (al::isActionEnd(this))
        startAttack();
}

void BossRaid::killRivetPopnAll() {
    for (s32 i = 0; i < getEnableRivetCount(); i++) {
        BossRaidRivet* rivet = (BossRaidRivet*)mRivetList->getActor(i);
        rivet->tryKillPopn();
    }
}

void BossRaid::startAttack() {
    al::setNerve(this, &StartAttack);
}

void BossRaid::exeDamage() {
    if (al::isFirstStep(this)) {
        const char* actionName;
        if (--mPhase < 1)
            actionName = "DamageLast";
        else
            actionName = "Damage";
        al::startAction(this, actionName);
        al::startAction(mArmorBodyActor, actionName);

        if (mPhase == 2)
            rs::showCapMessageBossDamage(this, "BossRaid_Damage2", 90, 30);
        else if (mPhase == 1)
            rs::showCapMessageBossDamage(this, "BossRaid_Damage1", 90, 30);

        killRivetPopnAll();
        mMtxConnector->clear();
        if (mPhase <= 0)
            al::stopAllBgm(this, -1);
    }

    if (al::isStep(this, 7))
        al::startHitReaction(this, "ダメージ");

    if (mPhase <= 0) {
        if (al::isActionEnd(this))
            al::setNerve(this, &NrvBossRaid.PreDemoBattleEnd);
        return;
    }

    if (al::isGreaterEqualStep(this, 11) && !rs::isActiveDemo(this) &&
        rs::requestStartDemoWithPlayer(this, false)) {
        if (rs::isPlayerOnActor(this)) {
            mMtxConnector->init(al::getJointMtxPtr(this, "ArmorPos"));
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
            al::ParabolicPath* path = mParabolicPath;
            f32 rate = al::calcNerveRate(this, 0x26, 168);
            sead::Vector3f pos;
            path->calcPosition(&pos, rate);
            rs::replaceDemoPlayer(this, pos, mInitQuat);
        } else if (rs::isActiveDemo(this)) {
            rs::requestEndDemoWithPlayer(this);
        }
    }
    if (al::isStep(this, 60)) {
        mArmorActor->appear();
        for (s32 i = 0; i < getEnableRivetCount(); i++) {
            BossRaidRivet* rivet = (BossRaidRivet*)mRivetList->getActor(i);
            rivet->reset();
        }
    }
    if (al::isStep(this, 61)) {
        resetChainAll();
        startRoarAnimParts();
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
    for (s32 i = 0; i < getEnableRivetCount(); i++) {
        BossRaidRivet* rivet = (BossRaidRivet*)mRivetList->getActor(i);
        rivet->reset();
    }
}

void BossRaid::startRoarAnimParts() {
    al::startAction(mArmorActor, "Roar");
    for (s32 i = 0; i < getEnableRivetCount(); i++) {
        BossRaidRivet* rivet = (BossRaidRivet*)mRivetList->getActor(i);
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
    for (s32 i = 0; i < mRivetList->getActorCount(); i++) {
        BossRaidRivet* rivet = (BossRaidRivet*)mRivetList->getActor(i);
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

s32 BossRaid::getShotLevel() const {
    return (3 - mPhase) + (mLevel == 2 ? 3 : 0);
}

void BossRaid::startElectricParts() {
    al::startAction(mArmorActor, "Electric");
    for (s32 i = 0; i < getEnableRivetCount(); i++) {
        BossRaidRivet* rivet = (BossRaidRivet*)mRivetList->getActor(i);
        rivet->startElectric();
    }
}

void BossRaid::endElectricParts() {
    al::startAction(mArmorActor, "ElectricEnd");
    for (s32 i = 0; i < getEnableRivetCount(); i++) {
        BossRaidRivet* rivet = (BossRaidRivet*)mRivetList->getActor(i);
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
    for (s32 i = 0; i < mRivetList->getActorCount(); i++)
        ((BossRaidRivet*)mRivetList->getActor(i))->killChain();
}

void BossRaid::appearAndHideRivetAll() {
    for (s32 i = 0; i < mRivetList->getActorCount(); i++)
        al::hideModelIfShow(mRivetList->getActor(i));
}

void BossRaid::showRivetAll() {
    for (s32 i = 0; i < getEnableRivetCount(); i++) {
        mRivetList->getActor(i)->makeActorAlive();
        al::showModelIfHide(mRivetList->getActor(i));
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
