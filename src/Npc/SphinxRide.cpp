#include "Npc/SphinxRide.h"

#include <math/seadMatrix.h>
#include <math/seadMatrixCalcCommon.hpp>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Area/AreaObj.h"
#include "Library/Area/AreaObjGroup.h"
#include "Library/Area/AreaObjUtil.h"
#include "Library/Base/StringUtil.h"
#include "Library/Bgm/BgmLineFunction.h"
#include "Library/Camera/ActorCameraTarget.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Collision/CollisionParts.h"
#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/PartsConnector.h"
#include "Library/Controller/PadRumbleFunction.h"
#include "Library/Demo/DemoFunction.h"
#include "Library/Effect/EffectSystemInfo.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/Joint/JointLookAtController.h"
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
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Se/SeFunction.h"
#include "Library/Shadow/ActorShadowUtil.h"
#include "Library/Stage/StageSwitchUtil.h"

#include "Npc/NpcStateReaction.h"
#include "Npc/NpcStateReactionParam.h"
#include "Npc/SphinxRideAim.h"
#include "Npc/SphinxRideEvent.h"
#include "Npc/SphinxRideParam.h"
#include "Npc/SphinxRideStateRevival.h"
#include "Npc/SphinxRideStateStop.h"
#include "System/GameDataFunction.h"
#include "System/GameDataUtil.h"
#include "Util/CameraUtil.h"
#include "Util/DemoUtil.h"
#include "Util/InputInterruptTutorialUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerDemoUtil.h"
#include "Util/PlayerPuppetFunction.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(SphinxRide, Wait);
NERVE_IMPL(SphinxRide, Stop);
NERVE_IMPL(SphinxRide, Reaction);
NERVE_IMPL(SphinxRide, Standby);
NERVE_IMPL(SphinxRide, Revival);
NERVE_IMPL(SphinxRide, DemoStandbyStart);
NERVE_IMPL(SphinxRide, DemoStandbyTurnZero);
NERVE_IMPL(SphinxRide, GetOff);
NERVE_IMPL(SphinxRide, Jump);
NERVE_IMPL_(SphinxRide, RideStartLeft, GetOnStart);
NERVE_IMPL_(SphinxRide, RideStartRight, GetOnStart);
NERVE_IMPL(SphinxRide, GetOnStartOn);
NERVE_IMPL(SphinxRide, Run);
NERVE_IMPL(SphinxRide, GetOn);
NERVE_IMPL(SphinxRide, Land);
NERVE_IMPL(SphinxRide, Clash);
NERVE_IMPL(SphinxRide, DemoTurnEnd);
NERVE_IMPL(SphinxRide, EventWait);
NERVE_IMPL(SphinxRide, Fall);

NERVES_MAKE_STRUCT(SphinxRide, Wait, Stop, Reaction, Standby, Revival, DemoStandbyStart,
                   DemoStandbyTurnZero, GetOff, Jump, RideStartLeft, RideStartRight,
                   GetOnStartOn, Run, Clash, DemoTurnEnd, EventWait, Fall);
NERVES_MAKE_NOSTRUCT(SphinxRide, GetOn, Land);
}  // namespace

static NpcStateReactionParam sReactionWaitParam(u8"ReactionWait", u8"ReactionCapWait");
static NpcStateReactionParam sReactionCapStandbyParam(u8"ReactionCapStandby",
                                                      u8"ReactionCapStandby");
static constexpr SphinxRideParam sSphinxRideParam = {0.95f, 0.9f};

struct RumbleParam {
    f32 volumeLeft;
    f32 volumeRight;
    const char* name;
};

static RumbleParam sRunRumbleParams[] = {
    {0.12f, 0.05f, "PresetZaZa_nvibEdit"},
    {0.12f, 0.05f, "PresetKott"},
    {0.1f, 0.1f, "PresetDon"},
};
static RumbleParam sRunSlowRumbleParams[] = {
    {0.06f, 0.06f, "PresetZaZa_nvibEdit"},
    {0.05f, 0.05f, "PresetKott"},
    {0.02f, 0.02f, "PresetDon"},
};
static RumbleParam sStopRumbleParams[] = {
    {0.13f, 0.13f, "PresetZaZa_nvibEdit"},
    {0.12f, 0.12f, "PresetKott"},
    {0.05f, 0.05f, "PresetDon"},
};

static sead::Vector3f sArrowCheckOffset = {0.0f, 40.0f, 0.0f};
static sead::Vector3f sArrowCheckDir = {0.0f, -345.0f, 0.0f};

static s32 getMaterialRumbleIndex(al::LiveActor* actor) {
    if (al::isCollidedFloorCode(actor, "Poison") ||
        al::isCollidedGroundFloorCode(actor, "Poison"))
        return 2;
    const char* material = al::getCollidedFloorMaterialCodeName(actor);
    if (material) {
        bool isStoneRough = al::isEqualString(material, "StoneRough");
        bool isSandDesert = al::isEqualString(material, "SandDesert");
        s32 idx = isStoneRough & 1;
        if (isSandDesert)
            idx = 0;
        return idx;
    }
    return 0;
}

static bool calcSlopeProjectedFront(sead::Vector3f* out, al::LiveActor* actor) {
    if (!al::isOnGround(actor, 0))
        return false;

    const sead::Vector3f& normal = al::getOnGroundNormal(actor, 0);
    sead::Vector3f frontDir;
    al::calcFrontDir(&frontDir, actor);

    sead::Vector3f cross1;
    cross1.x = normal.y * frontDir.z - normal.z * frontDir.y;
    cross1.y = normal.z * frontDir.x - normal.x * frontDir.z;
    cross1.z = normal.x * frontDir.y - normal.y * frontDir.x;
    al::tryNormalizeOrDirZ(&cross1);

    out->x = cross1.y * normal.z - cross1.z * normal.y;
    out->y = cross1.z * normal.x - cross1.x * normal.z;
    out->z = cross1.x * normal.y - cross1.y * normal.x;
    al::tryNormalizeOrDirZ(out);
    return true;
}

static void addGravityScaleAndLimitVelocity(SphinxRide* actor, f32 gravity, f32 hScale,
                                            f32 vScale);

SphinxRide::SphinxRide(const char* name) : al::LiveActor(name) {}

// NON_MATCHING: complex init with direct collider/joint memory writes
void SphinxRide::init(const al::ActorInitInfo& initInfo) {
    al::initActor(this, initInfo);
    al::initNerve(this, &NrvSphinxRide.Wait, 3);

    mStateStop = new SphinxRideStateStop(this, sSphinxRideParam);
    al::initNerveState(this, mStateStop, &NrvSphinxRide.Stop, u8"ストップ状態");

    mStateReaction = NpcStateReaction::create(this, nullptr);
    al::initNerveState(this, mStateReaction, &NrvSphinxRide.Reaction, u8"リアクション");

    mTalkNpcParam = rs::initTalkNpcParam(this, nullptr);

    s32 linkCount = al::calcLinkChildNum(initInfo, "SphinxRideEvent");
    for (s32 i = 0; i < linkCount; i++) {
        auto* event = new SphinxRideEvent(this);
        al::initLinksActor(event, initInfo, "SphinxRideEvent", i);
        al::registerSubActorSyncClipping(this, event);
    }

    mEventFlowExecutor = rs::initEventFlow(this, initInfo, nullptr, nullptr);
    al::initEventReceiver(mEventFlowExecutor, (al::IEventFlowEventReceiver*)this);
    rs::initEventQueryJudge(mEventFlowExecutor, (al::IEventFlowQueryJudge*)this);
    rs::initEventCharacterName(mEventFlowExecutor, initInfo, "SphinxRide");
    rs::initEventParam(mEventFlowExecutor, mTalkNpcParam, nullptr);
    rs::initEventCameraObject(mEventFlowExecutor, initInfo, "TalkCamera");

    bool isPayCoin = GameDataFunction::isPayCoinToSphinx(this);
    bool isStartStandby = false;
    al::tryGetArg(&isStartStandby, initInfo, "IsStartStandby");

    const char* flowName = "PayCoin";
    if (isStartStandby || isPayCoin) {
        mIsCanRidePlacement = true;
        al::validateShadowMask(this, "StandByFront");
        al::validateShadowMask(this, "StandByBack");
        al::invalidateShadowMask(this, "Wait");
        al::setNerve(this, &NrvSphinxRide.Standby);
        flowName = "Standby";
    }

    rs::startEventFlow(mEventFlowExecutor, flowName);

    al::invalidateHitSensor(this, "PushTalk");
    al::initJointControllerKeeper(this, 9);
    al::initJointGlobalQuatController(this, &mJointQuat, "AllRoot");
    al::calcUpDir(&mUpDir, this);
    al::calcFrontDir(&mFrontDir, this);
    al::initJointLocalZRotator(this, &mAllRootRotateZ, "AllRoot");
    al::initJointLocalZRotator(this, &mPlayerRotateZ, "Player");
    updateJointPose();

    mSphinxRideAim = new SphinxRideAim(this);

    s32 cameraAngle = -1;
    al::tryGetArg(&cameraAngle, initInfo, "CameraAngle");
    mCameraAngle = (f32)cameraAngle;
    al::tryGetArg(&cameraAngle, initInfo, "StartDegree");
    mStartDegree = (f32)cameraAngle;

    al::setEffectFollowPosPtr(this, "HitSmallWallClash", &mEffectFollowPos);
    al::hideSilhouetteModel(this);
    mBindKeepDemoInfo = rs::initBindKeepDemoInfo();
    al::createAndSetColliderSpecialPurpose(this, "MoveLimit");
    al::setColliderRadius(this, 100.0f);

    if (al::isExistLinkChild(initInfo, "SphinxRevivalPoint", 0) &&
        rs::isKidsMode((const al::LiveActor*)this)) {
        auto* revival = new SphinxRideStateRevival(this, initInfo, false);
        mStateRevival = revival;
        al::initNerveState(this, revival, &NrvSphinxRide.Revival, u8"復活状態");
        al::invalidateClipping(this);
    } else {
        GameDataHolderAccessor accessor(this);
        if (GameDataFunction::isTimeBalloonSequence(accessor) ||
            GameDataFunction::isRaceStartFlag(accessor)) {
            auto* revival = new SphinxRideStateRevival(this, initInfo, true);
            mStateRevival = revival;
            al::initNerveState(this, revival, &NrvSphinxRide.Revival, u8"復活状態");
            al::invalidateClipping(this);
        }
    }

    al::registActorToDemoInfo(this, initInfo);
    mCameraTarget = al::createActorCameraTarget(this, 100.0f);
    makeActorAlive();
}

// NON_MATCHING: complex joint pose computation with ground normal projection
void SphinxRide::updateJointPose() {
    if (al::isNerve(this, &NrvSphinxRide.DemoStandbyStart) ||
        al::isNerve(this, &NrvSphinxRide.DemoStandbyTurnZero) ||
        al::isNerve(this, &NrvSphinxRide.DemoTurnEnd) ||
        al::isNerve(this, &NrvSphinxRide.EventWait)) {
        al::calcFrontDir(&mFrontDir, this);
        al::calcUpDir(&mUpDir, this);
    } else {
        sead::Vector3f frontDir;
        al::calcFrontDir(&frontDir, this);

        if (al::isOnGround(this, 0)) {
            const sead::Vector3f& groundNormal = al::getCollidedGroundNormal(this);
            al::lerpVec(&mUpDir, mUpDir, groundNormal, 0.1f);
        }

        al::lerpVec(&mFrontDir, mFrontDir, frontDir, 0.1f);
    }

    al::makeQuatUpFront(&mJointQuat, mUpDir, mFrontDir);
    mScaledUpDir = mUpDir * 100.0f;
}

void SphinxRide::initAfterPlacement() {
    if (al::isNerve(this, &NrvSphinxRide.Standby)) {
        al::validateShadowMask(this, "StandByFront");
        al::validateShadowMask(this, "StandByBack");
        al::invalidateShadowMask(this, "Wait");
    } else {
        al::validateShadowMask(this, "Wait");
        al::invalidateShadowMask(this, "StandByFront");
        al::invalidateShadowMask(this, "StandByBack");
    }

    rs::tryRegisterSphinxQuizRouteKillSensorAfterPlacement(al::getHitSensor(this, "PlayerBody"));
}

void SphinxRide::movement() {
    al::LiveActor::movement();
}

// NON_MATCHING: compiler reordering, evaluation optimization, MI cast codegen
void SphinxRide::control() {
    if (mPlayerPuppet) {
        const sead::Vector2f& moveStick = rs::getPuppetMoveStick(mPlayerPuppet);
        sead::Vector3f viewInput;
        const sead::Matrix34f* viewMtx = al::getViewMtxPtr(this, 0);
        al::calcVecViewInput(&viewInput, moveStick, sead::Vector3f::ey, viewMtx);
        rs::copyPuppetDitherAlpha(mPlayerPuppet, this);

        const sead::Vector3f& trans = al::getTrans(this);
        al::AreaObj* forceOffArea =
            al::tryFindAreaObj(this, "SphinxRideGetOffForceArea", trans);
        if (forceOffArea) {
            requestGetOffForce();
            forceOffArea->invalidate();
        }

        al::AreaObj* inCheckArea =
            al::tryFindAreaObj(this, "SphinxRideInCheckArea", al::getTrans(this));
        if (inCheckArea) {
            al::tryOnStageSwitch(inCheckArea, "SwitchAreaOn");
            inCheckArea->invalidate();
        }

        al::AreaObjGroup* keepOnGroup =
            al::tryFindAreaObjGroup(this, "SphinxRideInCheckAreaKeepOn");
        if (keepOnGroup && keepOnGroup->getSize() >= 1) {
            for (s32 i = 0; i < keepOnGroup->getSize(); i++) {
                al::AreaObj* area = keepOnGroup->getAreaObj(i);
                if (al::isInAreaPos(area, al::getTrans(this)))
                    al::tryOnStageSwitch(area, "SwitchAreaOn");
                else
                    al::tryOffStageSwitch(area, "SwitchAreaOn");
            }
        }

        al::AreaObjGroup* rideOffGroup =
            al::tryFindAreaObjGroup(this, "SphinxRideInRideOffCheckAreaKeepOn");
        if (rideOffGroup && rideOffGroup->getSize() >= 1) {
            for (s32 i = 0; i < rideOffGroup->getSize(); i++) {
                al::AreaObj* area = rideOffGroup->getAreaObj(i);
                al::tryOffStageSwitch(area, "SwitchAreaOn");
            }
        }
    } else {
        al::AreaObjGroup* rideOffGroup =
            al::tryFindAreaObjGroup(this, "SphinxRideInRideOffCheckAreaKeepOn");
        if (rideOffGroup && rideOffGroup->getSize() >= 1) {
            for (s32 i = 0; i < rideOffGroup->getSize(); i++) {
                al::AreaObj* area = rideOffGroup->getAreaObj(i);
                if (al::isInAreaPos(area, al::getTrans(this)))
                    al::tryOnStageSwitch(area, "SwitchAreaOn");
            }
        }
    }

    if (mStateRevival) {
        if (!mStateRevival->isNoMove()) {
            bool safetyRecovery = rs::isPlayerSafetyPointRecovery(this);
            if ((!_208 && safetyRecovery) || al::isInDeathArea(this)) {
                _208 = rs::isPlayerSafetyPointRecovery(this);
                if (!al::isNerve(this, &NrvSphinxRide.Revival)) {
                    if (mPlayerPuppet) {
                        al::invalidateHitSensor(this, "BindFront");
                        al::invalidateHitSensor(this, "BindBack");
                        al::disableBgmLineChange(this);
                        al::stopBgm(this, "SphinxRide", 120);
                        mBgmLineChangeCount = 120;
                        rs::tryCloseBindTutorial(this);
                    }
                    al::setNerve(this, &NrvSphinxRide.Revival);
                }
            } else if (al::isNerve(this, &NrvSphinxRide.Standby) &&
                       al::isInAreaObj(this, "ChangeStageArea",
                                       al::getTrans(this))) {
                al::setNerve(this, &NrvSphinxRide.Revival);
            } else {
                _208 = rs::isPlayerSafetyPointRecovery(this);
            }
        } else {
            _208 = rs::isPlayerSafetyPointRecovery(this);
        }
    }

    mFrontDir = al::getTrans(this);

    if (_230 > 0)
        _230--;

    s32 bgmCount = mBgmLineChangeCount - 1;
    if (bgmCount >= 0) {
        mBgmLineChangeCount = bgmCount;
        if (bgmCount == 0)
            al::enableBgmLineChange(this);
    }

    if (mPlayerPuppet)
        mSphinxRideAim->updateReset();
    else
        mSphinxRideAim->updateLook();

    updateJointPose();

    bool isRunSlow = al::isActionPlaying(this, "RunSlow");
    f32 targetPlayerRotateZ;
    if (isRunSlow) {
        mPlayerRotateZTarget = 0.0f;
        mAllRootRotateZTarget = 0.0f;
        targetPlayerRotateZ = 0.0f;
    } else {
        targetPlayerRotateZ = mPlayerRotateZTarget;
    }

    if (!al::isNear(mPlayerRotateZ, targetPlayerRotateZ, 0.001f))
        mPlayerRotateZ = al::lerpValue(mPlayerRotateZ, targetPlayerRotateZ, 0.2f);

    if (!al::isNear(mAllRootRotateZ, mAllRootRotateZTarget, 0.001f))
        mAllRootRotateZ = al::lerpValue(mAllRootRotateZ, mAllRootRotateZTarget, 0.1f);

    if (al::isOnGround(this, 0)) {
        al::HitSensor* bodySensor = al::getHitSensor(this, "PlayerBody");
        al::sendMsgPlayerFloorTouchToColliderGround(this, bodySensor);
    }
}

// NON_MATCHING: missing call to sub_71003D54F4 (unbind helper, not yet decompiled)
void SphinxRide::requestGetOffForce() {
    if (!mPlayerPuppet)
        return;

    // sub_71003D54F4: complex unbind helper (664 bytes)
    // showPuppetShadow, hideSilhouetteModelIfShow, calculate puppet position,
    // checkStrikeSphereMove, validatePuppetCollider, startPuppetAction("Jump"),
    // endBindJumpAndPuppetNull, resetCameraTarget

    al::invalidateHitSensor(this, "BindFront");
    al::invalidateHitSensor(this, "BindBack");
    al::disableBgmLineChange(this);
    al::stopBgm(this, "SphinxRide", 120);
    mBgmLineChangeCount = 120;
    rs::tryCloseBindTutorial(this);

    if (al::isOnGround(this, 0))
        al::setNerve(this, &NrvSphinxRide.GetOff);
    else
        al::setNerve(this, &NrvSphinxRide.Fall);
}

void SphinxRide::updateCollider() {
    al::LiveActor::updateCollider();
    if (mPlayerPuppet)
        rs::requestUpdateSafetyPoint(mPlayerPuppet, this, nullptr);
}

// NON_MATCHING: data section offsets, const pool loading pattern
void SphinxRide::calcAnim() {
    al::LiveActor::calcAnim();

    if (!mPlayerPuppet || rs::isActiveBindKeepDemo(mBindKeepDemoInfo))
        return;

    rs::setPuppetVelocity(mPlayerPuppet, sead::Vector3f::zero);

    const sead::Matrix34f* jointMtx = al::getJointMtxPtr(this, "Player");

    sead::Quatf quat;
    sead::Matrix34CalcCommon<f32>::toQuat(quat, *jointMtx);
    quat.normalize();

    sead::Vector3f pos = {(*jointMtx)(0, 3), (*jointMtx)(1, 3), (*jointMtx)(2, 3)};

    if (al::isNerve(this, &NrvSphinxRide.RideStartLeft) ||
        al::isNerve(this, &NrvSphinxRide.RideStartRight) ||
        al::isNerve(this, &NrvSphinxRide.GetOnStartOn)) {
        f32 frameMax = rs::getPuppetAnimFrameMax(mPlayerPuppet);
        f32 rate = al::calcNerveRate(this, (s32)frameMax);
        al::slerpQuat(&quat, mSavedPuppetQuat, quat, rate);
        al::lerpVec(&pos, mSavedPuppetTrans, pos, rate);
    }
    rs::setPuppetTrans(mPlayerPuppet, pos);
    rs::setPuppetQuat(mPlayerPuppet, quat);
}

void SphinxRide::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (!mPlayerPuppet) {
        if (al::isSensorMapObj(self)) {
            if (_230 || !rs::sendMsgPushToPlayer(other, self)) {
                if (!al::isNerve(this, &NrvSphinxRide.Wait) &&
                    !al::isNerve(this, &NrvSphinxRide.DemoStandbyStart) &&
                    !al::isNerve(this, &NrvSphinxRide.DemoStandbyTurnZero) &&
                    !al::isNerve(this, &NrvSphinxRide.DemoTurnEnd))
                    al::sendMsgPush(other, self);
            }
        }
        return;
    }

    if (al::isSensorPlayerAttack(self)) {
        if (!al::isNerve(this, &NrvSphinxRide.Run) && !al::isNerve(this, &NrvSphinxRide.Stop))
            return;

        if (al::isSensorNpc(other) ||
            al::isSensorHostName(other, "メガネー")) {
            if (al::isSensorName(self, "AttackReflect")) {
                const sead::Vector3f& trans = al::getTrans(this);
                sead::Vector3f diff = {mPrevTrans.x - trans.x, mPrevTrans.y - trans.y,
                                       mPrevTrans.z - trans.z};
                if (diff.length() > 30.0f &&
                    rs::sendMsgSphinxRideAttackReflect(other, self)) {
                    al::startAction(this, "ClashObj");
                    al::setNerve(this, &NrvSphinxRide.Clash);
                    return;
                }
            }
            if (al::isSensorName(self, "PlayerAttack")) {
                if (rs::sendMsgSphinxRideAttack(other, self)) {
                    mLastAttackSensor = other;
                    return;
                }
            }
        } else {
            if (al::isSensorName(self, "AttackReflect")) {
                if (rs::sendMsgSphinxRideAttackReflect(other, self)) {
                    al::startAction(this, "ClashObj");
                    al::setNerve(this, &NrvSphinxRide.Clash);
                    return;
                }
            }
            if (al::isSensorName(self, "PlayerAttack")) {
                if (rs::sendMsgSphinxRideAttack(other, self))
                    return;
            }
        }
    } else if (al::isSensorPlayer(self)) {
        if (rs::sendMsgPlayerItemGetAll(other, self))
            return;
    }

    if (al::isSensorMapObj(self) && al::isSensorHostName(other, "柴犬"))
        al::sendMsgPush(other, self);
}

bool SphinxRide::receiveMsg(const al::SensorMsg* msg, al::HitSensor* self, al::HitSensor* other) {
    if (rs::isMsgPlayerDisregardTargetMarker(msg) || al::isMsgExplosion(msg))
        return true;

    if (rs::isMsgSphinxQuizRouteKill(msg)) {
        if (!al::isAlive(this))
            return false;
        rs::addDemoActor(this, false);
        al::startHitReaction(this, "消滅");
        al::hideModelIfShow(this);
        makeActorDead();
        return true;
    }

    if (mPlayerPuppet) {
        if (al::isMsgBindEnd(msg)) {
            al::invalidateHitSensor(this, "BindFront");
            al::invalidateHitSensor(this, "BindBack");
            al::disableBgmLineChange(this);
            al::stopBgm(this, "SphinxRide", 120);
            mBgmLineChangeCount = 120;
            rs::tryCloseBindTutorial(this);

            if (al::isOnGround(this, 0))
                al::setNerve(this, &NrvSphinxRide.GetOff);
            else
                al::setNerve(this, &NrvSphinxRide.Fall);
            return true;
        }

        if (rs::tryReceiveBindCancelMsgAndPuppetNull(&mPlayerPuppet, msg)) {
            al::resetCameraTarget(this, mCameraTarget);
            al::invalidateHitSensor(this, "BindFront");
            al::invalidateHitSensor(this, "BindBack");
            al::disableBgmLineChange(this);
            al::stopBgm(this, "SphinxRide", 120);
            mBgmLineChangeCount = 120;
            rs::tryCloseBindTutorial(this);
            al::setNerve(this, &NrvSphinxRide.GetOff);
            return true;
        }

        if (rs::isMsgCapTrampolineAttack(msg)) {
            al::calcPosBetweenSensors(&mFrontDir, other, self, 0.0f);
            al::startHitReaction(this, "破壊");
            return true;
        }

        if (rs::isMsgRequestSphinxJump(msg)) {
            f32 selfY = al::getSensorPos(self).y;
            if (selfY <= al::getSensorPos(other).y && !al::isNerve(this, &NrvSphinxRide.Jump) &&
                al::getVelocity(this).y <= 0.0f &&
                rs::tryGetRequestSphinxJumpInfo(&_1bc, msg)) {
                al::setNerve(this, &NrvSphinxRide.Jump);
                return true;
            }
            return false;
        }

        if (al::isMsgGoalKill(msg) && al::isAlive(this)) {
            al::startHitReaction(this, "消滅");
            al::hideModelIfShow(this);
            makeActorDead();
            return true;
        }
    }

    if (al::isSensorBindableAll(other)) {
        bool isStandby = al::isNerve(this, &NrvSphinxRide.Standby);

        if (rs::isMsgRequestPlayerSpinJump(msg)) {
            al::isMsgBindStart(msg);
        } else {
            bool isPlayer2D = rs::isPlayer2D(this);
            if (isStandby && al::isMsgBindStart(msg) && !isPlayer2D) {
                if (rs::isPlayerOnGround(this)) {
                    if (rs::isPlayerInputTriggerRide(this))
                        return true;
                } else {
                    if (rs::isRideOn(self, other))
                        return true;
                }
                return false;
            }
        }

        if (al::isMsgBindInit(msg) && !rs::isPlayer2D(this)) {
            mPlayerPuppet = rs::startPuppet(other, self);
            bool isPuppetOnGround = rs::isCollidedPuppetGround(mPlayerPuppet);
            rs::validatePuppetLookAt(mPlayerPuppet);
            rs::invalidatePuppetCollider(mPlayerPuppet);
            al::setCameraTarget(this, mCameraTarget);
            al::invalidateClipping(this);
            rs::calcPuppetQuat(&mSavedPuppetQuat, mPlayerPuppet);
            const sead::Vector3f& puppetTrans = rs::getPuppetTrans(mPlayerPuppet);
            mSavedPuppetTrans = puppetTrans;

            if (isPuppetOnGround) {
                sead::Vector3f dirH = sead::Vector3f::zero;
                if (al::calcDirBetweenSensorsH(&dirH, other, self)) {
                    sead::Vector3f sideDir = sead::Vector3f::zero;
                    al::calcSideDir(&sideDir, this);
                    if (sideDir.dot(dirH) > 0.0f) {
                        if (mPlayerPuppet)
                            rs::startPuppetAction(mPlayerPuppet, "SphinxRideRideStartL");
                        al::setNerve(this, &NrvSphinxRide.RideStartLeft);
                    } else {
                        if (mPlayerPuppet)
                            rs::startPuppetAction(mPlayerPuppet, "SphinxRideRideStartR");
                        al::setNerve(this, &NrvSphinxRide.RideStartRight);
                    }
                } else {
                    if (mPlayerPuppet)
                        rs::startPuppetAction(mPlayerPuppet, "SphinxRideRideStartR");
                    al::setNerve(this, &NrvSphinxRide.RideStartRight);
                }
            } else {
                al::setNerve(this, &NrvSphinxRide.GetOnStartOn);
            }
            return true;
        }

        if (rs::isMsgBindCollidedGround(msg) || rs::isMsgEnableMapCheckPointWarp(msg))
            return al::isCollidedGround(this);
    }

    bool isBindKeepActive = rs::isActiveBindKeepDemo(mBindKeepDemoInfo);
    if (isBindKeepActive) {
        if (rs::tryEndBindKeepDemo(mBindKeepDemoInfo, msg, mPlayerPuppet)) {
            if (mPlayerPuppet)
                rs::startPuppetAction(mPlayerPuppet, "SphinxRideRide");
            al::startBgm(this, "SphinxRide", 0, 0);
            al::setNerve(this, &NrvSphinxRide.Run);
            return true;
        }
    } else {
        if (rs::tryStartBindKeepDemo(mBindKeepDemoInfo, msg, mPlayerPuppet))
            return true;
    }

    if (!mPlayerPuppet) {
        if (!al::isSensorPlayer(self)) {
            if (mStateReaction->receiveMsg(msg, self, other))
                goto doReaction;
        }
        if (mStateReaction->receiveMsgWithoutTrample(msg, self, other))
            goto doReaction;
        if (false) {
        doReaction:
            if (al::isNerve(this, &NrvSphinxRide.Reaction))
                return true;
            if (_208)
                mStateReaction->mParam = &sReactionCapStandbyParam;
            else
                mStateReaction->mParam = &sReactionWaitParam;
            al::setNerve(this, &NrvSphinxRide.Reaction);
            return true;
        }
        if (mStateReaction->receiveMsgNoReaction(msg, self, other))
            return true;
    }

    if (al::isSensorPlayer(other) &&
        (al::isSensorNpc(self) || al::isSensorHostName(self, "メガネー")))
        return al::tryReceiveMsgPushAndAddVelocityH(this, msg, self, other, 2.0f);

    return false;
}

void SphinxRide::startClipped() {
    al::LiveActor::startClipped();
    if (mBgmLineChangeCount >= 1) {
        mBgmLineChangeCount = 0;
        al::enableBgmLineChange(this);
    }
}

bool SphinxRide::receiveEvent(const al::EventFlowEventData* event) {
    if (al::isEventName(event, "PayCoinToSphinx")) {
        GameDataFunction::isPayCoinToSphinx(this);
        mIsCanRidePlacement = true;
        GameDataFunction::payCoinToSphinx(this);
        return true;
    }

    if (al::isEventName(event, "TurnToStartDir")) {
        al::setNerve(this, &NrvSphinxRide.DemoStandbyStart);
        return true;
    }

    if (al::isEventName(event, "NoTurnToStartDir")) {
        al::setNerve(this, &NrvSphinxRide.DemoStandbyTurnZero);
        return true;
    }

    if (al::isEventName(event, "ValidateTalkPushSensor")) {
        al::validateHitSensor(this, "PushTalk");
        return true;
    }

    return false;
}

const char* SphinxRide::judgeQuery(const char* query) const {
    if (!al::isEqualString(query, "SphinxTurn"))
        return nullptr;
    if (al::isNearZero(mStartDegree, 0.001f))
        return "No";
    return "Yes";
}

void SphinxRide::disappear() {
    al::startHitReaction(this, u8"消滅");
    al::hideModelIfShow(this);
}

bool SphinxRide::isCanRidePlacement() {
    return mIsCanRidePlacement;
}

void SphinxRide::exeWait() {
    if (al::isFirstStep(this)) {
        al::invalidateHitSensor(this, "BindFront");
        al::invalidateHitSensor(this, "BindBack");
        al::startAction(this, "Wait");
    }

    rs::updateEventFlow(mEventFlowExecutor);

    if (al::isOnGround(this, 0)) {
        const al::CollisionParts* parts = al::tryGetCollidedGroundCollisionParts(this);
        mCollisionPartsConnector->init(&parts->getBaseMtx(), parts->getBaseInvMtx(), parts);
    }
}

// NON_MATCHING: IUseCollision MI null check codegen pattern, data section offsets
void SphinxRide::exeStandby() {
    if (al::isFirstStep(this)) {
        al::tryStartActionIfNotPlaying(this, "Standby");
        al::validateHitSensor(this, "BindFront");
        al::validateHitSensor(this, "BindBack");
        al::invalidateHitSensor(this, "PushTalk");
        mAllRootRotateZTarget = 0.0f;
        mPlayerRotateZTarget = 0.0f;
        mAdlibCountdown = 292;
        mIsAdlibPlaying = false;
    }

    if (al::isOnGround(this, 0)) {
        al::scaleVelocityParallelVertical(this, al::getCollidedGroundNormal(this), 0.0f, 0.5f);
    } else {
        al::scaleVelocityHV(this, 0.5f, 0.98f);
    }
    al::addVelocityToGravityNaturalOrFittedGround(this, 1.0f);

    sead::Vector3f arrowStart = al::getTrans(this) + sArrowCheckOffset;
    if (alCollisionUtil::checkStrikeArrow(this, arrowStart, sArrowCheckDir, nullptr, nullptr) ==
            0 &&
        !al::isOnGround(this, 15)) {
        al::setNerve(this, &NrvSphinxRide.Fall);
        return;
    }

    if (mIsAdlibPlaying) {
        if (al::isActionEnd(this)) {
            al::tryStartActionIfNotPlaying(this, "Standby");
            mIsAdlibPlaying = false;
            mAdlibCountdown = 292;
        }
    } else {
        if (--mAdlibCountdown <= 0) {
            const char* adlibName;
            if (al::isHalfProbability())
                adlibName = "AdlibStandbyA";
            else
                adlibName = "AdlibStandbyB";
            al::startAction(this, adlibName);
            mIsAdlibPlaying = true;
        }
    }

    trySlipOnMoveLimit();
    rs::updateEventFlow(mEventFlowExecutor);
}

void SphinxRide::setNerveFall() {
    al::setNerve(this, &NrvSphinxRide.Fall);
}

void SphinxRide::trySlipOnMoveLimit() {
    if (!al::isOnGround(this, 0))
        return;

    const al::CollisionParts* parts = al::tryGetCollidedGroundCollisionParts(this);
    if (!parts)
        return;

    // Check special purpose name (offset 0x140 in CollisionParts is mSpecialPurpose)
    const char* purpose = reinterpret_cast<const char*>(
        *reinterpret_cast<const uintptr_t*>(reinterpret_cast<const u8*>(parts) + 0x140));
    if (!purpose || !al::isEqualString(purpose, "MoveLimit"))
        return;

    const sead::Vector3f& normal = al::getOnGroundNormal(this, 0);
    f32 angle = al::calcAngleDegree(normal, sead::Vector3f::ey);
    if (angle <= 30.0f)
        return;

    // cross = ey × normal
    sead::Vector3f cross;
    cross.x = 1.0f * normal.x - normal.y * 0.0f;
    cross.y = normal.y * 0.0f - normal.z * 1.0f;
    cross.z = normal.z * 0.0f - 0.0f * normal.x;
    if (al::isNearZero(cross, 0.001f))
        return;

    al::tryNormalizeOrDirZ(&cross);

    // slip = normal × cross
    sead::Vector3f slip;
    slip.x = normal.y * cross.z - normal.z * cross.y;
    slip.y = normal.z * cross.x - cross.z * normal.x;
    slip.z = cross.y * normal.x - normal.y * cross.x;

    sead::Vector3f slipVel = {slip.x * 6.0f, slip.y * 6.0f, slip.z * 6.0f};
    al::addVelocity(this, slipVel);
    al::limitVelocityDir(this, slip, 300.0f);
}

void SphinxRide::exeReaction() {
    if (al::isOnGround(this, 0)) {
        al::scaleVelocityParallelVertical(this, al::getCollidedGroundNormal(this), 0.0f, 0.5f);
    } else {
        al::scaleVelocityHV(this, 0.5f, 0.98f);
    }
    al::addVelocityToGravityNaturalOrFittedGround(this, 1.0f);

    if (al::updateNerveState(this)) {
        if (mIsCanRidePlacement)
            al::setNerve(this, &NrvSphinxRide.Standby);
        else
            al::setNerve(this, &NrvSphinxRide.Wait);
    }
}

// NON_MATCHING: demo callback system
void SphinxRide::exeDemoStandbyStart() {}

// NON_MATCHING: demo callback system
void SphinxRide::exeDemoStandbyTurnZero() {}

void SphinxRide::exeDemoTurnEnd() {
    if (al::isFirstStep(this))
        al::startAction(this, "TurnEnd");

    if (al::isStep(this, 12)) {
        al::validateShadowMask(this, "StandByFront");
        al::validateShadowMask(this, "StandByBack");
        al::invalidateShadowMask(this, "Wait");
    }

    if (al::isActionEnd(this))
        al::setNerve(this, &NrvSphinxRide.Standby);
}

void SphinxRide::exeGetOnStart() {
    if (rs::isPuppetActionEnd(mPlayerPuppet))
        al::setNerve(this, &GetOn);
}

void SphinxRide::exeGetOnStartOn() {
    if (al::isFirstStep(this))
        rs::startPuppetAction(mPlayerPuppet, "SphinxRideRideOn");

    f32 frameMax = rs::getPuppetAnimFrameMax(mPlayerPuppet);
    if (al::isGreaterEqualStep(this, (s32)frameMax))
        al::setNerve(this, &GetOn);
}

// NON_MATCHING: complex get on handling
void SphinxRide::exeGetOn() {
    if (al::isFirstStep(this)) {
        if (mPlayerPuppet)
            rs::startPuppetAction(mPlayerPuppet, "SphinxRideGetOn");
        al::startAction(this, "GetOn");
        BindInfo info = {"SphinxRide"};
        rs::tryAppearBindTutorial(this, info);
        rs::rideSphinx(this);
        al::showSilhouetteModel(this);
        rs::hidePuppetShadow(mPlayerPuppet);
        if (!al::isNear(mCameraAngle, -1.0f, 0.001f))
            rs::requestSetCameraAngleV(this, mCameraAngle, 0);
        *(reinterpret_cast<u8*>(mJointLookAtController) + 82) = 0;
    }

    rs::requestCameraTurnToFront(this, 0.5f, 0.2f, 0);

    if (al::isActionEnd(this)) {
        al::startBgm(this, "SphinxRide", 0, 0);
        al::setNerve(this, &NrvSphinxRide.Run);
    }
}

bool SphinxRide::isValidateCameraAngleV(f32 angle) {
    return !al::isNear(angle, -1.0f, 0.001f);
}

// NON_MATCHING: complex run state with rumble, wall collision, animation rate
void SphinxRide::exeRun() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Run");
        rs::startPuppetAction(mPlayerPuppet, "SphinxRideRide");
        rs::setPuppetAnimRate(mPlayerPuppet, 1.0f);
        _1c0 = 0;
    }

    if (tryGetOffAndCancelBind())
        return;

    // Rumble based on action and frame
    if (al::isActionPlaying(this, "Run")) {
        s32 idx = getMaterialRumbleIndex(this);
        const RumbleParam& param = sRunRumbleParams[idx];
        f32 frame = al::getActionFrame(this);
        if (frame >= 6.0f && frame <= 7.0f)
            alPadRumbleFunction::startPadRumbleWithVolume(this, param.name, param.volumeRight,
                                                          param.volumeLeft);
        frame = al::getActionFrame(this);
        if (frame >= 9.0f && frame <= 10.0f)
            alPadRumbleFunction::startPadRumbleWithVolume(this, param.name, param.volumeLeft,
                                                          param.volumeRight);
        frame = al::getActionFrame(this);
        if (frame >= 19.0f && frame <= 20.0f)
            alPadRumbleFunction::startPadRumbleWithVolume(this, param.name, param.volumeRight,
                                                          param.volumeLeft);
        frame = al::getActionFrame(this);
        if (frame >= 0.0f && frame <= 1.0f)
            alPadRumbleFunction::startPadRumbleWithVolume(this, param.name, param.volumeLeft,
                                                          param.volumeRight);
    } else if (al::isActionPlaying(this, "RunSlow")) {
        f32 frame = al::getActionFrame(this);
        bool shouldRumble = false;
        if (frame >= 7.0f && frame <= 9.0f)
            shouldRumble = true;
        if (!shouldRumble) {
            frame = al::getActionFrame(this);
            if (frame >= 0.0f && frame <= 2.0f)
                shouldRumble = true;
        }
        if (shouldRumble) {
            s32 idx = getMaterialRumbleIndex(this);
            const RumbleParam& param = sRunSlowRumbleParams[idx];
            alPadRumbleFunction::startPadRumbleWithVolume(this, param.name, param.volumeLeft,
                                                          param.volumeRight);
        }
    }

    // Wall collision detection and action switching
    bool isCollidedWall = al::isCollidedWallVelocity(this);
    bool isRunSlow = al::isActionPlaying(this, "RunSlow");
    sead::Vector3f hVel, vVel;
    al::separateVelocityHV(&hVel, &vVel, this);

    bool switchedToSlow = false;
    if (al::isGreaterEqualStep(this, _1c0 + 10)) {
        if (isCollidedWall && hVel.length() < 20.0f) {
            mLastAttackSensor = nullptr;
            switchedToSlow = true;
        } else {
            al::HitSensor* saved = mLastAttackSensor;
            mLastAttackSensor = nullptr;
            if (saved)
                switchedToSlow = true;
        }
    } else {
        mLastAttackSensor = nullptr;
        if (isRunSlow)
            switchedToSlow = true;
    }

    if (switchedToSlow) {
        if (al::tryStartActionIfNotPlaying(this, "RunSlow")) {
            rs::startPuppetAction(mPlayerPuppet, "SphinxRideRunSlow");
            rs::setPuppetAnimRate(mPlayerPuppet, 1.0f);
            _1c0 = al::getNerveStep(this);
        }
    } else {
        if (al::tryStartActionIfNotPlaying(this, "Run")) {
            rs::startPuppetAction(mPlayerPuppet, "SphinxRideRide");
            rs::setPuppetAnimRate(mPlayerPuppet, 1.0f);
            _1c0 = al::getNerveStep(this);
        }
        s32 step = al::getNerveStep(this);
        f32 rate = (1.0f - fminf((f32)step / 60.0f, 1.0f)) * 0.6f + 1.0f;
        rs::setPuppetAnimRate(mPlayerPuppet, al::getActionFrameRate(this));
        al::setActionFrameRate(this, rate);
    }

    // Movement
    al::separateVelocityHV(&hVel, &vVel, this);
    f32 hSpeed = hVel.length();
    f32 turnRate = (hSpeed / 50.0f < 0.8f) ? 0.7f : 1.4f;
    updateRun(turnRate);

    // Player rotate Z calculation from view input
    const sead::Vector2f& moveStick = rs::getPuppetMoveStick(mPlayerPuppet);
    const sead::Matrix34f* viewMtx = al::getViewMtxPtr(this, 0);
    sead::Vector3f viewInput;
    al::calcDirViewInput(&viewInput, moveStick, sead::Vector3f::ey, viewMtx);
    sead::Vector3f frontDir;
    al::calcFrontDir(&frontDir, this);

    f32 angle = 0.0f;
    if (!al::tryCalcAngleOnPlaneDegree(&angle, viewInput, frontDir, sead::Vector3f::ey)) {
        angle = 0.0f;
    }

    f32 clampedPos;
    if (angle > 0.0f)
        clampedPos = fminf(angle, 90.0f);
    else
        clampedPos = fmaxf(angle, -90.0f);
    mAllRootRotateZTarget = (clampedPos / 90.0f) * 25.0f;
    f32 forNeg = fminf(angle, 90.0f);
    f32 negClamped = fmaxf(angle, -90.0f);
    mPlayerRotateZTarget = ((angle <= 0.0f ? negClamped : forNeg) / 90.0f) * 25.0f;
}

// NON_MATCHING: bind keep demo check and getoff trigger
bool SphinxRide::tryGetOffAndCancelBind() {
    if (rs::isActiveBindKeepDemo(mBindKeepDemoInfo))
        return false;
    if (!rs::isTriggerGetOff(mPlayerPuppet))
        return false;

    requestGetOffForce();
    _230 = 10;
    al::invalidateHitSensor(this, "BindFront");
    al::invalidateHitSensor(this, "BindBack");
    al::disableBgmLineChange(this);
    al::stopBgm(this, "SphinxRide", 120);
    mBgmLineChangeCount = 120;
    rs::tryCloseBindTutorial(this);

    if (al::isOnGround(this, 0))
        al::setNerve(this, &NrvSphinxRide.GetOff);
    else
        al::setNerve(this, &NrvSphinxRide.Fall);
    return true;
}

// NON_MATCHING: IUseCollision MI cast, missing collision effect code block, regalloc
void SphinxRide::updateRun(f32 turnRate) {
    static f32 sGravScaleTable[] = {0.95f, 0.7f};

    bool isPuppetSwing = rs::isPuppetInputSwing(mPlayerPuppet);
    bool isPuppetButton = rs::isPuppetTriggerAnyButton(mPlayerPuppet);

    if (isPuppetSwing || isPuppetButton) {
        if (isPuppetSwing)
            *(reinterpret_cast<u8*>(mSphinxRideAim) + 40) = 1;
        al::setNerve(this, &NrvSphinxRide.Stop);
        return;
    }

    sead::Vector3f arrowStart = al::getTrans(this) + sArrowCheckOffset;
    if (alCollisionUtil::checkStrikeArrow(this, arrowStart, sArrowCheckDir, nullptr, nullptr) ==
            0 &&
        !al::isOnGround(this, 15)) {
        al::setNerve(this, &NrvSphinxRide.Fall);
        return;
    }

    if (sendMsgCollidedCactus()) {
        _138 = false;
        f32 gravScale = 0.95f;

        bool isCollidedWall = al::isCollidedWallVelocity(this);
        bool isRun = al::isNerve(this, &NrvSphinxRide.Run);
        controlAndRotateYAndMove(turnRate, 2.5f, isRun & !isCollidedWall);

        f32 addGrav = al::isOnGround(this, 0) ? 1.0f : 4.3f;
        addGravityScaleAndLimitVelocity(this, addGrav, gravScale, 0.9f);
        trySlipOnMoveLimit();

        if (al::isOnGround(this, 3)) {
            const char* material = al::getCollidedFloorMaterialCodeName(this);
            al::tryUpdateSeMaterialCode(this, material);
        } else {
            al::tryUpdateEffectMaterialCode(this, "NoCollide");
        }
        return;
    }

    bool isCollidedWall = al::isCollidedWallVelocity(this);
    bool isAngleLessThan45 = false;

    if (isCollidedWall) {
        const sead::Vector3f& wallNormal = al::getCollidedWallNormal(this);
        sead::Vector3f frontDir;
        al::calcFrontDir(&frontDir, this);
        sead::Vector3f negNormal = {-wallNormal.x, -wallNormal.y, -wallNormal.z};
        f32 angleDeg = al::calcAngleDegree(frontDir, negNormal);
        isAngleLessThan45 = angleDeg < 45.0f;

        if (!_138 && angleDeg < 45.0f) {
            sead::Vector3f hVel, vVel;
            al::separateVelocityHV(&hVel, &vVel, this);
            if (hVel.length() >= 30.0f) {
                bool isMoveLimit = false;
                bool touched = sendMsgCollidedTouch();

                if (!isMoveLimit || touched) {
                    al::getCollidedWallPos(this);
                    al::startHitReaction(this, "壁当たり");
                    al::startAction(this, "ClashWall");
                    al::setNerve(this, &NrvSphinxRide.Clash);
                    return;
                }
            }
        }
    }

    _138 = isCollidedWall;
    f32 gravScale = sGravScaleTable[isAngleLessThan45 ? 1 : 0];

    bool isRun = al::isNerve(this, &NrvSphinxRide.Run);
    controlAndRotateYAndMove(turnRate, 2.5f, isRun & !isCollidedWall);

    f32 addGrav = al::isOnGround(this, 0) ? 1.0f : 4.3f;
    addGravityScaleAndLimitVelocity(this, addGrav, gravScale, 0.9f);
    trySlipOnMoveLimit();

    if (al::isOnGround(this, 3)) {
        const char* material = al::getCollidedFloorMaterialCodeName(this);
        al::tryUpdateSeMaterialCode(this, material);
    } else {
        al::tryUpdateEffectMaterialCode(this, "NoCollide");
    }
}

void SphinxRide::exeClash() {
    if (al::isFirstStep(this)) {
        sead::Vector3f hVel, vVel;
        al::separateVelocityHV(&hVel, &vVel, this);
        al::tryNormalizeOrDirZ(&hVel);
        sead::Vector3f negHVel = {-hVel.x, -hVel.y, -hVel.z};
        al::setVelocityToDirection(this, negHVel, 10.0f);
        mAllRootRotateZTarget = 0.0f;
        mPlayerRotateZTarget = 0.0f;
        if (mPlayerPuppet)
            rs::startPuppetAction(mPlayerPuppet, "SphinxRideClash");
    }

    updateGravityAndDump(0.95f, 0.9f);

    if (al::isActionEnd(this))
        al::setNerve(this, &NrvSphinxRide.Run);
}

static void addGravityScaleAndLimitVelocity(SphinxRide* actor, f32 gravity, f32 hScale,
                                            f32 vScale) {
    sead::Vector3f gravDir;
    if (al::isOnGround(actor, 0)) {
        const sead::Vector3f& normal = al::getCollidedGroundNormal(actor);
        gravDir = {-normal.x, -normal.y, -normal.z};
    } else {
        gravDir = {0.0f, -1.0f, 0.0f};
    }

    al::scaleVelocityHV(actor, hScale, vScale);
    al::addVelocity(actor, gravDir * gravity);

    sead::Vector3f frontDir;
    al::calcFrontDir(&frontDir, actor);
    al::limitVelocityDirSign(actor, frontDir, 50.0f);
}

void SphinxRide::updateGravityAndDump(f32 hScale, f32 vScale) {
    f32 gravity = al::isOnGround(this, 0) ? 1.0f : 4.3f;
    addGravityScaleAndLimitVelocity(this, gravity, hScale, vScale);
}

// NON_MATCHING: IUseCollision MI null check codegen pattern, data section offsets
void SphinxRide::exeStop() {
    if (al::isFirstStep(this)) {
        mPlayerRotateZTarget = 0.0f;
        mAllRootRotateZTarget = 0.0f;
    }

    const char* material = al::getCollidedFloorMaterialCodeName(this);
    s32 idx;
    if (al::isCollidedFloorCode(this, "Poison") ||
        al::isCollidedGroundFloorCode(this, "Poison")) {
        idx = 2;
    } else if (material) {
        bool isStoneRough = al::isEqualString(material, "StoneRough");
        bool isSandDesert = al::isEqualString(material, "SandDesert");
        idx = isStoneRough & 1;
        if (isSandDesert)
            idx = 0;
    } else {
        idx = 0;
    }

    alPadRumbleFunction::startPadRumbleWithVolume(
        this, sStopRumbleParams[idx].name, sStopRumbleParams[idx].volumeLeft,
        sStopRumbleParams[idx].volumeLeft);
    sendMsgCollidedCactus();

    if (al::updateNerveState(this)) {
        if (!*(reinterpret_cast<bool*>(mStateStop) + 52))
            al::setNerve(this, &NrvSphinxRide.Run);
    } else {
        sead::Vector3f arrowStart = al::getTrans(this) + sArrowCheckOffset;
        if (alCollisionUtil::checkStrikeArrow(this, arrowStart, sArrowCheckDir, nullptr,
                                              nullptr) == 0 &&
            !al::isOnGround(this, 15))
            al::setNerve(this, &NrvSphinxRide.Fall);
    }
}

// NON_MATCHING: zero-init from const pool, member-by-member Vector3f copy codegen
bool SphinxRide::sendMsgCollidedCactus() {
    if (!al::isCollidedWallVelocity(this))
        return false;

    sead::Vector3f wallPos = al::getCollidedWallPos(this);
    sead::Vector3f wallNormal = al::getCollidedWallNormal(this);
    al::HitSensor* wallSensor = al::getCollidedWallSensor(this);
    al::HitSensor* bodySensor = al::getHitSensor(this, "PlayerBody");

    if (!rs::sendMsgSphinxRideAttackTouchThrough(wallSensor, bodySensor, wallPos, wallNormal))
        return false;

    al::startHitReactionHitEffect(this, "スフィンクス壁突進", wallPos);
    return true;
}

// NON_MATCHING: IUseCollision MI null check codegen, data offsets
void SphinxRide::exeFall() {
    if (al::isFirstStep(this)) {
        mAllRootRotateZTarget = 0.0f;
        mPlayerRotateZTarget = 0.0f;
        al::startAction(this, "Fall");
        if (mPlayerPuppet)
            rs::startPuppetAction(mPlayerPuppet, "SphinxRideFall");
    }

    updateGravityAndDump(0.95f, 0.9f);

    if (mPlayerPuppet) {
        sead::Vector3f inputDir;
        al::calcFrontDir(&inputDir, this);
        const sead::Vector2f& moveStick = rs::getPuppetMoveStick(mPlayerPuppet);
        const sead::Matrix34f* viewMtx = al::getViewMtxPtr(this, 0);
        al::calcVecViewInput(&inputDir, moveStick, sead::Vector3f::ey, viewMtx);
        al::turnToDirection(this, inputDir, 0.7f);
        sead::Vector3f newFront;
        al::calcFrontDir(&newFront, this);
        al::addVelocity(this, newFront);
    }

    if (al::isOnGround(this, 0))
        al::setNerve(this, &Land);
}

// NON_MATCHING: movement and rotation
void SphinxRide::controlAndRotateYAndMove(f32 turnRate, f32 speed, bool adjustSlope) {
    sead::Vector3f inputDir;
    al::calcFrontDir(&inputDir, this);
    const sead::Vector2f& moveStick = rs::getPuppetMoveStick(mPlayerPuppet);
    const sead::Matrix34f* viewMtx = al::getViewMtxPtr(this, 0);
    al::calcVecViewInput(&inputDir, moveStick, sead::Vector3f::ey, viewMtx);
    al::turnToDirection(this, inputDir, turnRate);

    sead::Vector3f frontDir;
    al::calcFrontDir(&frontDir, this);
    if (adjustSlope && calcSlopeProjectedFront(&frontDir, this))
        al::tryNormalizeOrDirZ(&frontDir);

    sead::Vector3f velocity = {frontDir.x * speed, frontDir.y * speed, frontDir.z * speed};
    al::addVelocity(this, velocity);
}

void SphinxRide::exeLand() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Land");
        if (!mPlayerPuppet)
            al::setVelocityZero(this);
    }

    updateGravityAndDump(0.95f, 0.9f);

    if (al::isActionEnd(this)) {
        if (mPlayerPuppet) {
            al::setNerve(this, &NrvSphinxRide.Run);
        } else {
            al::startAction(this, "Standby");
            al::setNerve(this, &NrvSphinxRide.Standby);
        }
    }
}

// NON_MATCHING: IUseCollision MI null check codegen, data offsets
void SphinxRide::exeGetOff() {
    if (al::isFirstStep(this)) {
        mAllRootRotateZTarget = 0.0f;
        mPlayerRotateZTarget = 0.0f;
        al::startAction(this, "GetOff");
    }

    if (al::isOnGround(this, 0)) {
        al::scaleVelocityParallelVertical(this, al::getCollidedGroundNormal(this), 0.0f, 0.5f);
    } else {
        al::scaleVelocityHV(this, 0.5f, 0.98f);
    }
    al::addVelocityToGravityNaturalOrFittedGround(this, 1.0f);

    sead::Vector3f arrowStart = al::getTrans(this) + sArrowCheckOffset;
    if (alCollisionUtil::checkStrikeArrow(this, arrowStart, sArrowCheckDir, nullptr, nullptr) ||
        al::isOnGround(this, 15)) {
        trySlipOnMoveLimit();
        if (!al::isActionEnd(this))
            return;
        if (!mStateRevival)
            al::validateClipping(this);
        al::setNerve(this, &NrvSphinxRide.Standby);
    } else {
        al::setNerve(this, &NrvSphinxRide.Fall);
    }
}

// NON_MATCHING: IUseCollision MI null check codegen, data offsets
void SphinxRide::exeJump() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "JumpStart");
        if (mPlayerPuppet)
            rs::startPuppetAction(mPlayerPuppet, "SphinxRideJumpStart");
        al::addVelocityJump(this, _1bc * 1.25f);
        mAllRootRotateZTarget = 0.0f;
        mPlayerRotateZTarget = 0.0f;
        al::requestCancelCameraInterpole(this, 0);
    }

    if (al::isActionPlaying(this, "JumpStart") && al::isActionEnd(this)) {
        al::startAction(this, "Jump");
        if (mPlayerPuppet)
            rs::startPuppetAction(mPlayerPuppet, "SphinxRideJump");
    }

    if (mPlayerPuppet) {
        sead::Vector3f inputDir;
        al::calcFrontDir(&inputDir, this);
        const sead::Vector2f& moveStick = rs::getPuppetMoveStick(mPlayerPuppet);
        const sead::Matrix34f* viewMtx = al::getViewMtxPtr(this, 0);
        al::calcVecViewInput(&inputDir, moveStick, sead::Vector3f::ey, viewMtx);
        al::turnToDirection(this, inputDir, 0.7f);
        sead::Vector3f newFront;
        al::calcFrontDir(&newFront, this);
        al::addVelocity(this, newFront);
    }

    addGravityScaleAndLimitVelocity(this, 1.29f, 0.95f, 0.98f);

    if (al::isOnGround(this, 0))
        al::setNerve(this, &Land);
}

void SphinxRide::exeRevival() {
    if (al::updateNerveState(this)) {
        al::startHitReaction(this, u8"出現");
        al::setNerve(this, &NrvSphinxRide.Standby);
    }
}

void SphinxRide::exeEventWait() {
    if (al::isFirstStep(this)) {
        al::setVelocity(this, sead::Vector3f::zero);
        al::startAction(this, "Wait");
    }

    addGravityScaleAndLimitVelocity(this, al::isOnGround(this, 0) ? 1.0f : 4.3f, 0.95f, 0.9f);
}

bool SphinxRide::isRidePlayer() const {
    return mPlayerPuppet != nullptr;
}

bool SphinxRide::isPlayerInputSwingSphinxStop() const {
    return rs::isPuppetInputSwing(mPlayerPuppet);
}

// NON_MATCHING: zero-init from const pool, member-by-member Vector3f copy codegen
// NON_MATCHING: same codegen pattern as sendMsgCollidedCactus
bool SphinxRide::sendMsgCollidedTouch() {
    if (!al::isCollidedWallVelocity(this))
        return false;

    sead::Vector3f wallPos = al::getCollidedWallPos(this);
    sead::Vector3f wallNormal = al::getCollidedWallNormal(this);
    al::HitSensor* wallSensor = al::getCollidedWallSensor(this);
    al::HitSensor* bodySensor = al::getHitSensor(this, "PlayerBody");

    if (!rs::sendMsgSphinxRideAttackTouch(wallSensor, bodySensor, wallPos, wallNormal))
        return false;

    al::startHitReactionHitEffect(this, "スフィンクス壁突進", wallPos);
    return true;
}

bool SphinxRide::calcCheckCollidedWallCommon(sead::Vector3f* wallPos, sead::Vector3f* wallNormal,
                                             al::HitSensor** bodySensor,
                                             al::HitSensor** wallSensor) {
    if (!al::isCollidedWallVelocity(this))
        return false;
    *wallPos = al::getCollidedWallPos(this);
    *wallNormal = al::getCollidedWallNormal(this);
    *wallSensor = al::getCollidedWallSensor(this);
    *bodySensor = al::getHitSensor(this, "PlayerBody");
    return true;
}

// NON_MATCHING: vtable offset for makeActorAlive (0x18 vs 0x20)
void SphinxRide::startStandby(const sead::Vector3f& trans, const sead::Vector3f& front) {
    if (al::isDead(this)) {
        al::showModelIfHide(this);
        makeActorAlive();
    }
    al::resetRotatePosition(this, trans, front);
    al::calcUpDir(&mUpDir, this);
    al::calcFrontDir(&mFrontDir, this);
    rs::startEventFlow(mEventFlowExecutor, "Standby");
    al::setNerve(this, &NrvSphinxRide.Standby);
}

bool SphinxRide::isNerveStandby() const {
    return al::isNerve(this, &NrvSphinxRide.Standby);
}

void SphinxRide::setupEventFade() {
    if (mPlayerPuppet)
        rs::setPuppetAnimRate(mPlayerPuppet, 0.0f);
}

void SphinxRide::startEventWait() {
    al::setNerve(this, &NrvSphinxRide.EventWait);
}

void SphinxRide::endEventWait(f32 startDegree) {
    if (al::isNearZero(startDegree, 0.001f)) {
        mStartDegree = startDegree;
        al::setNerve(this, &NrvSphinxRide.DemoStandbyTurnZero);
    } else {
        al::setNerve(this, &NrvSphinxRide.DemoStandbyStart);
    }
}

f32 calcRunSpeedRate(const al::LiveActor* actor) {
    sead::Vector3f hVel, vVel;
    al::separateVelocityHV(&hVel, &vVel, actor);
    return hVel.length() / 50.0f;
}
