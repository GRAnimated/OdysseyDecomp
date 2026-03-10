#include "Npc/Shibaken.h"

#include <container/seadPtrArray.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <prim/seadDelegate.h>

#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/KCollisionServer.h"
#include "Library/Joint/JointControllerBase.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/Joint/JointSpringControllerHolder.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementInfo.h"

#include "MapObj/DigPoint.h"
#include "Npc/NpcStateReaction.h"
#include "Npc/NpcStateReactionParam.h"
#include "Npc/ShibakenFunction.h"
#include "Npc/ShibakenMoveAnimCtrl.h"
#include "Npc/ShibakenStateBark.h"
#include "Npc/ShibakenStateCapCatch.h"
#include "Npc/ShibakenStateJump.h"
#include "Npc/ShibakenStatePointChase.h"
#include "Npc/ShibakenStateSit.h"
#include "Npc/ShibakenStateWait.h"
#include "Npc/ShibakenStateWaitFar.h"
#include "Npc/TalkNpcCap.h"
#include "Player/PlayerPushReceiver.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/Hack.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/SensorMsgFunction.h"
#include "Util/SpecialBuildUtil.h"

extern char sIsNotMoveLimit[];

namespace {
struct CollisionPartsListSetter {
    al::LiveActor* actor;
    sead::PtrArray<al::CollisionParts>* array;

    void addParts(al::CollisionParts* parts) { array->pushBack(parts); }
};

class ShibakenChestJointController : public al::JointControllerBase {
public:
    ShibakenChestJointController() : al::JointControllerBase(1) {}

    void calcJointCallback(s32, sead::Matrix34f*) override {}

    const char* getCtrlTypeName() const override { return ""; }

    al::LiveActor* mActor = nullptr;
    sead::Vector3f mFrontDir = {0.0f, 0.0f, 0.0f};
    f32 mLerpState1 = 0.0f;
    f32 mLerpState2 = 0.0f;
};

struct NrvShibakenType {
    u64 WaitInit, Wait, WaitFar, Reaction, PointChase, CapCatch, Bark, Jump, Sit, Reset,
        PlayerChase, SleepStart, PlayerChaseTurn;
};

NrvShibakenType NrvShibaken;
}  // namespace

#define NERVE(name) reinterpret_cast<const al::Nerve*>(&NrvShibaken.name)

extern NpcStateReactionParam sShibakenReactionParam;

Shibaken::Shibaken(const char* name) : al::LiveActor(name) {}

// NON_MATCHING: regswaps and minor codegen differences throughout
void Shibaken::init(const al::ActorInitInfo& info) {
    mIsHomeShip = al::isObjectName(info, "ShibakenHomeShipInside");

    const char* suffix = mIsHomeShip ? "InHome" : nullptr;
    al::initActorWithArchiveName(this, info, "Shibaken", suffix);
    al::initNerve(this, NERVE(WaitInit), 9);

    mInitTrans = al::getTrans(this);
    mInitQuat = al::getQuat(this);

    if (!rs::isExistTalkNpcParamHolder(this)) {
        makeActorDead();
        return;
    }

    GameDataHolderAccessor accessor(this);
    if (GameDataFunction::isWorldMoon(accessor))
        mWorldMoonFlag = 1;

    if (!mIsHomeShip)
        mCapTarget = rs::createCapTargetInfo(this, nullptr);

    mTalkNpcCap = TalkNpcCap::createForShibaken(this, info);
    if (mTalkNpcCap) {
        al::initSubActorKeeperNoFile(this, info, 1);
        al::registerSubActor(this, mTalkNpcCap);
        al::onSyncClippingSubActor(this, mTalkNpcCap);
        al::onSyncAppearSubActor(this, mTalkNpcCap);
        al::onSyncHideSubActor(this, mTalkNpcCap);
        al::onSyncAlphaMaskSubActor(this, mTalkNpcCap);
    }

    mPushReceiver = new PlayerPushReceiver(this);

    mMoveAnimCtrl = new ShibakenMoveAnimCtrl(this, 0.5f, 3.25f, 0.8f);

    auto* stateWait = new ShibakenStateWait(u8"待機", this, false);
    mStateWait = stateWait;

    mStateWaitFar = new ShibakenStateWaitFar(u8"遠目で待機", this);

    auto* stateReaction = NpcStateReaction::createForHuman(this, &sShibakenReactionParam);
    mStateReaction = stateReaction;

    mStatePointChase = new ShibakenStatePointChase(u8"ここ掘れポイント追いかけ", this,
                                                   mMoveAnimCtrl, stateReaction);

    mStateCapCatch = new ShibakenStateCapCatch(u8"キャップキャッチ", this);

    mStateBark = new ShibakenStateBark(u8"敵に吠える", this, mStateReaction);

    auto* stateJump = new ShibakenStateJump(u8"ジャンプ", this);
    mStateJump = stateJump;

    al::initNerveState(this, stateWait, NERVE(WaitInit), u8"初回待機");
    al::initNerveState(this, mStateWait, NERVE(Wait), u8"待機");
    al::initNerveState(this, mStateWaitFar, NERVE(WaitFar), u8"遠目で待機");
    al::initNerveState(this, mStateReaction, NERVE(Reaction), u8"リアクション");
    al::initNerveState(this, mStatePointChase, NERVE(PointChase), u8"ここ掘れポイント追いかけ");
    al::initNerveState(this, mStateCapCatch, NERVE(CapCatch), u8"キャップキャッチ");
    al::initNerveState(this, mStateBark, NERVE(Bark), u8"敵に吠える");
    al::initNerveState(this, stateJump, NERVE(Jump), u8"ジャンプ");

    al::tryGetArg(&mIsInvalidJumpLowWall, info, "IsInvalidJumpLowWall");

    s32 rangeSensorRadius = 3000;
    if (al::tryGetArg(&rangeSensorRadius, info, "RangeSensorRadius") && rangeSensorRadius > 0)
        mRangeSensorRadius = (f32)rangeSensorRadius;

    al::setSensorRadius(this, "Range", mRangeSensorRadius);

    if (!mIsHomeShip)
        al::setColliderFilterCollisionParts(
            this, reinterpret_cast<al::CollisionPartsFilterBase*>(sIsNotMoveLimit));

    auto* collisionArray = new sead::PtrArray<al::CollisionParts>();
    collisionArray->setBuffer(32, reinterpret_cast<al::CollisionParts**>(collisionArray + 1));
    _1c8 = collisionArray;

    auto* digPointHolder = static_cast<void**>(::operator new(0x28));
    digPointHolder[0] = nullptr;
    digPointHolder[1] = nullptr;
    digPointHolder[2] = nullptr;
    reinterpret_cast<s32*>(digPointHolder)[6] = -1;
    digPointHolder[4] = nullptr;

    if (al::isExistLinkChild(info, "DigPoint", 0)) {
        al::PlacementInfo placementInfo;
        al::getLinksInfo(&placementInfo, info, "DigPoint");
        auto* locater = static_cast<ShibakenDigPointLocater*>(::operator new(0x18));
        initShibakenDigPointLocater(locater, info, placementInfo);
        digPointHolder[0] = locater;
        locater->isValid = false;
    }

    auto* smellArray = new DigPoint*[8];
    digPointHolder[1] = smellArray;
    auto* smellArray2 = reinterpret_cast<DigPoint**>(digPointHolder[1]);
    smellArray[4] = nullptr;
    smellArray[0] = nullptr;
    smellArray[1] = nullptr;
    smellArray[2] = nullptr;
    smellArray[3] = nullptr;
    smellArray2[5] = nullptr;
    smellArray2[6] = nullptr;
    smellArray2[7] = nullptr;
    mDigPointHolder = digPointHolder;

    if (digPointHolder[0]) {
        mStatePointChase->startFirstWait(*static_cast<DigPoint**>(digPointHolder[0]));
        al::setNerve(this, NERVE(PointChase));
    }

    s32 springJointNum = al::JointSpringControllerHolder::calcInitFileSpringControlJointNum(
        this, "InitJointSpringCtrl");
    al::initJointControllerKeeper(this, springJointNum + 2);

    auto* chestJoint = new ShibakenChestJointController();
    chestJoint->mActor = this;
    s32 chestJointIndex = al::getJointIndex(this, "chest");
    chestJoint->appendJointId(chestJointIndex);
    al::registerJointController(chestJoint->mActor, chestJoint);
    mChestJoint = chestJoint;

    auto* springHolder = new al::JointSpringControllerHolder();
    springHolder->init(this, "InitJointSpringCtrl");

    TalkNpcParam* talkParam = rs::initTalkNpcParam(this, nullptr);
    mJointLookAt = rs::tryCreateAndAppendNpcJointLookAtController(this, talkParam);

    bool isKeepWait = false;
    if (mIsHomeShip || (al::tryGetArg(&isKeepWait, info, "IsKeepWait") && isKeepWait)) {
        auto* stateSit = new ShibakenStateSit(u8"座り", this, mStateReaction, true);
        mStateSit = stateSit;
        al::initNerveState(this, stateSit, NERVE(Sit), u8"座り");
        al::setNerve(this, NERVE(Sit));
    }

    if (mIsHomeShip) {
        GameDataHolderAccessor acc(this);
        if (!GameDataFunction::isGameClear(acc)) {
            makeActorDead();
            return;
        }
        GameDataHolderAccessor acc2(this);
        s32 special1 = GameDataFunction::getWorldIndexSpecial1();
        if (!GameDataFunction::isUnlockedWorld(acc2, special1)) {
            makeActorDead();
            return;
        }
        GameDataHolderAccessor acc3(this);
        if (GameDataFunction::isWorldSand(acc3)) {
            makeActorDead();
            return;
        }
        GameDataHolderAccessor acc4(this);
        if (GameDataFunction::isWorldCity(acc4)) {
            makeActorDead();
            return;
        }
        GameDataHolderAccessor acc5(this);
        if (GameDataFunction::isWorldSea(acc5)) {
            makeActorDead();
            return;
        }
        GameDataHolderAccessor acc6(this);
        if (GameDataFunction::isWorldMoon(acc6)) {
            makeActorDead();
            return;
        }
        GameDataHolderAccessor acc7(this);
        if (GameDataFunction::isWorldPeach(acc7)) {
            makeActorDead();
            return;
        }
    }

    if (rs::isModeE3Rom())
        makeActorDead();
    else
        makeActorAlive();
}

void Shibaken::initAfterPlacement() {
    auto** holder = static_cast<ShibakenDigPointLocater**>(mDigPointHolder);
    if (holder) {
        const sead::Vector3f& trans = al::getTrans(this);
        ShibakenDigPointLocater* locater = holder[0];
        if (locater)
            updateShibakenDigPointLocaterHintTrans(locater, trans);
    }
}

// NON_MATCHING: PIC mode symbol access and register allocation differences
void Shibaken::movement() {
    if (mIsHomeShip) {
        al::LiveActor::movement();
        return;
    }

    auto* collisionArray = static_cast<sead::PtrArray<al::CollisionParts>*>(_1c8);

    CollisionPartsListSetter setter;
    setter.actor = this;
    setter.array = collisionArray;

    auto* savedArray = alCollisionUtil::getCollisionPartsPtrArray(this);
    collisionArray->clear();

    sead::Delegate1<CollisionPartsListSetter, al::CollisionParts*> delegate;
    delegate.bind(&setter, &CollisionPartsListSetter::addParts);

    const sead::Vector3f& trans = al::getTrans(this);
    f32 radius = al::getColliderRadius(this);
    alCollisionUtil::searchCollisionParts(
        this, trans, radius * 1.5f, delegate,
        reinterpret_cast<al::CollisionPartsFilterBase*>(sIsNotMoveLimit));

    alCollisionUtil::validateCollisionPartsPtrArray(this, collisionArray);
    al::LiveActor::movement();

    if (setter.actor)
        alCollisionUtil::validateCollisionPartsPtrArray(setter.actor, savedArray);
    else
        alCollisionUtil::invalidateCollisionPartsPtrArray(setter.actor);
}

// NON_MATCHING: Vector3f copy generates 3 word stores instead of qword+word
void Shibaken::control() {
    mMoveAnimCtrl->update();

    if (!mIsHomeShip) {
        al::updateMaterialCodePuddle(this, al::isInWater(this));
        if (al::isCollidedGround(this))
            al::setMaterialCode(this, al::getCollidedFloorMaterialCodeName(this));
        else
            al::resetMaterialCode(this);
    }

    auto* chestJoint = static_cast<ShibakenChestJointController*>(mChestJoint);
    if (chestJoint) {
        if (al::isNormalize(chestJoint->mFrontDir, 0.001f)) {
            sead::Vector3f frontDir = {0.0f, 0.0f, 0.0f};
            al::calcFrontDir(&frontDir, chestJoint->mActor);
            const sead::Vector3f& gravity = al::getGravity(chestJoint->mActor);
            sead::Vector3f up = {-gravity.x, -gravity.y, -gravity.z};
            f32 angle = al::calcAngleOnPlaneDegree(chestJoint->mFrontDir, frontDir, up);
            f32 normalized = al::normalizeAbs(angle, 0.0f, 5.0f);
            f32 lerped1 = al::lerpValue(chestJoint->mLerpState2, normalized, 0.2f);
            f32 prev = chestJoint->mLerpState1;
            chestJoint->mLerpState2 = lerped1;
            chestJoint->mLerpState1 = al::lerpValue(prev, lerped1, 0.1f);
            chestJoint->mFrontDir = frontDir;
        } else {
            al::calcFrontDir(&chestJoint->mFrontDir, chestJoint->mActor);
        }
    }

    if (mJointLookAt)
        rs::updateNpcJointLookAtController(mJointLookAt);
}

void Shibaken::startClipped() {
    al::LiveActor::startClipped();
    if (al::isNerve(this, NERVE(CapCatch))) {
        if (al::isActionPlaying(this, "Move"))
            al::setNerve(this, NERVE(PlayerChase));
        else
            al::setNerve(this, NERVE(Wait));
    }
}

void Shibaken::updateCollider() {
    sead::Vector3f pushedVelocity = {0.0f, 0.0f, 0.0f};
    sead::Vector3f savedVelocity = al::getVelocity(this);
    mPushReceiver->calcPushedVelocity(&pushedVelocity, savedVelocity);
    al::setVelocity(this, pushedVelocity);
    al::LiveActor::updateCollider();
    al::setVelocity(this, savedVelocity);
    mPushReceiver->clear();

    auto** holder = static_cast<ShibakenDigPointLocater**>(mDigPointHolder);
    const sead::Vector3f& trans = al::getTrans(this);
    ShibakenDigPointLocater* locater = holder[0];
    if (locater)
        updateShibakenDigPointLocaterHintTrans(locater, trans);
}

void Shibaken::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    mStateBark->attackSensor(self, other);
    if (al::isSensorEnemyBody(self)) {
        rs::attackSensorNpcCommon(self, other);
        rs::sendMsgPushToMotorcycle(other, self);
        rs::sendMsgShibakenKick(other, self);
    }
    if (al::isSensorEnemyAttack(self))
        rs::sendMsgShibakenApproach(other, self);
}

// NON_MATCHING: branch polarity difference (tbnz vs tbz) in stateReaction check
bool Shibaken::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) {
    if (mCapTarget) {
        if (rs::tryReceiveMsgInitCapTargetAndSetCapTargetInfo(
                msg, static_cast<CapTargetInfo*>(mCapTarget)))
            return true;
    } else {
        if (rs::isMsgInitCapTarget(msg))
            return false;
    }

    if (rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;

    if (al::isNerve(this, NERVE(Reset)))
        return false;

    if (rs::isMsgKillByHomeDemo(msg)) {
        al::hideModelIfShow(this);
        al::setVelocityZero(this);
        al::invalidateHitSensors(this);
        al::setNerve(this, NERVE(Reset));
        return true;
    }

    if (!mIsHomeShip) {
        if (rs::isMsgCapCancelLockOn(msg)) {
            if (al::isNerve(this, NERVE(CapCatch))) {
                al::setNerve(this, NERVE(Wait));
                return true;
            }
            return true;
        }

        if (!al::isSensorNpc(self)) {
            if (al::isNerve(this, NERVE(CapCatch))) {
                if (mStateCapCatch->receiveMsg(msg, other, self))
                    return true;
            } else if (mStateCapCatch->tryStartByReceiveMsg(msg, other, self)) {
                al::setNerve(this, NERVE(CapCatch));
                return true;
            }
        }
    }

    if (al::isNerve(this, NERVE(Sit))) {
        if (mStateSit->receiveMsg(msg, other, self))
            return true;
    } else if (al::isNerve(this, NERVE(PointChase))) {
        if (mStatePointChase->receiveMsg(msg, other, self))
            return true;
    } else if (al::isNerve(this, NERVE(Bark))) {
        if (mStateBark->receiveMsg(msg, other, self))
            return true;
    } else if (al::isNerve(this, NERVE(Jump))) {
        if (mStateJump->receiveMsg(msg, other, self))
            return true;
    } else {
        if (al::isNerve(this, NERVE(CapCatch)) && rs::isMsgCapReflect(msg))
            return false;

        auto* stateReaction = mStateReaction;
        if (al::isSensorEnemyBody(self) &&
            !(al::isNerve(this, NERVE(Reaction)) && !al::isNewNerve(this)) &&
            !rs::isMsgFireDamageAll(msg) && !rs::isMsgSphinxRideAttackReflect(msg) &&
            stateReaction->receiveMsg(msg, other, self)) {
            al::setNerve(this, NERVE(Reaction));
            return true;
        }
    }

    if (mIsHomeShip)
        return false;

    if (al::isSensorNpc(self) && rs::isMsgDigPointSmell(msg)) {
        auto** holder = static_cast<void***>(mDigPointHolder);
        DigPoint* smellDigPoint = rs::getSmellDigPoint(msg);

        if (holder[0] && *reinterpret_cast<DigPoint**>(holder[0]) == smellDigPoint)
            return false;

        if (smellDigPoint->_110)
            return false;

        if (reinterpret_cast<DigPoint*>(holder[4]) == smellDigPoint)
            return false;

        auto** smellArray = reinterpret_cast<DigPoint**>(holder[1]);
        for (s32 i = 0; i < 8; i++) {
            if (smellArray[i] == smellDigPoint)
                return false;
            if (!smellArray[i]) {
                smellArray[i] = smellDigPoint;
                return true;
            }
        }

        return false;
    }

    if (al::isSensorEnemyBody(self)) {
        if (rs::isMsgIgnorePushMotorcycle(msg))
            return true;
        mPushReceiver->receivePushMsgHacker(msg, other, self, 5.0f, false);
    }
    return false;
}
