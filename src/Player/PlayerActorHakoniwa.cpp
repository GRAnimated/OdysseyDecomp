#include "Player/PlayerActorHakoniwa.h"

#include "Library/Area/AreaObj.h"
#include "Library/Area/AreaObjGroup.h"
#include "Library/Area/AreaObjUtil.h"
#include "Library/Base/StringUtil.h"
#include "Library/Bgm/BgmLineFunction.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Demo/DemoActor.h"
#include "Library/Execute/ExecuteUtil.h"
#include "Library/Layout/LayoutActorUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/ComboCounter.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Nature/WaterSurfaceFinder.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Obj/FootPrintHolder.h"
#include "Library/Obj/FootPrintServer.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Scene/SceneObjUtil.h"
#include "Library/Screen/ScreenFunction.h"
#include "Library/Shadow/ActorShadowUtil.h"
#include "Library/Stage/StageSwitchUtil.h"
#include "Library/Thread/FunctorV0M.h"
#include "Project/HitSensor/HitSensor.h"

#include "Layout/GaugeAir.h"
#include "MapObj/CapMessageShowInfo.h"
#include "Player/ActorStateSandGeyser.h"
#include "Player/HackCap.h"
#include "Player/HackCapJudgeHoldHoveringJump.h"
#include "Player/HackCapJudgePreInputHoveringJump.h"
#include "Player/HackCapJudgePreInputSeparateJump.h"
#include "Player/HackCapJudgePreInputSeparateThrow.h"
#include "Player/InitPlayerModelHolder.h"
#include "Player/PlayerActionDiveInWater.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerAreaChecker.h"
#include "Player/PlayerBindKeeper.h"
#include "Player/PlayerCapActionHistory.h"
#include "Player/PlayerCapFunction.h"
#include "Player/PlayerCapManHeroEyesControl.h"
#include "Player/PlayerCarryKeeper.h"
#include "Player/PlayerColliderHakoniwa.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerContinuousJump.h"
#include "Player/PlayerContinuousLongJump.h"
#include "Player/PlayerCounterAfterUpperPunch.h"
#include "Player/PlayerCounterForceRun.h"
#include "Player/PlayerCounterIceWater.h"
#include "Player/PlayerCounterQuickTurnJump.h"
#include "Player/PlayerDamageKeeper.h"
#include "Player/PlayerDemoActionFlag.h"
#include "Player/PlayerEffect.h"
#include "Player/PlayerEquipmentUser.h"
#include "Player/PlayerExternalVelocity.h"
#include "Player/PlayerEyeSensorHitHolder.h"
#include "Player/PlayerFormSensorCollisionArranger.h"
#include "Player/PlayerFunction.h"
#include "Player/PlayerHackKeeper.h"
#include "Player/PlayerHitPush.h"
#include "Player/PlayerInfo.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerInputFunction.h"
#include "Player/PlayerJointControlKeeper.h"
#include "Player/PlayerJudgeActiveCameraSubjective.h"
#include "Player/PlayerJudgeAirForceCount.h"
#include "Player/PlayerJudgeCameraInWater.h"
#include "Player/PlayerJudgeCameraSubjective.h"
#include "Player/PlayerJudgeCapCatch.h"
#include "Player/PlayerJudgeCapCatchPop.h"
#include "Player/PlayerJudgeDead.h"
#include "Player/PlayerJudgeDeadWipeStart.h"
#include "Player/PlayerJudgeDirectRolling.h"
#include "Player/PlayerJudgeDiveInWater.h"
#include "Player/PlayerJudgeDrawForward.h"
#include "Player/PlayerJudgeEnableGuideArrow.h"
#include "Player/PlayerJudgeEnablePeachAmiibo.h"
#include "Player/PlayerJudgeEnableStandUp.h"
#include "Player/PlayerJudgeFailureCameraSubjective.h"
#include "Player/PlayerJudgeForceLand.h"
#include "Player/PlayerJudgeForceRolling.h"
#include "Player/PlayerJudgeForceSlopeSlide.h"
#include "Player/PlayerJudgeGrabCeil.h"
#include "Player/PlayerJudgeInWater.h"
#include "Player/PlayerJudgeInvalidateInputFall.h"
#include "Player/PlayerJudgeLongFall.h"
#include "Player/PlayerJudgeOutInWater.h"
#include "Player/PlayerJudgePlaySwitchOnAreaWaitAnim.h"
#include "Player/PlayerJudgePoleClimb.h"
#include "Player/PlayerJudgePreInputCapThrow.h"
#include "Player/PlayerJudgePreInputHackAction.h"
#include "Player/PlayerJudgePreInputJump.h"
#include "Player/PlayerJudgeRecoveryLifeFast.h"
#include "Player/PlayerJudgeSafetyPointRecovery.h"
#include "Player/PlayerJudgeSameNerve.h"
#include "Player/PlayerJudgeSandSink.h"
#include "Player/PlayerJudgeSleep.h"
#include "Player/PlayerJudgeSlopeSlide.h"
#include "Player/PlayerJudgeSpeedCheckFall.h"
#include "Player/PlayerJudgeStartHipDrop.h"
#include "Player/PlayerJudgeStartRise.h"
#include "Player/PlayerJudgeStartRolling.h"
#include "Player/PlayerJudgeStartRun.h"
#include "Player/PlayerJudgeStartSquat.h"
#include "Player/PlayerJudgeStartWaterSurfaceRun.h"
#include "Player/PlayerJudgeStatusPoleClimb.h"
#include "Player/PlayerJudgeTalkGround.h"
#include "Player/PlayerJudgeTalkSwim.h"
#include "Player/PlayerJudgeWallCatch.h"
#include "Player/PlayerJudgeWallCatchInputDir.h"
#include "Player/PlayerJudgeWallHitDown.h"
#include "Player/PlayerJudgeWallHitDownForceRun.h"
#include "Player/PlayerJudgeWallHitDownRolling.h"
#include "Player/PlayerJudgeWallKeep.h"
#include "Player/PlayerJumpMessageRequest.h"
#include "Player/PlayerModelChangerHakoniwa.h"
#include "Player/PlayerModelHolder.h"
#include "Player/PlayerOxygen.h"
#include "Player/PlayerPainPartsKeeper.h"
#include "Player/PlayerPuppet.h"
#include "Player/PlayerPushReceiver.h"
#include "Player/PlayerRecoverySafetyPoint.h"
#include "Player/PlayerRippleGenerator.h"
#include "Player/PlayerSandSinkAffect.h"
#include "Player/PlayerSeCtrl.h"
#include "Player/PlayerSeparateCapFlag.h"
#include "Player/PlayerSpinCapAttack.h"
#include "Player/PlayerStainControl.h"
#include "Player/PlayerStateAbyss.h"
#include "Player/PlayerStateBind.h"
#include "Player/PlayerStateCameraSubjective.h"
#include "Player/PlayerStateCapCatchPop.h"
#include "Player/PlayerStateDamageFire.h"
#include "Player/PlayerStateDamageLife.h"
#include "Player/PlayerStateDamageSwim.h"
#include "Player/PlayerStateEndHack.h"
#include "Player/PlayerStateFallHakoniwa.h"
#include "Player/PlayerStateGrabCeil.h"
#include "Player/PlayerStateHack.h"
#include "Player/PlayerStateHeadSliding.h"
#include "Player/PlayerStateHipDrop.h"
#include "Player/PlayerStateJump.h"
#include "Player/PlayerStateLongJump.h"
#include "Player/PlayerStatePoleClimb.h"
#include "Player/PlayerStatePress.h"
#include "Player/PlayerStateRise.h"
#include "Player/PlayerStateRolling.h"
#include "Player/PlayerStateRunHakoniwa2D3D.h"
#include "Player/PlayerStateSandSink.h"
#include "Player/PlayerStateSlope.h"
#include "Player/PlayerStateSpinCap.h"
#include "Player/PlayerStateSquat.h"
#include "Player/PlayerStateSwim.h"
#include "Player/PlayerStateWait.h"
#include "Player/PlayerStateWallAir.h"
#include "Player/PlayerStateWallCatch.h"
#include "Player/PlayerTrigger.h"
#include "Player/PlayerWallActionHistory.h"
#include "Player/PlayerWetControl.h"
#include "Scene/SceneObjFactory.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "System/GameDataUtil.h"
#include "Util/ActorDimensionKeeper.h"
#include "Util/CameraUtil.h"
#include "Util/DemoUtil.h"
#include "Util/JudgeUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackFunction.h"
#include "Util/PlayerHackInputFunction.h"
#include "Util/PlayerTypeUtil.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"
#include "Util/SpecialBuildUtil.h"
#include "Util/StageInputFunction.h"
#include "Util/WaterSurfaceShadow.h"
#include "Util/WorldEndBorderKeeper.h"

namespace rs {

bool is2D(const IUseDimension* dimension);
u8 isIn2DArea(const IUseDimension* dimension);
void pushOutFrom2DArea(al::LiveActor* actor, const IUseDimension* dimension, f32 radius,
                       f32 halfHeight);
bool snap2DGravityPoseWithRotateCenter(al::LiveActor* actor, IUsePlayerCollision* collision,
                                       const IUseDimension* dimension, f32 diskHalfHeight,
                                       f32 offset, const sead::Vector3f& rotateCenter);

}  // namespace rs

namespace {

bool isEnableReactionCapCatch(const al::LiveActor* player, const PlayerCarryKeeper* carryKeeper,
                              const PlayerModelChangerHakoniwa* modelChanger,
                              const PlayerStateWait* stateWait, const PlayerStateJump* stateJump,
                              const PlayerStateWallAir* stateWallAir,
                              const PlayerStateSwim* stateSwim);

void updateSeparateCap(al::LiveActor* player, PlayerSeparateCapFlag* separateCapFlag, HackCap* cap,
                       PlayerAnimator* animator, PlayerColliderHakoniwa* collider,
                       PlayerModelChangerHakoniwa* modelChanger, PlayerCarryKeeper* carryKeeper,
                       PlayerPuppet* puppet, PlayerEquipmentUser* equipmentUser,
                       const PlayerConst* playerConst);

bool syncDimensionState(
    al::LiveActor* player, ActorDimensionKeeper* dimensionKeeper, PlayerColliderHakoniwa* collider,
    PlayerInput* input, const IUseDimension* dimension, PlayerModelChangerHakoniwa* modelChanger,
    const al::HitSensor* bindSensor, const PlayerPuppet* puppet, const PlayerConst* playerConst);

bool isEnableCollisionSnap(const al::LiveActor* player, const PlayerStateSpinCap* stateSpinCap,
                           const PlayerStateGrabCeil* stateGrabCeil,
                           const PlayerStateDamageFire* stateDamageFire);

void setNerveRollingFromGround(al::LiveActor* player, PlayerTrigger* trigger,
                               IUsePlayerCollision* collision);

bool tryDamageIceWater(const al::LiveActor* player, PlayerDamageKeeper* damageKeeper,
                       PlayerOxygen* oxygen, PlayerCounterIceWater* counterIceWater,
                       const PlayerConst* playerConst, PlayerTrigger* trigger,
                       PlayerJudgeDeadWipeStart* judgeDeadWipeStart);

void startRecoveryFromDeathArea(al::LiveActor* player, HackCap* hackCap,
                                PlayerCarryKeeper* carryKeeper,
                                PlayerBindKeeper* bindKeeper,
                                PlayerEquipmentUser* equipmentUser,
                                PlayerStateAbyss* stateAbyss);

bool processPlayerDamage(const al::LiveActor* player, PlayerDamageKeeper* damageKeeper,
                         PlayerCarryKeeper* carryKeeper, PlayerTrigger* trigger,
                         const PlayerConst* playerConst,
                         PlayerRecoverySafetyPoint* recoverySafetyPoint, const PlayerInput* input);

void syncSeparateCapVisibility(PlayerAnimator* animator, HackCap* cap,
                               PlayerModelChangerHakoniwa* modelChanger,
                               const PlayerSeparateCapFlag* separateCapFlag);

class PlayerActorHakoniwaNrvFall : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvWait : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvSquat : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvRun : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvSlope : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvRolling : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvSpinCap : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvJump : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvCapCatchPop : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvWallAir : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvWallCatch : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvGrabCeil : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvPoleClimb : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvHipDrop : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvHeadSliding : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvLongJump : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvSandSink : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvSandGeyser : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvRise : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvSwim : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvDamage : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvDamageSwim : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvDamageFire : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvPress : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvHack : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvEndHack : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvBind : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvCamera : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvAbyss : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvDemo : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

class PlayerActorHakoniwaNrvDead : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override;
};

bool isCarryActionInvalid(const al::LiveActor* player, const PlayerBindKeeper* bindKeeper,
                          const PlayerStateWallAir* stateWallAir,
                          const PlayerStateSwim* stateSwim,
                          const PlayerStatePoleClimb* statePoleClimb);

bool isPlayerActionInvalid(const al::LiveActor* player, const PlayerBindKeeper* bindKeeper);

struct {
    PlayerActorHakoniwaNrvFall Fall;
    PlayerActorHakoniwaNrvWait Wait;
    PlayerActorHakoniwaNrvSquat Squat;
    PlayerActorHakoniwaNrvRun Run;
    PlayerActorHakoniwaNrvSlope Slope;
    PlayerActorHakoniwaNrvRolling Rolling;
    PlayerActorHakoniwaNrvSpinCap SpinCap;
    PlayerActorHakoniwaNrvJump Jump;
    PlayerActorHakoniwaNrvCapCatchPop CapCatchPop;
    PlayerActorHakoniwaNrvWallAir WallAir;
    PlayerActorHakoniwaNrvWallCatch WallCatch;
    PlayerActorHakoniwaNrvGrabCeil GrabCeil;
    PlayerActorHakoniwaNrvPoleClimb PoleClimb;
    PlayerActorHakoniwaNrvHipDrop HipDrop;
    PlayerActorHakoniwaNrvHeadSliding HeadSliding;
    PlayerActorHakoniwaNrvLongJump LongJump;
    PlayerActorHakoniwaNrvSandSink SandSink;
    PlayerActorHakoniwaNrvSandGeyser SandGeyser;
    PlayerActorHakoniwaNrvRise Rise;
    PlayerActorHakoniwaNrvSwim Swim;
    PlayerActorHakoniwaNrvDamage Damage;
    PlayerActorHakoniwaNrvDamageSwim DamageSwim;
    PlayerActorHakoniwaNrvDamageFire DamageFire;
    PlayerActorHakoniwaNrvPress Press;
    PlayerActorHakoniwaNrvHack Hack;
    PlayerActorHakoniwaNrvEndHack EndHack;
    PlayerActorHakoniwaNrvBind Bind;
    PlayerActorHakoniwaNrvCamera Camera;
    PlayerActorHakoniwaNrvAbyss Abyss;
    PlayerActorHakoniwaNrvDemo Demo;
    PlayerActorHakoniwaNrvDead Dead;
} NrvPlayerActorHakoniwa;
}  // namespace

PlayerActorHakoniwa::PlayerActorHakoniwa(const char* name)
    : PlayerActorBase(name), mComboCounter(new al::ComboCounter()) {
}

void PlayerActorHakoniwa::initPlayer(const al::ActorInitInfo& actorInitInfo,
                                     const PlayerInitInfo& playerInitInfo) {
    using PlayerActorHakoniwaFunctor =
        al::FunctorV0M<PlayerActorHakoniwa*, void (PlayerActorHakoniwa::*)()>;

    al::initActorWithArchiveName(this, actorInitInfo, "PlayerActorHakoniwa", nullptr);
    rs::initPlayerActorInfo(this, playerInitInfo);
    al::onCollide(this);

    bool isMoon = false;
    al::tryGetArg(&isMoon, actorInitInfo, "IsMoon");
    al::tryGetArg(&mIsLongShadow, actorInitInfo, "IsLongShadow");
    bool isCapInvalid = false;
    al::tryGetArg(&isCapInvalid, actorInitInfo, "IsInvalidateCap");
    if (isCapInvalid)
        GameDataFunction::disableCapByPlacement(this);
    al::registerExecutorFunctor(
        "プレイヤー後処理", actorInitInfo.executeDirector,
        PlayerActorHakoniwaFunctor(this, &PlayerActorHakoniwa::updateModelShadowDropLength));

    mBodyHitSensor = al::getHitSensor(this, "Body");
    mDimensionKeeper = new ActorDimensionKeeper(this);
    mTrigger = new PlayerTrigger();

    IUseDimension* dimension = this;
    al::AudioKeeper* audioKeeper = getAudioKeeper();
    auto* modelHolder = new PlayerModelHolder(3);
    al::LiveActor* normalModel = new al::LiveActor("マリオモデル");
    const char* modelName = rs::getInitPlayerModelName(playerInitInfo);
    const char* capTypeName = rs::getInitCapTypeName(playerInitInfo);
    bool isClosetScenePlayer = rs::isClosetScenePlayer(playerInitInfo);
    PlayerCostumeInfo* costume = PlayerFunction::initMarioModelActor(
        normalModel, actorInitInfo, modelName, capTypeName, audioKeeper, isClosetScenePlayer);
    modelHolder->registerModel(normalModel, "Normal");

    InitPlayerModelHolder{modelHolder, actorInitInfo, costume, modelName}();
    mModelHolder = modelHolder;

    mPainPartsKeeper = new PlayerPainPartsKeeper(this, costume);
    if (rs::isNeedCreateNoseNeedle(playerInitInfo))
        mPainPartsKeeper->createNoseNeedle(mModelHolder, actorInitInfo);
    mConst = PlayerFunction::createMarioConst(isMoon ? "Moon" : "");
    auto* jointControlKeeper = new PlayerJointControlKeeper(
        mModelHolder->findModelActor("Normal"), mConst, costume->isNeedShrinkNose(),
        costume->isEnableBigEar(), costume->isFollowJoeStrap());
    mJointControlKeeper = jointControlKeeper;

    const PlayerConst* playerConst = mConst;
    auto* collider = new PlayerColliderHakoniwa(this, playerConst);
    collider->init();
    mCollider = collider;
    mInput = new PlayerInput(this, collider, dimension);
    mModelChanger =
        new PlayerModelChangerHakoniwa(this, mModelHolder, mPainPartsKeeper, costume, dimension);
    mAnimator = new PlayerAnimator(mModelHolder, PlayerFunction::createPlayerDitherAnimator(
                                                     static_cast<al::LiveActor*>(this), 200.0f));
    if (costume->isNeedFullFaceAnim())
        mAnimator->validateFullFaceAnim();
    mEffect = new PlayerEffect(this, mModelHolder, mJointControlKeeper->getEffectMtx());

    mContinuousJump = new PlayerContinuousJump(mConst);
    mContinuousLongJump = new PlayerContinuousLongJump(mConst);
    mDamageKeeper = new PlayerDamageKeeper(this, mEffect);
    mDemoActionFlag = new PlayerDemoActionFlag();
    mCapActionHistory = new PlayerCapActionHistory(this, mConst, mTrigger, mCollider);
    mAreaChecker = new PlayerAreaChecker(this, mModelHolder);
    mWaterSurfaceFinder = new al::WaterSurfaceFinder(this);
    mWorldEndBorderKeeper = new WorldEndBorderKeeper(this);
    mWallActionHistory = new PlayerWallActionHistory();
    mCounterAfterUpperPunch = new PlayerCounterAfterUpperPunch();
    mCounterForceRun = new PlayerCounterForceRun();
    mCounterQuickTurnJump = new PlayerCounterQuickTurnJump(mConst, mTrigger);
    mJumpMessageRequest = new PlayerJumpMessageRequest();
    mSandSinkAffect = new PlayerSandSinkAffect(this, mConst, mInput, mCollider, mEffect);
    mEyeSensorHitHolder = new PlayerEyeSensorHitHolder(128);
    mEyeSensorHitHolder->createTargetMarkerBuffer(128);
    mPushReceiver = new PlayerPushReceiver(this);
    mHitPush = new PlayerHitPush(this, mConst);
    mExternalVelocity = new PlayerExternalVelocity(this, mCollider, al::getTransPtr(this));
    al::HitSensor* bodyHitSensor = mBodyHitSensor;
    al::LiveActor* currentModel = mModelHolder->getCurrentModelActor();
    auto* footPrintHolder =
        new al::FootPrintHolder(currentModel, "PlayerAnimation", bodyHitSensor,
                                static_cast<al::FootPrintServer*>(
                                    al::getSceneObj(currentModel, SceneObjID_alFootPrintServer)));
    footPrintHolder->clearAnimationNames();
    mFootPrintHolder = footPrintHolder;
    mSeparateCapFlag = new PlayerSeparateCapFlag();
    mWetControl = new PlayerWetControl(this, mModelHolder->findModelActor("Normal"), mAreaChecker);
    mWetControl->setEffect(mEffect);

    const IUsePlayerCollision* collision = getPlayerCollision();
    const IUsePlayerHeightCheck* heightCheck = mCollider;
    const IUsePlayerFallDistanceCheck* fallDistanceCheck = mCollider;
    const IUsePlayerCeilingCheck* ceilingCheck = mCollider;

    mHackCapJudgePreInputSeparateThrow =
        new HackCapJudgePreInputSeparateThrow(mInput, mSeparateCapFlag);
    mHackCapJudgePreInputSeparateJump =
        new HackCapJudgePreInputSeparateJump(mInput, mSeparateCapFlag);
    const char* hackCapTypeName = rs::getInitCapTypeName(playerInitInfo);
    mHackCap = new HackCap(this, hackCapTypeName, mInput, mAreaChecker, mWallActionHistory,
                           mCapActionHistory, mEyeSensorHitHolder, mSeparateCapFlag, collision,
                           heightCheck, mWetControl, mJointControlKeeper,
                           mHackCapJudgePreInputSeparateThrow, mHackCapJudgePreInputSeparateJump);
    mHackCap->init(actorInitInfo);
    al::registerExecutorFunctor(
        "帽子装着位置更新", actorInitInfo.executeDirector,
        PlayerActorHakoniwaFunctor(this, &PlayerActorHakoniwa::executeAfterCapTarget));
    PlayerCounterAfterCapCatch* counterAfterCapCatch = mCapActionHistory->getCounterAfterCapCatch();

    mRippleGenerator = new PlayerRippleGenerator(this, mHackCap, mModelHolder);
    mRecoverySafetyPoint =
        new PlayerRecoverySafetyPoint(this, mHackCap, actorInitInfo, dimension,
                                      mCollider->getCollisionFilter2D(), mBodyHitSensor);
    mPuppet = new PlayerPuppet(this, mHackCap, mAnimator, mCollider, mDimensionKeeper,
                               mModelChanger, mWorldEndBorderKeeper, mCounterForceRun,
                               mDamageKeeper, mEffect, mInput, mConst);
    mBindKeeper = new PlayerBindKeeper(mBodyHitSensor, mPuppet);
    mCarryKeeper = new PlayerCarryKeeper(this, mBodyHitSensor, mAnimator, mModelChanger, mCollider,
                                         mJointControlKeeper->getHandLegAngleParam());
    mEquipmentUser = new PlayerEquipmentUser(mInput, mModelHolder, mModelChanger);
    mSeCtrl = new PlayerSeCtrl(this, mAnimator, mHackCap, mModelChanger,
                               mModelHolder->getCurrentModelActor(), mExternalVelocity);
    mHackKeeper = new PlayerHackKeeper(this, mHackCap, mRecoverySafetyPoint, mInput, getViewMtx(),
                                       mDamageKeeper, mModelChanger, mCollider);
    mHackKeeper->createHackModel(actorInitInfo);
    mFormSensorCollisionArranger =
        new PlayerFormSensorCollisionArranger(this, mCollider, mModelChanger, mHackKeeper);

    PlayerModelHolder* modelHolderForEyes = mModelHolder;
    al::LiveActor* head = al::getSubActor(modelHolderForEyes->findModelActor("Normal"), "頭");
    al::LiveActor* capEye = al::tryGetSubActor(head, "キャップの目");
    al::LiveActor* capEye2D =
        al::getSubActor(modelHolderForEyes->findModelActor("Normal2D"), "キャップの目[2D]");
    mCapManHeroEyesControl =
        new PlayerCapManHeroEyesControl("キャップの目の制御", capEye, capEye2D);

    mJudgeAirForceCount = new PlayerJudgeAirForceCount(this, mExternalVelocity, collision);
    mJudgeCapCatchPop = new PlayerJudgeCapCatchPop(this, mInput, collision, mModelChanger,
                                                   mCapActionHistory, counterAfterCapCatch);
    mJudgeDeadWipeStart = new PlayerJudgeDeadWipeStart(this, mConst);
    mJudgeDirectRolling = new PlayerJudgeDirectRolling(
        this, mConst, mInput, collision, mModelChanger, mCarryKeeper, mCounterForceRun);
    mJudgeEnableStandUp = new PlayerJudgeEnableStandUp(collision, mCollider);
    mJudgeForceSlopeSlide = new PlayerJudgeForceSlopeSlide(this, mConst, collision);
    mJudgeForceRolling = new PlayerJudgeForceRolling(this, collision);
    mJudgeGrabCeil = new PlayerJudgeGrabCeil(this, mConst, collision, mModelChanger, mCarryKeeper,
                                             mExternalVelocity);
    mJudgeInWater[0] =
        new PlayerJudgeInWater(this, mConst, collision, mAreaChecker, mWaterSurfaceFinder,
                               heightCheck, mCounterForceRun, false, false, false);
    mJudgeInWater[1] =
        new PlayerJudgeInWater(this, mConst, collision, mAreaChecker, mWaterSurfaceFinder,
                               heightCheck, mCounterForceRun, false, false, true);
    mJudgeInWater[2] =
        new PlayerJudgeInWater(this, mConst, collision, mAreaChecker, mWaterSurfaceFinder,
                               heightCheck, nullptr, false, false, false);
    mJudgeInWater[3] =
        new PlayerJudgeInWater(this, mConst, collision, mAreaChecker, mWaterSurfaceFinder,
                               heightCheck, mCounterForceRun, false, true, false);
    mJudgeInvalidateInputFall = new PlayerJudgeInvalidateInputFall(this, mAreaChecker, collision);
    mJudgeLongFall = new PlayerJudgeLongFall(this, mConst, fallDistanceCheck, mHackKeeper,
                                             mBindKeeper, mModelChanger);
    mJudgePoleClimb = new PlayerJudgePoleClimb(this, mConst, collision, mModelChanger, mCarryKeeper,
                                               mExternalVelocity, mInput, mTrigger);
    mJudgePreInputCapThrow =
        new PlayerJudgePreInputCapThrow(mConst, mInput, mCarryKeeper, mHackCap);
    mJudgePreInputHackAction = new PlayerJudgePreInputHackAction(mConst, mInput);
    mHackCapJudgePreInputHoveringJump =
        new HackCapJudgePreInputHoveringJump(this, collision, mHackCap, mInput);
    mJudgeSandSink = new PlayerJudgeSandSink(collision, mSandSinkAffect);
    mJudgeSlopeSlide = new PlayerJudgeSlopeSlide(this, mConst, mInput, collision, mModelChanger);
    mJudgeStartHipDrop = new PlayerJudgeStartHipDrop(mConst, mInput, mCollider, mModelChanger);
    mJudgeStartRise = new PlayerJudgeStartRise(this, mAreaChecker, mModelChanger);
    mJudgeStartRolling =
        new PlayerJudgeStartRolling(this, mConst, mInput, collision, mModelChanger, mCarryKeeper);
    mJudgeStartSquat = new PlayerJudgeStartSquat(mInput, mCounterForceRun, mCarryKeeper);
    mJudgeWallCatch =
        new PlayerJudgeWallCatch(this, mConst, collision, mModelChanger, mCarryKeeper,
                                 mExternalVelocity, mInput, mTrigger, mCounterForceRun);
    mJudgeWallCatchInputDir = new PlayerJudgeWallCatchInputDir(
        this, mConst, mInput, collision, mModelChanger, mCarryKeeper, mExternalVelocity,
        mCounterForceRun, heightCheck);
    mJudgeWallHitDown = new PlayerJudgeWallHitDown(this, collision, mConst, mTrigger);
    mJudgeWallHitDownForceRun =
        new PlayerJudgeWallHitDownForceRun(this, collision, mConst, mCounterForceRun, mTrigger);
    mJudgeWallHitDownRolling = new PlayerJudgeWallHitDownRolling(this, collision, mConst, mTrigger);
    mJudgeWallKeep = new PlayerJudgeWallKeep(
        this, mConst, mInput, collision, mModelChanger, heightCheck, mCounterAfterUpperPunch,
        mWallActionHistory, mCarryKeeper, mTrigger, mCounterForceRun);
    mJudgeCameraSubjective = new PlayerJudgeCameraSubjective(this, collision, mInput);

    mSpinCapAttack = new PlayerSpinCapAttack(mHackCap, mConst, mTrigger, mInput,
                                             counterAfterCapCatch, mJudgePreInputCapThrow);
    mActionDiveInWater = new PlayerActionDiveInWater(
        mAnimator, mModelChanger, mCarryKeeper,
        new PlayerJudgeDiveInWater(this, mConst, heightCheck, fallDistanceCheck));
    mJudgeForceLand = new PlayerJudgeForceLand(mJudgeLongFall, mTrigger);
    mJudgeOutInWater = new PlayerJudgeOutInWater(this, mJudgeInWater[0], mJudgeInWater[3]);
    mJudgePreInputJump = new PlayerJudgePreInputJump(mConst, mInput, mJudgeForceSlopeSlide);
    mPuppet->setJudgePreInputJump(mJudgePreInputJump);
    PlayerHackKeeper* hackKeeperForJudge = mHackKeeper;
    hackKeeperForJudge->mJudgePreInputJump = mJudgePreInputJump;
    hackKeeperForJudge = mHackKeeper;
    hackKeeperForJudge->mJudgePreInputHackAction = mJudgePreInputHackAction;
    mJudgeStartWaterSurfaceRun =
        new PlayerJudgeStartWaterSurfaceRun(this, mWaterSurfaceFinder, mCounterForceRun);
    mJudgeSpeedCheckFall =
        new PlayerJudgeSpeedCheckFall(this, collision, mConst, mJudgeStartWaterSurfaceRun);
    mJudgeStartRun =
        new PlayerJudgeStartRun(this, mConst, collision, mInput, mCounterForceRun, mJudgeForceLand);

    al::initNerve(this, &NrvPlayerActorHakoniwa.Fall, 29);
    mStateWait = new PlayerStateWait(
        this, mConst, collision, mModelChanger, mJointControlKeeper, mWaterSurfaceFinder,
        heightCheck, mJudgeForceLand, mAnimator, mTrigger, mCapManHeroEyesControl,
        mJointControlKeeper->getCenterDynamicsParam(), mJointControlKeeper->getGroundPoseParam());
    mStateSandSink = new PlayerStateSandSink(this, mConst, mInput, mTrigger, mCollider, mAnimator,
                                             mJudgeStartRun, mJudgePreInputJump);
    mActorStateSandGeyser = new ActorStateSandGeyser(this);
    mStateSpinCap = new PlayerStateSpinCap(this, mConst, mInput, mCounterForceRun,
                                           mWaterSurfaceFinder, mCollider, mTrigger, mSpinCapAttack,
                                           mAnimator, mJointControlKeeper->getCapThrowParam());
    mStateSquat =
        new PlayerStateSquat(this, mConst, mAnimator, mInput, collision, mModelChanger, mTrigger,
                             mJudgeStartSquat, mJudgeStartRun, mJudgeEnableStandUp);
    mStateRunHakoniwa2D3D = new PlayerStateRunHakoniwa2D3D(
        this, mConst, this, mInput, collision, mWaterSurfaceFinder, mCounterForceRun,
        mCounterQuickTurnJump, mTrigger, mAnimator, mEffect,
        mJointControlKeeper->getCenterDynamicsParam(), isMoon);
    mStateSlope = new PlayerStateSlope(this, mConst, mInput, collision, mAnimator);
    mStateRolling = new PlayerStateRolling(this, mConst, mInput, collision, mTrigger, mAnimator,
                                           mEffect, mJudgeStartRolling, mJudgeEnableStandUp,
                                           mJudgePreInputJump, mJudgePreInputCapThrow,
                                           mJudgeForceRolling, mContinuousLongJump, mSeCtrl);
    mStateJump = new PlayerStateJump(
        this, mConst, mModelChanger, dimension, mInput, mJumpMessageRequest,
        new PlayerJudgeCapCatch(this, counterAfterCapCatch), mCounterForceRun, mJointControlKeeper,
        mCollider, mTrigger, mContinuousJump, mAnimator, mActionDiveInWater, mHackCap,
        new HackCapJudgeHoldHoveringJump(this, mCollider, mHackCap, mInput, mConst), isMoon);
    mStateCapCatchPop =
        new PlayerStateCapCatchPop(this, mConst, collision, mInput, mAnimator, mHackCap);
    mStateWallAir = new PlayerStateWallAir(this, mConst, mInput, mTrigger, mCollider,
                                           mJudgeWallKeep, mJudgePreInputJump, mAnimator,
                                           mWallActionHistory, mActionDiveInWater);
    mStateWallCatch = new PlayerStateWallCatch(
        this, mConst, mInput, getPlayerCollision(), mCollider, mModelHolder, mAreaChecker,
        mAnimator, mTrigger, mJudgePreInputJump, mJointControlKeeper->getGrabParam());
    mStateGrabCeil = new PlayerStateGrabCeil(this, mConst, mInput, mModelHolder, mTrigger,
                                             getPlayerCollision(), mAnimator, mJudgePreInputJump,
                                             mBodyHitSensor, mJointControlKeeper->getGrabParam());
    mStatePoleClimb = new PlayerStatePoleClimb(this, mConst, mInput, mTrigger, mModelHolder,
                                               getPlayerCollision(), mAnimator, mWallActionHistory,
                                               mJointControlKeeper->getHandLegAngleParam(),
                                               mJudgePreInputJump, mActionDiveInWater);
    mStateHipDrop = new PlayerStateHipDrop(this, mConst, mInput, mCollider, mAnimator, mTrigger);
    mStateHeadSliding =
        new PlayerStateHeadSliding(this, mConst, collision, mInput, mActionDiveInWater, mAnimator);
    mStateLongJump = new PlayerStateLongJump(this, mConst, mInput, collision, mTrigger, mAnimator,
                                             mContinuousLongJump, mActionDiveInWater);
    mStateFallHakoniwa = new PlayerStateFallHakoniwa(this, mConst, collision, mInput, mTrigger,
                                                     mAreaChecker, this, mAnimator);
    mStateRise = new PlayerStateRise(this, mConst, collision, mInput, mAnimator);
    mStateSwim =
        new PlayerStateSwim(this, mConst, collision, mInput, mTrigger, mCarryKeeper, mModelHolder,
                            mExternalVelocity, mAnimator, mSpinCapAttack, mWaterSurfaceFinder,
                            mEffect, mJointControlKeeper->getSwimParam());
    mStateDamageLife = new PlayerStateDamageLife(this, mConst, collision, mInput, ceilingCheck,
                                                 mAnimator, mTrigger);
    mStateDamageSwim =
        new PlayerStateDamageSwim(this, mConst, collision, mInput, mTrigger, mAnimator,
                                  mWaterSurfaceFinder, mEffect, mJudgeInWater[0], mJudgeOutInWater);
    mStateDamageFire =
        new PlayerStateDamageFire(this, mConst, collision, mInput, mAnimator, mJudgeSpeedCheckFall);
    mStatePress = new PlayerStatePress(this, collision, mAnimator);
    mStateBind = new PlayerStateBind(this, mConst, mInput, mPuppet, collision, mTrigger);
    mStateHack = new PlayerStateHack(this, mHackKeeper, mModelChanger, mAnimator, mHackCap);
    mStateEndHack = new PlayerStateEndHack(this, mConst, collision, mInput, mTrigger, mAnimator);
    mStateCameraSubjective =
        new PlayerStateCameraSubjective(this, mJudgeCameraSubjective->getTicket(), mModelChanger,
                                        mCarryKeeper, mAnimator, mConst, collision, mInput);
    mStateAbyss = new PlayerStateAbyss(this, mConst, mRecoverySafetyPoint, mCollider, mAnimator,
                                       mModelHolder->findModelActor("Normal2D"));

    al::initNerveState(this, mStateWait, &NrvPlayerActorHakoniwa.Wait, "Wait");
    al::initNerveState(this, mStateSquat, &NrvPlayerActorHakoniwa.Squat, "しゃがみ");
    al::initNerveState(this, mStateRunHakoniwa2D3D, &NrvPlayerActorHakoniwa.Run, "Run");
    al::initNerveState(this, mStateSlope, &NrvPlayerActorHakoniwa.Slope, "Slope");
    al::initNerveState(this, mStateRolling, &NrvPlayerActorHakoniwa.Rolling, "Rolling");
    al::initNerveState(this, mStateSpinCap, &NrvPlayerActorHakoniwa.SpinCap, "SpinCap");
    al::initNerveState(this, mStateJump, &NrvPlayerActorHakoniwa.Jump, "Jump");
    al::initNerveState(this, mStateCapCatchPop, &NrvPlayerActorHakoniwa.CapCatchPop, "帽子キャッチ浮きあがり");
    al::initNerveState(this, mStateWallAir, &NrvPlayerActorHakoniwa.WallAir, "壁接触[空中]");
    const al::Nerve* wallCatchNerve = &NrvPlayerActorHakoniwa.WallCatch;
    al::initNerveState(this, mStateWallCatch, wallCatchNerve, "WallCatch");
    const al::Nerve* grabCeilNerve = &NrvPlayerActorHakoniwa.GrabCeil;
    al::initNerveState(this, mStateGrabCeil, grabCeilNerve, "GrabCeil");
    const al::Nerve* poleClimbNerve = &NrvPlayerActorHakoniwa.PoleClimb;
    al::initNerveState(this, mStatePoleClimb, poleClimbNerve, "PoleClimb");
    al::initNerveState(this, mStateHipDrop, &NrvPlayerActorHakoniwa.HipDrop, "HipDrop");
    al::initNerveState(this, mStateHeadSliding, &NrvPlayerActorHakoniwa.HeadSliding, "HeadSliding");
    al::initNerveState(this, mStateLongJump, &NrvPlayerActorHakoniwa.LongJump, "幅跳び");
    al::initNerveState(this, mStateFallHakoniwa, &NrvPlayerActorHakoniwa.Fall, "Fall");
    al::initNerveState(this, mStateSandSink, &NrvPlayerActorHakoniwa.SandSink, "SandSink");
    al::initNerveState(this, mActorStateSandGeyser, &NrvPlayerActorHakoniwa.SandGeyser,
                       "SandGeyser");
    al::initNerveState(this, mStateRise, &NrvPlayerActorHakoniwa.Rise, "Rise");
    const al::Nerve* swimNerve = &NrvPlayerActorHakoniwa.Swim;
    al::initNerveState(this, mStateSwim, swimNerve, "Swim");
    al::initNerveState(this, mStateDamageLife, &NrvPlayerActorHakoniwa.Damage, "Damage");
    al::initNerveState(this, mStateDamageSwim, &NrvPlayerActorHakoniwa.DamageSwim, "水中・水面ダメージ");
    al::initNerveState(this, mStateDamageFire, &NrvPlayerActorHakoniwa.DamageFire, "DamageFire");
    al::initNerveState(this, mStatePress, &NrvPlayerActorHakoniwa.Press, "Press");
    al::initNerveState(this, mStateHack, &NrvPlayerActorHakoniwa.Hack, "Hack");
    al::initNerveState(this, mStateEndHack, &NrvPlayerActorHakoniwa.EndHack, "憑依解除");
    al::initNerveState(this, mStateBind, &NrvPlayerActorHakoniwa.Bind, "Bind");
    al::initNerveState(this, mStateCameraSubjective, &NrvPlayerActorHakoniwa.Camera, "Camera");
    al::initNerveState(this, mStateAbyss, &NrvPlayerActorHakoniwa.Abyss, "奈落死");
    mEquipmentUser->setStateRolling(mStateRolling);

    mOxygen = new PlayerOxygen();
    mOxygen->setup(mConst->getOxygenNoReduceFrame(), mConst->getOxygenReduceFrame(),
                   mConst->getOxygenRecoveryFrame(), mConst->getOxygenDamageInterval());
    mOxygen->reset();
    auto* iceWaterJudge =
        new PlayerJudgeInWater(this, mConst, collision, mAreaChecker, mWaterSurfaceFinder,
                               heightCheck, mCounterForceRun, true, false, false);
    mCounterIceWater =
        new PlayerCounterIceWater(this, actorInitInfo, mConst, mCollider, iceWaterJudge);
    mStainControl = new PlayerStainControl(this, mModelHolder->findModelActor("Normal"),
                                           mModelChanger, mHackCap, mEffect);
    mJudgeRecoveryLifeFast =
        new PlayerJudgeRecoveryLifeFast(this, mDamageKeeper, mBindKeeper, collision, mInput,
                                        mHackKeeper, mStateWait, mStateSwim, iceWaterJudge);

    PlayerInfo* playerInfo = new PlayerInfo();
    mInfo = playerInfo;
    mInfo->mModelChangerHakoniwa = mModelChanger;
    mInfo->mOxygen = mOxygen;
    playerInfo->mBindKeeper = mBindKeeper;
    PlayerInfo* infoForField = mInfo;
    infoForField->mDamageKeeper = mDamageKeeper;
    infoForField = mInfo;
    infoForField->mDemoActionFlag = mDemoActionFlag;
    infoForField = mInfo;
    infoForField->mEquipmentUser = mEquipmentUser;
    infoForField = mInfo;
    infoForField->mHackCap = mHackCap;
    infoForField = mInfo;
    infoForField->mWorldEndBorderKeeper = mWorldEndBorderKeeper;
    infoForField = mInfo;
    infoForField->mFormSensorCollisionArranger = mFormSensorCollisionArranger;
    infoForField = mInfo;
    infoForField->mAnimator = mAnimator;
    infoForField = mInfo;
    infoForField->mInput = mInput;
    infoForField = mInfo;
    infoForField->mModelHolder = mModelHolder;
    infoForField = mInfo;
    infoForField->mCeilingCheck = mCollider;
    infoForField = mInfo;
    infoForField->mHackKeeper = mHackKeeper;
    infoForField = mInfo;
    infoForField->mCapManHeroEyesControl = mCapManHeroEyesControl;
    infoForField = mInfo;
    infoForField->mCarryKeeper = mCarryKeeper;
    infoForField = mInfo;
    infoForField->mJointControlKeeper = mJointControlKeeper;
    infoForField = mInfo;
    infoForField->mCounterIceWater = mCounterIceWater;
    infoForField = mInfo;
    infoForField->mStainControl = mStainControl;
    infoForField = mInfo;
    infoForField->mFootPrintHolder = mFootPrintHolder;
    infoForField = mInfo;
    infoForField->mBodyHitSensor = mBodyHitSensor;
    infoForField = mInfo;
    infoForField->mRecoverySafetyPoint = mRecoverySafetyPoint;
    infoForField = mInfo;
    infoForField->mCostumeInfo = costume;
    PlayerInfo* judgeInfo = mInfo;
    judgeInfo->mJudgeCameraInWater =
        new PlayerJudgeCameraInWater(this, mJudgeInWater[0], mWaterSurfaceFinder);
    judgeInfo = mInfo;
    judgeInfo->mJudgeActiveCameraSubjective =
        new PlayerJudgeActiveCameraSubjective(mStateCameraSubjective);
    mInfo->mJudgeTalkGround = new PlayerJudgeTalkGround(
        this, mModelChanger, mHackKeeper, mCarryKeeper, collision, mInput, mConst, mStateWait);
    mInfo->mJudgeTalkSwim = new PlayerJudgeTalkSwim(mHackKeeper, mCarryKeeper, mStateSwim);
    mInfo->mJudgeDead = new PlayerJudgeDead(this);
    PlayerInfo* deadWipeInfo = mInfo;
    deadWipeInfo->mJudgeDeadWipeStart = mJudgeDeadWipeStart;
    judgeInfo = mInfo;
    judgeInfo->mJudgeDrawForward = new PlayerJudgeDrawForward(this, mJudgeDeadWipeStart);
    judgeInfo = mInfo;
    judgeInfo->mJudgeNrvPoleClimb = new PlayerJudgeSameNerve(this, poleClimbNerve);
    judgeInfo = mInfo;
    judgeInfo->mJudgeNrvGrabCeil = new PlayerJudgeSameNerve(this, grabCeilNerve);
    judgeInfo = mInfo;
    judgeInfo->mJudgeNrvWallCatch = new PlayerJudgeSameNerve(this, wallCatchNerve);
    judgeInfo = mInfo;
    judgeInfo->mJudgeNrvSwim = new PlayerJudgeSameNerve(this, swimNerve);
    judgeInfo = mInfo;
    judgeInfo->mJudgeFailureCameraSubjective = new PlayerJudgeFailureCameraSubjective(
        mInput, new PlayerJudgeSameNerve(this, &NrvPlayerActorHakoniwa.Camera));
    judgeInfo = mInfo;
    judgeInfo->mJudgeSafetyPointRecovery = new PlayerJudgeSafetyPointRecovery(
        new PlayerJudgeSameNerve(this, &NrvPlayerActorHakoniwa.Abyss), mStateAbyss);
    judgeInfo = mInfo;
    judgeInfo->mJudgeStatusPoleClimb = new PlayerJudgeStatusPoleClimb(
        new PlayerJudgeSameNerve(this, poleClimbNerve), mStatePoleClimb);
    judgeInfo = mInfo;
    judgeInfo->mJudgePlaySwitchOnAreaWaitAnim = new PlayerJudgePlaySwitchOnAreaWaitAnim(mStateWait);
    judgeInfo = mInfo;
    judgeInfo->mJudgeSleep = new PlayerJudgeSleep(mStateWait);
    judgeInfo = mInfo;
    judgeInfo->mJudgeEnableGuideArrow = new PlayerJudgeEnableGuideArrow(mPuppet);
    judgeInfo = mInfo;
    judgeInfo->mJudgeEnablePeachAmiibo = new PlayerJudgeEnablePeachAmiibo(mStateDamageFire);
    PlayerInfo* moonInfo = mInfo;
    moonInfo->mIsMoon = isMoon;

    CapFunction::putOnCapPlayer(static_cast<HackCap*>(mHackCap), mAnimator);
    syncSensorAndCollision();
    makeActorAlive();
    rs::resetCollision(mCollider);
    mGaugeAir = new GaugeAir("酸素タイマー", al::getLayoutInitInfo(actorInitInfo));
    WaterSurfaceShadow* waterSurfaceShadow =
        new WaterSurfaceShadow(actorInitInfo, "WaterSurfaceShadow");
    mWaterSurfaceShadow = waterSurfaceShadow;
    waterSurfaceShadow->setAppearParam(1500.0f, 0.25f, 0.75f);
    waterSurfaceShadow->setScale(0.5f);
}

void PlayerActorHakoniwa::updateModelShadowDropLength() {
    PlayerColliderHakoniwa* collider = mCollider;
    const IUsePlayerHeightCheck* heightCheck = collider;
    PlayerModelHolder* modelHolder = mModelHolder;
    PlayerModelChangerHakoniwa* modelChanger = mModelChanger;
    PlayerAreaChecker* areaChecker = mAreaChecker;
    PlayerDemoActionFlag* demoActionFlag = mDemoActionFlag;
    const PlayerConst* playerConst = mConst;
    PlayerStateWallCatch* stateWallCatch = mStateWallCatch;
    const bool isLongShadow = mIsLongShadow;

    if (modelChanger->is2DModel() || al::isNerve(this, &NrvPlayerActorHakoniwa.Hack))
        return;

    const char* shadowName = PlayerFunction::getPlayerDepthGroundShadowName();
    al::LiveActor* modelActor = modelHolder->getCurrentModelActor();
    if (al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) && demoActionFlag->isShadowLengthFixed()) {
        al::setEnableDepthShadowMapBottomGradation(modelActor, shadowName, false);
        al::setDepthShadowMapLength(modelActor, demoActionFlag->getShadowLength(), shadowName);
        al::setDepthShadowMapLengthFromActorTransFlag(modelActor, false, shadowName);
        return;
    }

    f32 shadowLength = 0.0f;
    s32 isBottomGradation = 0;
    if (!areaChecker->isInShadowLength(&shadowLength, al::getTrans(this))) {
        const f32 maxLength = isLongShadow ? playerConst->getShadowDropLengthExtend() :
                                             playerConst->getShadowDropLengthMax();
        shadowLength = maxLength;
        isBottomGradation = al::isNearZero(heightCheck->getGroundHeight()) & 1;

        if (heightCheck->isAboveGround()) {
            if (al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch) &&
                stateWallCatch->isWallCatchForm()) {
                isBottomGradation = false;
            } else {
                const sead::Vector3f& gravity = al::getGravity(this);
                const f32 gx = gravity.x;
                const f32 gy = gravity.y;
                const f32 gz = gravity.z;
                const f32 minLength = playerConst->getShadowDropLengthMin();
                const f32 dropHeight = heightCheck->getShadowDropHeight();
                const f32 heightTerm = dropHeight * playerConst->getShadowDropHeightScale();
                const sead::Vector3f& groundNormal = collider->getGroundNormal();
                const f32 normalDot =
                    (-gy * groundNormal.y - gx * groundNormal.x) - gz * groundNormal.z;
                const f32 normalTerm = sqrtf(sead::Mathf::max(0.0f, 1.0f - normalDot * normalDot)) *
                                       playerConst->getShadowDropNormalAdd();
                const f32 calculated = minLength + (heightTerm + normalTerm);
                shadowLength = calculated;
                f32 clampedLength;
                if (calculated < 1.0f) {
                    clampedLength = 1.0f;
                } else {
                    clampedLength = calculated;
                    if (calculated > maxLength)
                        clampedLength = maxLength;
                }
                shadowLength = clampedLength;
            }
        }
    }

    const bool isBottomGradationEnabled =
        al::isEnableDepthShadowMapBottomGradation(modelActor, shadowName);
    const f32 targetGradationLength = sead::Mathf::max(0.0f, shadowLength - 50.0f);
    f32 currentGradationLength;
    bool useTargetGradationLength = false;
    if (isBottomGradation) {
        currentGradationLength = targetGradationLength;
        useTargetGradationLength = !isBottomGradationEnabled;
    }
    if (!useTargetGradationLength) {
        const f32 previousGradationLength =
            al::getDepthShadowMapBottomGradationLength(modelActor, shadowName);
        currentGradationLength = previousGradationLength < targetGradationLength ?
                                     previousGradationLength :
                                     targetGradationLength;
    }

    const f32 gradationLength = al::lerpValue(
        currentGradationLength, isBottomGradation ? targetGradationLength : 0.0f, 0.5f);
    const bool isGradationZero = al::isNearZero(gradationLength);
    al::setDepthShadowMapLength(modelActor, shadowLength, shadowName);
    al::setEnableDepthShadowMapBottomGradation(modelActor, shadowName, !isGradationZero);
    al::setDepthShadowMapBottomGradationLength(modelActor, shadowName, gradationLength);
}

void PlayerActorHakoniwa::executeAfterCapTarget() {
    mHackCap->updateCapPose();
    mModelChanger->syncModelBoneVisibility();
}

void PlayerActorHakoniwa::syncSensorAndCollision() {
    if (mModelChanger->is2DModel())
        mFormSensorCollisionArranger->setFormModel2D();
    else
        mFormSensorCollisionArranger->setFormModel3D();

    if (al::isNerve(this, &NrvPlayerActorHakoniwa.Hack)) {
        mFormSensorCollisionArranger->setFormActionHack();
    } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Squat) ||
               al::isNerve(this, &NrvPlayerActorHakoniwa.Slope) ||
               (al::isNerve(this, &NrvPlayerActorHakoniwa.Rolling) && mStateRolling->isRolling()) ||
               (al::isNerve(this, &NrvPlayerActorHakoniwa.Jump) && mStateJump->isFormSquat2D()) ||
               (al::isNerve(this, &NrvPlayerActorHakoniwa.Damage) &&
                mStateDamageLife->isFormSquat())) {
        mFormSensorCollisionArranger->setFormActionSquat();
    } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch) &&
               mStateWallCatch->isWallCatchForm()) {
        PlayerFormSensorCollisionArranger* arranger = mFormSensorCollisionArranger;
        const sead::Vector3f front = mStateWallCatch->getWallCatchFront();
        arranger->setFormActionWallGrab(front);
    } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.GrabCeil) &&
               mStateGrabCeil->isFormGrabCeil()) {
        mFormSensorCollisionArranger->setFormActionGrabCeil(mStateGrabCeil->getGrabCeilFront());
    } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.PoleClimb) &&
               mStatePoleClimb->isFormPoleClimb()) {
        mFormSensorCollisionArranger->setFormActionPoleClimb(mStatePoleClimb->getPoleFront());
    } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Swim)) {
        mFormSensorCollisionArranger->setFormActionSwim();
    } else if (rs::isJudge(mInfo->getJudgeSafetyPointRecovery())) {
        mFormSensorCollisionArranger->setFormActionRecovery();
    } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss)) {
        mFormSensorCollisionArranger->setFormActionAbyss(al::getGravity(this));
    } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Dead)) {
        mFormSensorCollisionArranger->setFormActionDead();
    } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Bind)) {
        mFormSensorCollisionArranger->setFormActionBind(mPuppet->isSensorValid());
    } else {
        mFormSensorCollisionArranger->setFormActionStandup();
    }

    if (mSpinCapAttack->isValidAttackSensor(mAnimator))
        mFormSensorCollisionArranger->setFormAttackSensorSpin();
    else
        mFormSensorCollisionArranger->setFormAttackSensorNone();
    mFormSensorCollisionArranger->update();

    if (mModelChanger->is2DModel() || al::isNerve(this, &NrvPlayerActorHakoniwa.Hack)) {
        if (mModelChanger->isChange()) {
            mSandSinkAffect->clear();
            mFormSensorCollisionArranger->setCollisionShapeOffsetGround(0.0f);
        }
    } else {
        if (mTrigger->isOn(PlayerTrigger::EPreMovementTrigger_val0)) {
            mSandSinkAffect->clear();
        } else {
            if (mSandSinkAffect->isSink())
                mSandSinkAffect->reduceVelocity(al::getVelocityPtr(this));
            PlayerSandSinkAffect* sandSinkAffect = mSandSinkAffect;
            PlayerStateSandSink* stateSandSink = mStateSandSink;
            sandSinkAffect->update(al::isNerve(this, &NrvPlayerActorHakoniwa.SandSink) &&
                                   !stateSandSink->isJump());
        }

        if (mSandSinkAffect->isSink())
            mFormSensorCollisionArranger->setCollisionShapeOffsetGround(
                mSandSinkAffect->getSinkVelocity());
        else if (mSandSinkAffect->isSafe())
            mFormSensorCollisionArranger->setCollisionShapeOffsetGround(0.0f);
    }

    if (mTrigger->isOnCollisionExpandCheck())
        rs::resetCollisionExpandCheck(mCollider);
}

void PlayerActorHakoniwa::initAfterPlacement() {
    mDimensionKeeper->update();
    if (!(rs::isIn2DArea(this) & 1))
        mDimensionKeeper->invalidate();

    mJointControlKeeper->resetPartsDynamics();
    mModelChanger->initStartModel();
    al::addVelocityToGravity(this, 0.01f);
    updateCollider();
    al::setVelocityZero(this);

    if (mHackKeeper->executeForceHackStageStart(mBodyHitSensor, this)) {
        mStateHack->prepareStageStartHack();
        mModelChanger->syncHost(true);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Hack);
        return;
    }

    if (!al::isNerve(this, &NrvPlayerActorHakoniwa.Fall))
        return;

    PlayerTrigger* trigger;
    bool shouldWait;
    if (rs::isCollidedGround(mCollider)) {
        trigger = mTrigger;
        shouldWait = true;
    } else if (mCollider->isAboveGround()) {
        const f32 groundHeight = mCollider->getGroundHeight();
        trigger = mTrigger;
        shouldWait = groundHeight < 10.0f;
    } else {
        trigger = mTrigger;
        shouldWait = false;
    }

    if (shouldWait) {
        trigger->set(PlayerTrigger::ECollisionTrigger_val8);
        mStateWait->initSceneStartAnim();
        mModelChanger->resetPosition();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Wait);
        return;
    }

    trigger->set(PlayerTrigger::ECollisionTrigger_val7);
    mInput->setupSceneStartFall();
}

// NON_MATCHING: target is 0x1bc0 bytes and current is 0x1bbc. Recovering the target's branch-local
// PlayerCapManHeroEyesControl::mPuppetEye2D (+0x18) loads removes the extra getter call and preserves
// the target isHide-before-load order, leaving 300/300 direct calls. The remaining 4-byte mismatch is
// stack/block scheduling (target SafeString temporary SP+0x50 versus current SP+0xA0); next
// hypothesis is surrounding temporary order.
void PlayerActorHakoniwa::movement() {
    if (mHackCap->isNoPutOnHide()) {
        if (isEnableReactionCapCatch(this, mCarryKeeper, mModelChanger, mStateWait, mStateJump,
                                     mStateWallAir, mStateSwim)) {
            sead::Vector3f front = {0.0f, 0.0f, 0.0f};
            if (!rs::tryCalcPlayerModelHeadJointFront(&front, this))
                al::calcFrontDir(&front, this);
            const sead::Vector3f catchPos = mFormSensorCollisionArranger->getHeadPos() +
                                            front * mFormSensorCollisionArranger->getHeadRadius();
            HackCap* hackCap = mHackCap;
            PlayerAnimator* animator = mAnimator;
            IUsePlayerCollision* collider = mCollider;
            const PlayerConst* playerConst = mConst;
            if (animator->isSubAnimPlaying())
                animator->endSubAnim();
            const char* animName = "CatchCapJumpParts";
            if (al::isNerve(this, &NrvPlayerActorHakoniwa.Wait)) {
                animator->startSubAnim("CatchCap");
                animName = "CatchCap";
            } else if (rs::isOnGroundRunAngle(this, collider, playerConst)) {
                animName = "CatchCapParts";
            }
            animator->startUpperBodyAnimAndHeadVisKeep(animName);
            hackCap->startCatch(animName, true, catchPos);
        } else {
            CapFunction::putOnCapPlayer(static_cast<HackCap*>(mHackCap), mAnimator);
        }
    }

    mInput->update();
    rs::updateJudge(mJudgePreInputJump);
    rs::updateJudge(mJudgePreInputCapThrow);
    rs::updateJudge(mJudgePreInputHackAction);
    rs::updateJudge(mHackCapJudgePreInputSeparateThrow);
    rs::updateJudge(mHackCapJudgePreInputSeparateJump);
    rs::updateJudge(mHackCapJudgePreInputHoveringJump);

    if (rs::isPlayerDamageStopDemo(this)) {
        mModelChanger->updateDamageStopDemo();
        mAnimator->updateModelAlpha();
        PlayerModelChangerHakoniwa* modelChanger = mModelChanger;
        PlayerStateHack* stateHack = mStateHack;
        modelChanger->syncHostDamageStopDemo(!al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) ||
                                             stateHack->isEnableModelSyncShowHide());
        if (mHackKeeper->getHackSensor()) {
            mHackKeeper->sendSyncDamageVisibility();
            mHackCap->syncHackDamageVisibility(rs::isDamageVisibilityHide(this));
        }
        return;
    }

    mExternalVelocity->update();
    {
        PlayerEquipmentUser* equipmentUser = mEquipmentUser;
        PlayerCounterForceRun* counterForceRun = mCounterForceRun;
        s32 forceRunFrames = 0;
        f32 forceRunSpeed = 0.0f;
        if (PlayerEquipmentFunction::tryGetEquipmentForceDashInfo(&forceRunFrames, &forceRunSpeed,
                                                                  equipmentUser)) {
            if (counterForceRun->getCounter() <= 0)
                al::startHitReaction(this, "強制走り開始");
            counterForceRun->setupForceRun(forceRunFrames, forceRunSpeed);
        }
    }
    checkDamageFromCollision();
    mTrigger->clearPreMovementTrigger();
    executePreMovementNerveChange();

    PlayerHackKeeper* hackKeeper = mHackKeeper;
    if (!al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) || hackKeeper->isHackDemoStarted())
        mAnimator->updateAnimFrame();

    mTrigger->clearReceiveSensorTrigger();
    if (!al::isNerve(this, &NrvPlayerActorHakoniwa.SandGeyser))
        mActorStateSandGeyser->clearRequest();
    mTrigger->clearActionTrigger();
    mTrigger->clearAttackSensorTrigger();
    mEyeSensorHitHolder->clear();
    mHitPush->clearHitFlag();

    PlayerActorBase::movement();

    PlayerModelChangerHakoniwa* modelChanger = mModelChanger;
    PlayerStateHack* stateHack = mStateHack;
    const bool syncModel =
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) || stateHack->isEnableModelSyncShowHide();
    modelChanger->syncHost(syncModel);
    PlayerAnimator* animator = mAnimator;
    hackKeeper = mHackKeeper;
    PlayerPuppet* puppet = mPuppet;
    if (PlayerFunction::isPlayerDeadStatus(this) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) || hackKeeper->isHackDemoStarted()) {
        animator->resetModelAlpha();
    } else if (!al::isNerve(this, &NrvPlayerActorHakoniwa.Bind) || !puppet->isDemoPushDisabled()) {
        animator->updateModelAlpha();
    }
    PlayerPainPartsKeeper* painPartsKeeper = mPainPartsKeeper;
    painPartsKeeper->setModelAlphaMask(mAnimator->getModelAlpha());

    PlayerStainControl* stainControl = mStainControl;
    PlayerWetControl* wetControl = mWetControl;
    PlayerCounterIceWater* counterIceWater = mCounterIceWater;
    PlayerAreaChecker* areaChecker = mAreaChecker;
    PlayerStateWallCatch* stateWallCatch = mStateWallCatch;
    const PlayerConst* playerConstForStain = mConst;
    if (!al::isNerve(this, &NrvPlayerActorHakoniwa.Demo)) {
        sead::Vector3f stainPos = al::getTrans(this);
        if (stateWallCatch->isWallCatchForm())
            stainPos += al::getGravity(this) * playerConstForStain->getWallCatchStainAreaOffset();
        if (const al::AreaObj* stainArea = areaChecker->tryFindStainArea(stainPos)) {
            switch (stainArea->getPriority()) {
            case 1:
                stainControl->recordBlizzard();
                break;
            case 2:
                wetControl->recordForestWaterFall();
                break;
            case 3:
                wetControl->recordWaterSplash();
                break;
            case 4:
                stainControl->recordBlackSmoke();
                break;
            default:
                break;
            }
        }
        wetControl->update();
        if (counterIceWater->isInIceWater())
            stainControl->recordIceWater();
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.Swim) ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.DamageSwim))
            stainControl->recordInWater();
        else if (wetControl->isWet())
            stainControl->recordInWet();
        stainControl->update();
    }
    mTrigger->clearMaterialChangeTrigger();

    PlayerColliderHakoniwa* collider = mCollider;
    PlayerStatePoleClimb* statePoleClimb = mStatePoleClimb;
    PlayerModelHolder* modelHolder = mModelHolder;
    wetControl = mWetControl;
    PlayerTrigger* trigger = mTrigger;
    areaChecker = mAreaChecker;
    al::WaterSurfaceFinder* waterSurfaceFinder = mWaterSurfaceFinder;
    const PlayerConst* playerConstForMaterial = mConst;
    const IUsePlayerHeightCheck* playerHeightCheck =
        static_cast<const IUsePlayerHeightCheck*>(collider);
    const char* materialCode = nullptr;
    if (statePoleClimb->isAttachPole())
        materialCode = statePoleClimb->getMaterialCode();
    else if (rs::isCollidedGround(collider))
        materialCode = rs::getMaterialCodeGround(collider);
    else if (rs::isCollidedWall(collider))
        materialCode = rs::getMaterialCodeWall(collider);
    else if (al::isInFire(this))
        materialCode = "LavaWhite";
    else
        materialCode = "NoCollide";

    const bool isPuddle = rs::isInPuddleHeight(waterSurfaceFinder, playerHeightCheck,
                                               playerConstForMaterial->getSwimCenterOffset());
    const bool isWater = areaChecker->isInWater(al::getTrans(this)) && !isPuddle;
    const bool isWet = wetControl->isWet() && !isWater && !isPuddle;

    al::updateMaterialCodeWet(this, isWet);
    al::updateMaterialCodeWater(this, isWater);
    al::updateMaterialCodePuddle(this, isPuddle);
    al::setMaterialCode(this, materialCode);

    al::LiveActor* modelActor = modelHolder->getCurrentModelActor();
    al::updateMaterialCodeWet(modelActor, isWet);
    al::updateMaterialCodeWater(modelActor, isWater);
    al::updateMaterialCodePuddle(modelActor, isPuddle);
    al::setMaterialCode(modelActor, materialCode);
    if (isPuddle || al::isEqualString(materialCode, "Puddle"))
        trigger->set(PlayerTrigger::EMaterialChangeTrigger_val0);

    sead::Vector3f lookAtTargetPosition;
    bool hasLookAtTargetPosition;
    bool isLookAtEnabled;
    {
        {
            PlayerAnimator* lookAtAnimator = mAnimator;
            PlayerStateWait* stateWait = mStateWait;
            PlayerStateSwim* stateSwim = mStateSwim;
            PlayerStateRunHakoniwa2D3D* stateRun = mStateRunHakoniwa2D3D;
            PlayerPuppet* lookAtPuppet = mPuppet;
            PlayerDemoActionFlag* demoActionFlag = mDemoActionFlag;
            if (al::isNerve(this, &NrvPlayerActorHakoniwa.Bind) &&
                lookAtPuppet->isLookAtTargetPositionEnabled()) {
                lookAtTargetPosition = lookAtPuppet->getLookAtTargetPosition();
                hasLookAtTargetPosition = true;
                isLookAtEnabled = true;
            } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) &&
                       demoActionFlag->isLookAtTargetPositionEnabled()) {
                lookAtTargetPosition = demoActionFlag->getLookAtTargetPosition();
                hasLookAtTargetPosition = true;
                isLookAtEnabled = true;
            } else {
                lookAtTargetPosition = {0.0f, 0.0f, 0.0f};
                hasLookAtTargetPosition = false;
                if (!al::isPlayingEntranceCamera(this, 0) && !lookAtAnimator->isSubAnimPlaying()) {
                    if (al::isNerve(this, &NrvPlayerActorHakoniwa.Wait))
                        isLookAtEnabled = stateWait->isEnableLookAt();
                    else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Swim))
                        isLookAtEnabled = stateSwim->isEnableLookAt();
                    else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Run))
                        isLookAtEnabled = stateRun->isEnableLookAt();
                    else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Bind))
                        isLookAtEnabled = lookAtPuppet->isLookAtEnabled();
                    else
                        isLookAtEnabled = false;
                } else {
                    isLookAtEnabled = false;
                }
            }
        }

        PlayerJointControlKeeper* jointControlKeeper = mJointControlKeeper;
        PlayerEyeSensorHitHolder* eyeSensorHitHolder = mEyeSensorHitHolder;
        PlayerCarryKeeper* carryKeeper = mCarryKeeper;
        PlayerPainPartsKeeper* ikPainPartsKeeper = mPainPartsKeeper;
        PlayerStateRunHakoniwa2D3D* stateRun = mStateRunHakoniwa2D3D;
        PlayerStateHipDrop* stateHipDrop = mStateHipDrop;
        PlayerStateSpinCap* stateSpinCap = mStateSpinCap;
        PlayerStateWallCatch* ikStateWallCatch = mStateWallCatch;
        PlayerStateRolling* stateRolling = mStateRolling;

        f32 groundPoseRate = 0.0f;
        f32 bodyPoseRate = 0.0f;
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.Wait)) {
            groundPoseRate = mStateWait->getInverseKinematicsRate();
            bodyPoseRate = groundPoseRate;
        } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Run)) {
            groundPoseRate = stateRun->getInverseKinematicsRate();
        } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Rolling)) {
            groundPoseRate = stateRolling->getInverseKinematicsRate();
        } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Wait) ||
                   al::isNerve(this, &NrvPlayerActorHakoniwa.Squat) ||
                   al::isNerve(this, &NrvPlayerActorHakoniwa.Camera) ||
                   (al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) &&
                    mDemoActionFlag->isEnableIK()) ||
                   (al::isNerve(this, &NrvPlayerActorHakoniwa.HipDrop) &&
                    stateHipDrop->isEnableIK()) ||
                   (al::isNerve(this, &NrvPlayerActorHakoniwa.SpinCap) &&
                    stateSpinCap->isEnableIK()) ||
                   (al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch) &&
                    ikStateWallCatch->isEnableIK())) {
            groundPoseRate = 1.0f;
            bodyPoseRate = 1.0f;
        }

        f32 turnTiltRate = 0.0f;
        PlayerJointParamCenterDynamics* centerDynamics =
            jointControlKeeper->getCenterDynamicsParam();
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.Run) && !carryKeeper->isCarry()) {
            turnTiltRate = stateRun->getTurnTiltRate();
            centerDynamics->tiltRate = stateRun->getCenterTiltRate();
        } else {
            centerDynamics->tiltRate = 0.0f;
        }
        centerDynamics->tiltDirection = al::getVelocity(this);

        f32 capDynamicsRate = 1.0f;
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.Wait))
            capDynamicsRate = 0.0f;
        else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Run))
            capDynamicsRate = stateRun->getCapDynamicsRate();
        else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Bind))
            capDynamicsRate = 0.25f;

        f32 noseDynamicsRate = 1.0f;
        if (ikPainPartsKeeper->isInvalidNoseDynamics())
            noseDynamicsRate = 0.1f;
        else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Bind))
            noseDynamicsRate = 0.25f;

        jointControlKeeper->setInverseKinematicsRate(groundPoseRate, bodyPoseRate);
        jointControlKeeper->setTurnTiltRate(turnTiltRate);
        jointControlKeeper->setCapDynamicsRate(capDynamicsRate);
        jointControlKeeper->setNoseDynamicsRate(noseDynamicsRate);
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.GrabCeil))
            jointControlKeeper->setGrabCeilPose(true);

        if (hasLookAtTargetPosition) {
            jointControlKeeper->setLookAtTargetPosition(lookAtTargetPosition);
        } else {
            if (isLookAtEnabled) {
                if (al::HitSensor* targetSensor = eyeSensorHitHolder->getLookAtTargetSensor())
                    jointControlKeeper->updateLookAtTarget(targetSensor);
            }
            jointControlKeeper->setLookAtEnabled(isLookAtEnabled);
        }
        jointControlKeeper->update();
    }

    {
        PlayerJointControlKeeper* eyeJointControlKeeper = mJointControlKeeper;
        PlayerStateSwim* eyeStateSwim = mStateSwim;
        const f32 headTiltRate = al::isNerve(this, &NrvPlayerActorHakoniwa.Swim) ?
                                     eyeStateSwim->calcEyeHeadTiltRate() :
                                     eyeJointControlKeeper->getHeadTiltRate();
        PlayerAnimator* eyeAnimator = mAnimator;
        const PlayerConst* playerConst = mConst;
        al::HitSensor* lookAtSensor = eyeJointControlKeeper->getLookAtTargetSensor();
        bool shouldTrackEyes = false;
        bool isTiltEye = false;
        bool isTargetInSight = false;
        f32 eyeControlFrame = 0.0f;
        sead::Vector3f eyeTarget = {0.0f, 0.0f, 0.0f};
        sead::Vector3f targetDir;
        sead::Vector3f angleAxis;
        sead::Vector3f headPos = {0.0f, 0.0f, 0.0f};
        bool hasEyeTarget = false;
        if (hasLookAtTargetPosition && rs::tryCalcPlayerModelHeadJointPos(&headPos, this)) {
            eyeTarget = lookAtTargetPosition;
            hasEyeTarget = true;
        } else if (lookAtSensor && rs::tryCalcPlayerModelHeadJointPos(&headPos, this)) {
            hasEyeTarget = rs::tryCalcLookAtTargetInDistance(&eyeTarget, lookAtSensor, headPos,
                                                             playerConst->getLookAtEyeDistance());
        }

        if (hasEyeTarget) {
            sead::Vector3f headSide = {0.0f, 0.0f, 0.0f};
            sead::Vector3f headUp = {0.0f, 0.0f, 0.0f};
            if (rs::tryCalcPlayerModelHeadJointSide(&headSide, this) &&
                rs::tryCalcPlayerModelHeadJointUp(&headUp, this)) {
                const bool isEyeControlAnimActive = eyeAnimator->isEyeControlAnimActive();
                const f32 minH = isEyeControlAnimActive ?
                                     playerConst->getLookAtEyeAngleMinInSightH() :
                                     playerConst->getLookAtEyeAngleMinH();
                const f32 minV = isEyeControlAnimActive ?
                                     playerConst->getLookAtEyeAngleMinInSightV() :
                                     playerConst->getLookAtEyeAngleMinV();
                const sead::Vector2f headUpXY = {headUp.x, headUp.y};
                const f32 headUpZ = headUp.z;
                const sead::Vector3f eyeUp = -headUp;
                const f32 maxH = playerConst->getLookAtEyeAngleMaxH();
                const f32 maxV = playerConst->getLookAtEyeAngleMaxV();
                sead::Vector3f headFront = {
                    headSide.y * headUpZ - headSide.z * headUpXY.y,
                    headSide.z * headUpXY.x - headSide.x * headUpZ,
                    headSide.x * headUpXY.y - headSide.y * headUpXY.x,
                };
                if (al::tryNormalizeOrZero(&headFront)) {
                    targetDir = eyeTarget - headPos;
                    if (al::tryNormalizeOrZero(&targetDir) && headSide.dot(targetDir) >= 0.0f &&
                        al::isInAngleOnPlaneDegreeHV(targetDir, headFront, eyeUp, 0.0f, maxH, 0.0f,
                                                     maxV)) {
                        isTargetInSight = true;
                        if (al::isInAngleOnPlaneDegreeHV(targetDir, headFront, eyeUp, minH, maxH,
                                                         minV, maxV)) {
                            shouldTrackEyes = true;
                            angleAxis = -headSide;
                            eyeControlFrame = al::modf(al::calcAngleOnPlaneDegree(
                                                           headFront, targetDir, angleAxis) +
                                                           360.0f,
                                                       360.0f) +
                                              0.0f;
                        }
                    }
                }
            }
        }

        if (!isTargetInSight) {
            const f32 tiltRateAbs = sead::Mathf::abs(headTiltRate);
            const f32 tiltBorder = eyeAnimator->isEyeControlAnimTilt() ?
                                       playerConst->getTiltEyeBorderEnd() :
                                       playerConst->getTiltEyeBorderStart();
            if (tiltRateAbs >= tiltBorder) {
                sead::Vector3f headSide = {0.0f, 0.0f, 0.0f};
                sead::Vector3f headUp = {0.0f, 0.0f, 0.0f};
                if (rs::tryCalcPlayerModelHeadJointSide(&headSide, this) &&
                    rs::tryCalcPlayerModelHeadJointUp(&headUp, this)) {
                    const f32 scaledAngle =
                        playerConst->getTiltEyeAngleScale() *
                        al::calcAngleOnPlaneDegree(headUp, al::getGravity(this), headSide);
                    eyeControlFrame =
                        al::modf((scaledAngle > 0.0f ? -0.0f : 180.0f) - scaledAngle + 360.0f,
                                 360.0f) +
                        0.0f;
                    shouldTrackEyes = true;
                    isTiltEye = true;
                }
            } else if (eyeAnimator->isEyeControlAnimTilt()) {
                eyeAnimator->endEyeControlAnim(0);
            }
        }

        if (isLookAtEnabled && shouldTrackEyes) {
            if (!isTiltEye && eyeAnimator->isEyeControlAnimTilt())
                eyeAnimator->endEyeControlAnim(0);
            if (!eyeAnimator->isEyeControlAnimActive())
                eyeAnimator->startEyeControlAnim(isTiltEye);
        } else if (eyeAnimator->isEyeControlAnimActive()) {
            s32 keepFrame = playerConst->getLookAtEyeKeepFrame();
            if (al::isNerve(this, &NrvPlayerActorHakoniwa.Wait))
                keepFrame = isTargetInSight ? playerConst->getLookAtEyeKeepFrameInSight() :
                                              playerConst->getLookAtEyeKeepFrameWait();
            else if (isTargetInSight)
                keepFrame = 0;
            eyeAnimator->endEyeControlAnim(keepFrame);
        }
        if (!isLookAtEnabled)
            eyeAnimator->clearEndEyeControlAnimDelay();
        if (shouldTrackEyes)
            eyeAnimator->setEyeControlFrame(eyeControlFrame);
        eyeAnimator->updateEyeControlAnim();
    }

    if (!mBindKeeper->getBindSensor() && !al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Hack))
        mFootPrintHolder->update();

    PlayerColliderHakoniwa* recoveryCollider = mCollider;
    PlayerPuppet* recoveryPuppet = mPuppet;
    PlayerRecoverySafetyPoint* recoverySafetyPoint = mRecoverySafetyPoint;
    PlayerHackKeeper* recoveryHackKeeper = mHackKeeper;
    PlayerAreaChecker* recoveryAreaChecker = mAreaChecker;
    PlayerFormSensorCollisionArranger* recoveryFormArranger = mFormSensorCollisionArranger;
    PlayerStateWallAir* recoveryStateWallAir = mStateWallAir;
    al::HitSensor* recoveryBodySensor = mBodyHitSensor;
    recoverySafetyPoint->updateRecoveryBubble();
    if (!rs::isPlayerSafetyPointRecovery(this)) {
        recoverySafetyPoint->updateRecoveryAreaValidity();
        if (!recoverySafetyPoint->isActiveRecoveryArea()) {
            const al::AreaObj* recoveryArea = nullptr;
            if (recoveryAreaChecker->isInRecovery(&recoveryArea, al::getTrans(this)))
                recoverySafetyPoint->setRecoveryArea(recoveryArea);
        }

        if (!recoveryAreaChecker->isInRecoveryBan(al::getTrans(this)) &&
            recoveryCollider->isEnableStandUp()) {
            if (al::isNerve(this, &NrvPlayerActorHakoniwa.Demo)) {
                if (rs::isActiveDemoPlayerPuppetable(this) && !rs::isActiveDemoShineGet(this) &&
                    !rs::isActiveDemoShineDotGet(this)) {
                    recoverySafetyPoint->setSafetyPoint(al::getTrans(this), -al::getGravity(this),
                                                        nullptr);
                }
            } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Bind)) {
                bool requestSafetyPoint = false;
                bool skipIfValid = false;
                sead::Vector3f recoveryNormal = sead::Vector3f::zero;
                sead::Vector3f recoveryPos = sead::Vector3f::zero;
                const al::AreaObj* recoveryArea = nullptr;
                if (recoveryPuppet->tryUpdateRecoveryInfo(&requestSafetyPoint, &skipIfValid,
                                                          &recoveryNormal, &recoveryPos,
                                                          &recoveryArea)) {
                    if (requestSafetyPoint) {
                        recoverySafetyPoint->noticeRequestSafetyPoint(recoveryNormal, recoveryPos,
                                                                      recoveryArea);
                    } else {
                        recoverySafetyPoint->noticeDangerousPoint(recoveryNormal, skipIfValid);
                    }
                }
            } else if (!al::isNerve(this, &NrvPlayerActorHakoniwa.WallAir) ||
                       recoveryStateWallAir->isAir()) {
                sead::Vector3f safetyPos = sead::Vector3f::zero;
                sead::Vector3f safetyNormal = sead::Vector3f::zero;
                sead::Vector3f* defaultSafetyPos = nullptr;
                const al::AreaObj* safetyArea = nullptr;

                if (recoveryHackKeeper->getHackSensor()) {
                    if (!recoveryHackKeeper->isActiveHackStartDemo()) {
                        al::LiveActor* hackActor = recoveryHackKeeper->getHack();
                        const IUsePlayerCollision* hackCollision =
                            recoveryHackKeeper->isHackUsePlayerCollision() ?
                                recoveryHackKeeper->getPlayerCollision() :
                                nullptr;
                        if (rs::calcActorCollidedGroundSafetyPoint(
                                &safetyPos, &safetyNormal, &safetyArea, &defaultSafetyPos,
                                hackActor, recoveryBodySensor, hackCollision)) {
                            recoverySafetyPoint->noticeRequestSafetyPoint(safetyPos, safetyNormal,
                                                                          safetyArea);
                        } else {
                            sead::Vector3f dangerousPos = al::getTrans(hackActor);
                            bool skipIfValid = false;
                            rs::calcActorCollisionCodeNoSafetyPointPos(&dangerousPos, &skipIfValid,
                                                                       hackActor, hackCollision);
                            recoverySafetyPoint->noticeDangerousPoint(dangerousPos, skipIfValid);
                        }
                    }
                } else {
                    sead::Vector3f checkPos = al::getTrans(this);
                    if (rs::isCollidedGround(recoveryCollider))
                        checkPos = rs::getCollidedGroundPos(recoveryCollider);
                    const sead::Vector3f gravityUp = -al::getGravity(this);
                    if (rs::calcCollidedGroundSafetyPoint(
                            &safetyPos, &safetyNormal, &safetyArea, &defaultSafetyPos, this,
                            recoveryCollider, recoveryBodySensor, gravityUp, checkPos) &&
                        recoveryFormArranger->isEnableSafetyPointForm()) {
                        recoverySafetyPoint->setSafetyPoint(safetyPos, safetyNormal, safetyArea);
                        recoverySafetyPoint->setDefaultSafetyPos(defaultSafetyPos);
                    } else {
                        bool skipIfValid = false;
                        rs::calcActorCollisionCodeNoSafetyPointPos(&checkPos, &skipIfValid, this,
                                                                   recoveryCollider);
                        recoverySafetyPoint->noticeDangerousPoint(checkPos, skipIfValid);
                    }
                }
            }
        }
    }
    if (!rs::isPlayerSafetyPointRecovery(this))
        recoverySafetyPoint->checkInvalidateArea();

    mPushReceiver->clear();
    mCapActionHistory->update();
    mCounterAfterUpperPunch->update(mTrigger);
    mCounterForceRun->update();
    PlayerCounterIceWater* counterIceWaterAfterRecovery = mCounterIceWater;
    PlayerDamageKeeper* damageKeeperAfterRecovery = mDamageKeeper;
    if (al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.Hack)) {
        counterIceWaterAfterRecovery->clearIceWaterCount();
    } else {
        const bool isIceWater = al::isNerve(this, &NrvPlayerActorHakoniwa.Swim) ||
                                al::isNerve(this, &NrvPlayerActorHakoniwa.DamageSwim);
        counterIceWaterAfterRecovery->updateCount(isIceWater,
                                                  !damageKeeperAfterRecovery->isDamageInvalid());
    }
    mCounterQuickTurnJump->update();

    PlayerRippleGenerator* rippleGenerator = mRippleGenerator;
    PlayerColliderHakoniwa* rippleCollider = mCollider;
    const PlayerConst* rippleConst = mConst;
    PlayerCounterIceWater* rippleCounterIceWater = mCounterIceWater;
    PlayerStateWallCatch* rippleStateWallCatch = mStateWallCatch;
    PlayerStateSwim* rippleStateSwim = mStateSwim;
    PlayerStateJump* rippleStateJump = mStateJump;
    PlayerStateHipDrop* rippleStateHipDrop = mStateHipDrop;
    HackCap* rippleHackCap = mHackCap;
    if (al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) || rs::isPlayer2D(this)) {
        rippleGenerator->reset();
    } else {
        sead::Vector3f rippleOffset = sead::Vector3f::zero;
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch) &&
            rippleStateWallCatch->isWallCatchForm()) {
            sead::Vector3f front = sead::Vector3f::zero;
            al::calcFrontDir(&front, this);
            const f32 collisionRadius = rippleConst->getCollisionRadius();
            const sead::Vector3f frontOffset = front * collisionRadius;
            const sead::Vector3f& gravity = al::getGravity(this);
            const f32 tall = rippleConst->getTall();
            rippleOffset = gravity * tall * 0.5f - frontOffset;
        }
        rippleGenerator->setOffset(rippleOffset);

        const bool isSwim = al::isNerve(this, &NrvPlayerActorHakoniwa.Swim) &&
                            !rippleCounterIceWater->isInIceWater();
        const bool isSwimSurface = isSwim && !rippleStateSwim->isSpinCapSurface();
        const bool isHipDrop =
            rs::isJustLand(rippleCollider) && (rippleStateJump->isEnableTrampleByHipDropAttack() ||
                                               rippleStateHipDrop->isEnableHipDropAttack() ||
                                               rippleStateSwim->isEnableHipDropAttack());
        rippleGenerator->updateAndGenerate(isSwim, isSwimSurface, isHipDrop,
                                           rippleHackCap->isSeparateHipDropLand());
    }

    mWallActionHistory->update(mCollider);
    if (!al::isNerve(this, &NrvPlayerActorHakoniwa.Demo))
        mPainPartsKeeper->update();
    rs::updateJudge(mJudgeDeadWipeStart);

    const bool isGroundIdle = al::isNerve(this, &NrvPlayerActorHakoniwa.Run) ||
                              al::isNerve(this, &NrvPlayerActorHakoniwa.Wait);
    if (!al::isNerve(this, &NrvPlayerActorHakoniwa.Jump))
        mContinuousJump->update(isGroundIdle);
    if (!al::isNerve(this, &NrvPlayerActorHakoniwa.LongJump) && !mStateRolling->isRollingJump())
        mContinuousLongJump->update();
    mStateWait->tryClearIgnoreSwitchOnAreaAnim();

    GaugeAir* gaugeAir = mGaugeAir;
    if (PlayerFunction::isPlayerDeadStatus(this) || rs::isKidsMode(this) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.Camera) || al::isPlayingEntranceCamera(this, 0)) {
        if (gaugeAir->isAlive())
            gaugeAir->fastEnd();
    } else {
        if (mStateSwim->isReduceOxygen() || mStateDamageSwim->isReduceOxygen())
            mOxygen->reduce();
        else if (!PlayerFunction::isPlayerDeadStatus(this))
            mOxygen->recovery();
        gaugeAir->setRate(mOxygen->getRate());
        if (mOxygen->isReducing()) {
            if (!gaugeAir->isAlive())
                gaugeAir->start();
        } else if (mOxygen->getRate() >= 1.0f && gaugeAir->isWait()) {
            gaugeAir->endMax();
        }
    }

    const sead::Vector3f gaugeHeadPos = mFormSensorCollisionArranger->getHeadPos();
    sead::Vector3f cameraSide = {0.0f, 0.0f, 0.0f};
    sead::Vector3f cameraUp = {0.0f, 0.0f, 0.0f};
    al::calcCameraSideDir(&cameraSide, this, 0);
    al::calcCameraUpDir(&cameraUp, this, 0);
    const sead::Vector3f gaugeWorldPos = gaugeHeadPos + cameraSide * 30.0f + cameraUp * 25.0f;
    sead::Vector2f gaugeLayoutPos = sead::Vector2f::zero;
    al::calcLayoutPosFromWorldPos(&gaugeLayoutPos, this, gaugeWorldPos);
    al::setLocalTrans(gaugeAir, gaugeLayoutPos);

    mJudgeWallCatchInputDir->updateWallCatchEnviroment();
    if (!al::isNerve(this, &NrvPlayerActorHakoniwa.Hack)) {
        if (rs::isIn2DArea(this) && mModelChanger->is2DModel())
            alCameraFunction::validateCameraArea2D(this);
        else
            alCameraFunction::invalidateCameraArea2D(this);
    }

    mEffect->updateInvincibleEffect(mModelChanger, PlayerFunction::isPlayerDeadStatus(this));
    if (al::isNerve(this, &NrvPlayerActorHakoniwa.Hack))
        mHackCap->syncInvincibleEffect(mEffect->isInvincibleEffectActive() &&
                                       !mHackKeeper->isPuppetable2());
    mSeCtrl->update();
    updateSeparateCap(this, mSeparateCapFlag, mHackCap, mAnimator, mCollider, mModelChanger,
                      mCarryKeeper, mPuppet, mEquipmentUser, mConst);

    const PlayerConst* capEyesConst = mConst;
    HackCap* capEyesHackCap = mHackCap;
    PlayerCapManHeroEyesControl* capEyesControl = mCapManHeroEyesControl;
    PlayerSeparateCapFlag* capEyesSeparateFlag = mSeparateCapFlag;
    PlayerStateWait* capEyesStateWait = mStateWait;
    PlayerStateJump* capEyesStateJump = mStateJump;

    if (rs::isModeDiverOrJungleGymRom()) {
        mTrigger->clearDemoEndTrigger();
        return;
    }

    if (!capEyesControl->isDemo()) {
        if (rs::isSeparatePlay(this) && rs::isPlayer2D(this)) {
            const bool isCapHidden = capEyesHackCap->isHide();
            al::LiveActor* puppetEye = capEyesControl->mPuppetEye2D;
            if (isCapHidden) {
                if (capEyesStateJump->isHovering()) {
                    if (al::isDead(puppetEye)) {
                        puppetEye->appear();
                        al::startAction(puppetEye, "Appear");
                    } else {
                        al::tryStartActionIfNotPlaying(puppetEye, "Appear");
                    }
                } else if (al::isAlive(puppetEye)) {
                    al::tryStartActionIfNotPlaying(puppetEye, "Disappear");
                    if (al::isActionEnd(puppetEye))
                        puppetEye->kill();
                }
            } else if (al::isAlive(puppetEye)) {
                puppetEye->kill();
            }
        } else {
            al::LiveActor* puppetEye = capEyesControl->mPuppetEye2D;
            if (al::isAlive(puppetEye))
                puppetEye->kill();
        }

        const u32 capFlags = capEyesSeparateFlag->getRawFlags();
        const GameDataHolderAccessor accessor(this);
        if (((capFlags & 0xFF0000) != 0 || (capFlags & 0xFF) == 0 ||
             !capEyesSeparateFlag->isSeparateCapLocal() || capEyesSeparateFlag->isPuppetable()) &&
            GameDataFunction::isEnableCap(accessor) && capEyesHackCap->isPutOn() &&
            !rs::isPlayer2D(this)) {
            bool forceAppear = false;
            if ((capFlags & 0xFF0000) == 0 && capEyesSeparateFlag->isSeparateCap())
                forceAppear = !capEyesSeparateFlag->isSeparateCapLocal() ||
                              capEyesSeparateFlag->isPuppetable();

            const bool waitAppear =
                al::isNerve(this, &NrvPlayerActorHakoniwa.Wait) &&
                (al::isGreaterEqualStep(this, capEyesConst->getCapManHeroEyesWaitAppearFrame()) ||
                 capEyesStateWait->isAreaAnimCapManHeroEyes());
            const bool capMessage =
                rs::isActiveCapMessage(this) && !al::isNerve(this, &NrvPlayerActorHakoniwa.Demo);
            if (forceAppear) {
                if (capEyesControl->isDisappear())
                    capEyesControl->fastStart();
            } else if (waitAppear || capMessage) {
                if (capEyesControl->isDisappear())
                    capEyesControl->start();
            } else if (capEyesControl->isAppear()) {
                capEyesControl->end();
            }
        } else {
            if (capEyesControl->isDeadCap()) {
                mTrigger->clearDemoEndTrigger();
                return;
            }
            capEyesControl->kill();
        }
    }
    capEyesControl->update();
    mTrigger->clearDemoEndTrigger();
}

namespace {

bool isEnableReactionCapCatch(const al::LiveActor* player, const PlayerCarryKeeper* carryKeeper,
                              const PlayerModelChangerHakoniwa* modelChanger,
                              const PlayerStateWait* stateWait, const PlayerStateJump* stateJump,
                              const PlayerStateWallAir* stateWallAir,
                              const PlayerStateSwim* stateSwim) {
    if (carryKeeper->isCarry() || carryKeeper->isThrow() || modelChanger->is2DModel())
        return false;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.Wait))
        return stateWait->isEnableReactionCapCatch();
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.Run) ||
        al::isNerve(player, &NrvPlayerActorHakoniwa.SpinCap) ||
        al::isNerve(player, &NrvPlayerActorHakoniwa.HipDrop) ||
        al::isNerve(player, &NrvPlayerActorHakoniwa.LongJump) ||
        al::isNerve(player, &NrvPlayerActorHakoniwa.Fall) ||
        stateJump->isEnableReactionCapCatch() || stateWallAir->isEnableReactionCapCatch())
        return true;
    return stateSwim->isEnableReactionCapCatch();
}

}  // namespace

void PlayerActorHakoniwa::checkDamageFromCollision() {
    if (PlayerFunction::isPlayerDeadStatus(this) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss))
        return;

    if (rs::isTouchDeadCode(this, mCollider, mModelChanger, this, 7.0f)) {
        PlayerTrigger* trigger = mTrigger;
        if (rs::isCollisionCodePoisonTouch(mCollider))
            trigger->set(PlayerTrigger::ECollisionTrigger_val5);
        trigger->set(PlayerTrigger::ECollisionTrigger_val6);
        return;
    }

    if (rs::isTouchDamageFireCode(this, mCollider, mModelChanger)) {
        mTrigger->set(PlayerTrigger::ECollisionTrigger_val3);
        return;
    }

    if (rs::isTouchDamageCode(this, mCollider) && !mDamageKeeper->isDamageInvalid())
        mTrigger->set(PlayerTrigger::ECollisionTrigger_val2);
}

void PlayerActorHakoniwa::executePreMovementNerveChange() {
    PlayerHackKeeper* hackKeeper = mHackKeeper;
    bool shouldClearBindable =
        PlayerFunction::isPlayerDeadStatus(this) || al::isNerve(this, &NrvPlayerActorHakoniwa.Demo);
    if (!shouldClearBindable) {
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.Hack))
            shouldClearBindable = hackKeeper->getHackSensor();
        else
            shouldClearBindable = al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss);
    }

    PlayerBindKeeper* bindKeeper = mBindKeeper;
    if (shouldClearBindable) {
        bindKeeper->clearBindableSensor();
    } else if (bindKeeper->sendStartMsg()) {
        mSpinCapAttack->tryCancelCapState(mAnimator);
        if (mCarryKeeper->isCarry())
            mCarryKeeper->startCancelAndRelease();
        mInput->startBind();
        mAnimator->resetModelAlpha();
        mComboCounter->reset();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Bind);
        return;
    }

    if (!PlayerFunction::isPlayerDeadStatus(this) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Bind) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss) && rs::isPressedCollision(mCollider)) {
        mDamageKeeper->dead();
        mJudgeDeadWipeStart->setDeathType(DeathType::Press);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Press);
        return;
    }

    if (al::isNerve(this, &NrvPlayerActorHakoniwa.SandSink) &&
        mSandSinkAffect->isSinkDeathHeight() && !PlayerFunction::isPlayerDeadStatus(this)) {
        PlayerDamageKeeper* sandDamageKeeper = mDamageKeeper;
        PlayerConst* playerConst = mConst;
        if (!mRecoverySafetyPoint->isValid() || PlayerFunction::isPlayerHitPointOne(this)) {
            mDamageKeeper->dead();
            mJudgeDeadWipeStart->setDeathType(DeathType::SandSink);
            mStateSandSink->setSandSinkDead();
            return;
        }
        sandDamageKeeper->damageForce(playerConst->getDamageInvalidCountRecovery());
        al::startHitReaction(this, "泡復帰ダメージ");
        mSandSinkAffect->clear();
        startRecoveryFromDeathArea(this, mHackCap, mCarryKeeper, mBindKeeper, mEquipmentUser,
                                   mStateAbyss);
        return;
    }

    PlayerBindKeeper* demoBindKeeper = mBindKeeper;
    if (rs::isActiveDemo(this) || al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) ||
        PlayerFunction::isPlayerDeadStatus(this) || demoBindKeeper->getBindSensor()) {
        PlayerBindKeeper* returnBindKeeper = mBindKeeper;
        HackCap* hackCap = mHackCap;
        if (!rs::isActiveDemo(this) && !PlayerFunction::isPlayerDeadStatus(this) &&
            hackCap->isWaitHackLockOn() &&
            (returnBindKeeper->getBindSensor() || rs::isPlayerSafetyPointRecovery(this)))
            mHackCap->requestReturn(nullptr);
    } else if (mHackCap->sendMsgStartHack(mBodyHitSensor)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Hack);
        return;
    }

    PlayerHackKeeper* endHackKeeper = mHackKeeper;
    if (al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) && !endHackKeeper->getHackSensor()) {
        if (!mTrigger->isOnEndHackWithDamage() ||
            !processPlayerDamage(this, mDamageKeeper, mCarryKeeper, mTrigger, mConst,
                                 mRecoverySafetyPoint, mInput)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.EndHack);
            return;
        }
        if (mTrigger->isOn(PlayerTrigger::EReceiveSensorTrigger_val3))
            mTrigger->set(PlayerTrigger::EPreMovementTrigger_val2);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Damage);
        return;
    }

    if (mActorStateSandGeyser->isRequested() &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.SandGeyser)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SandGeyser);
        return;
    }

    if (mBindKeeper->getBindSensor()) {
        bool shouldProcessDamage = false;
        if (mBindKeeper->receiveRequestDamage() && !mDamageKeeper->isDamageInvalid())
            shouldProcessDamage = true;
        else if (mTrigger->isOnAnyDamage() && mBindKeeper->sendMsgBindDamage())
            shouldProcessDamage = true;

        if (shouldProcessDamage && processPlayerDamage(this, mDamageKeeper, mCarryKeeper, mTrigger,
                                                       mConst, mRecoverySafetyPoint, mInput)) {
            if (PlayerFunction::isPlayerDeadStatus(this)) {
                mBindKeeper->cancelBind();
                al::setNerve(this, &NrvPlayerActorHakoniwa.Damage);
                return;
            }
            al::startHitReaction(this, "バインド中ダメージ");
        }
    } else if (mHackKeeper->getHackSensor()) {
        if (mHackKeeper->receiveRequestDamage() &&
            processPlayerDamage(this, mDamageKeeper, mCarryKeeper, mTrigger, mConst,
                                mRecoverySafetyPoint, mInput)) {
            if (PlayerFunction::isPlayerDeadStatus(this)) {
                mHackKeeper->sendMarioDead();
                al::setNerve(this, &NrvPlayerActorHakoniwa.Damage);
                return;
            }
            al::startHitReaction(this, "ダメージひょうい");
            mModelChanger->startDamageStopDemo();
        }
    } else if (mTrigger->isOnAnyDamage() &&
               processPlayerDamage(this, mDamageKeeper, mCarryKeeper, mTrigger, mConst,
                                   mRecoverySafetyPoint, mInput)) {
        PlayerEquipmentFunction::tryNoticeEquipPlayerDamage(mEquipmentUser);
        if (mTrigger->isOn(PlayerTrigger::ECollisionTrigger_val5))
            mStainControl->recordPoison();

        if (mTrigger->isOn(PlayerTrigger::EPreMovementTrigger_val1)) {
            startRecoveryFromDeathArea(this, mHackCap, mCarryKeeper, mBindKeeper, mEquipmentUser,
                                       mStateAbyss);
            return;
        }
        if (mTrigger->isOnDamageFire()) {
            const char* materialCode = nullptr;
            if (mTrigger->tryGetRecMaterialCode(&materialCode))
                al::setMaterialCode(this, materialCode);
            mTrigger->set(PlayerTrigger::EPreMovementTrigger_val3);
            al::setNerve(this, &NrvPlayerActorHakoniwa.DamageFire);
            return;
        }
        if (mTrigger->isOn(PlayerTrigger::EActionTrigger_val23)) {
            if (!PlayerFunction::isPlayerDeadStatus(this))
                return;
        } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Swim)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.DamageSwim);
            return;
        } else if (mModelChanger->is2DModel() && !PlayerFunction::isPlayerDeadStatus(this)) {
            al::startHitReaction(this, "ダメージ2D");
            mModelChanger->startDamageStopDemo();
            return;
        } else {
            PlayerStateWallCatch* stateWallCatch = mStateWallCatch;
            if (al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch) &&
                stateWallCatch->isWallCatchForm())
                stateWallCatch->endFallFromWall();
        }
        al::setNerve(this, &NrvPlayerActorHakoniwa.Damage);
        return;
    }

    PlayerBindKeeper* hitPushBindKeeper = mBindKeeper;
    if (mHitPush->isHit() && !PlayerFunction::isPlayerDeadStatus(this) &&
        !hitPushBindKeeper->getBindSensor() && !al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss) &&
        (!mHitPush->isBlowDown() || processPlayerDamage(this, mDamageKeeper, mCarryKeeper, mTrigger,
                                                        mConst, mRecoverySafetyPoint, mInput))) {
        PlayerEquipmentFunction::tryNoticeEquipPlayerDamage(mEquipmentUser);
        al::setVelocity(this, mHitPush->getPush());
        if (!mHitPush->isBlowDown())
            mTrigger->set(PlayerTrigger::EPreMovementTrigger_val4);
        mTrigger->set(PlayerTrigger::EPreMovementTrigger_val2);
        PlayerStateWallCatch* stateWallCatch = mStateWallCatch;
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch) &&
            stateWallCatch->isWallCatchForm())
            stateWallCatch->endFallFromWall();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Damage);
    }

    PlayerBindKeeper* riseBindKeeper = mBindKeeper;
    PlayerJudgeStartRise* judgeStartRise = mJudgeStartRise;
    if (!PlayerFunction::isPlayerDeadStatus(this) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Rise) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss) && !riseBindKeeper->getBindSensor() &&
        rs::updateJudgeAndResult(judgeStartRise)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Rise);
        return;
    }

    PlayerBindKeeper* jumpBindKeeper = mBindKeeper;
    IUsePlayerCollision* playerCollision = getPlayerCollision();
    if (!PlayerFunction::isPlayerDeadStatus(this) && !jumpBindKeeper->getBindSensor() &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Camera) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Swim) &&
        rs::isTouchJumpCode(this, playerCollision)) {
        rs::requestReactionJumpCode(this, mCollider, mBodyHitSensor);
        if (rs::isCollisionCodeJump(mCollider)) {
            rs::requestRiseCameraAngleByPlayerCollideJumpCode(this);
            PlayerJumpMessageRequest* jumpRequest = mJumpMessageRequest;
            const f32 jumpPower = mConst->getTrampleJumpCodePower();
            jumpRequest->jumpType = static_cast<PlayerJumpType>(1);
            jumpRequest->jumpPower = jumpPower;
            jumpRequest->extendFrame = 0;
        } else {
            PlayerJumpMessageRequest* jumpRequest = mJumpMessageRequest;
            const f32 jumpPower = mConst->getTrampleJumpCodePowerSmall();
            jumpRequest->jumpType = static_cast<PlayerJumpType>(1);
            jumpRequest->jumpPower = jumpPower;
            jumpRequest->extendFrame = 0;
        }
        al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
        return;
    }

    PlayerRecoverySafetyPoint* recoverySafetyPoint = mRecoverySafetyPoint;
    PlayerPuppet* puppet = mPuppet;
    PlayerAreaChecker* areaChecker = mAreaChecker;
    PlayerHackKeeper* recoveryHackKeeper = mHackKeeper;
    if (recoverySafetyPoint->isActiveRecoveryArea() && recoverySafetyPoint->isValid() &&
        !rs::isPlayerSafetyPointRecovery(this) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) &&
        (!al::isNerve(this, &NrvPlayerActorHakoniwa.Bind) || puppet->isBindRecoveryEnabled()) &&
        (!al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) ||
         !recoveryHackKeeper->isActiveHackStartDemo())) {
        const al::AreaObj* recoveryArea = nullptr;
        if (!areaChecker->isInRecovery(&recoveryArea, al::getTrans(this))) {
            PlayerHackKeeper* forceRecoveryHackKeeper = mHackKeeper;
            if (forceRecoveryHackKeeper->getHackSensor())
                forceRecoveryHackKeeper->cancelForceRecovery();
            startRecoveryFromDeathArea(this, mHackCap, mCarryKeeper, mBindKeeper, mEquipmentUser,
                                       mStateAbyss);
        }
    }
}

namespace {

void updateSeparateCap(al::LiveActor* player, PlayerSeparateCapFlag* separateCapFlag, HackCap* cap,
                       PlayerAnimator* animator, PlayerColliderHakoniwa* collider,
                       PlayerModelChangerHakoniwa* modelChanger, PlayerCarryKeeper* carryKeeper,
                       PlayerPuppet* puppet, PlayerEquipmentUser* equipmentUser,
                       const PlayerConst* playerConst) {
    const al::IUseSceneObjHolder* sceneObjHolder = player;
    bool isSeparateCapEnabled = false;
    if (rs::isSeparatePlay(sceneObjHolder) && !modelChanger->is2DModel()) {
        GameDataHolderAccessor accessor(sceneObjHolder);
        isSeparateCapEnabled = GameDataFunction::isEnableCap(accessor);
    }

    bool isSeparateCapLocal = false;
    if (!isSeparateCapEnabled || rs::isPlayerSafetyPointRecovery(player) ||
        cap->isHackInvalidSeparatePlay() ||
        PlayerEquipmentFunction::isEquipmentNoCapThrow(equipmentUser)) {
        separateCapFlag->setSeparateCap(false);
    } else {
        f32 ceilingSpace = 0.0f;
        bool hasCeilingSpace = false;
        if (!carryKeeper->isCarryUp()) {
            hasCeilingSpace = rs::tryCalcPlayerCeilingSpace(
                &ceilingSpace, player, playerConst->getSeparateCheckHeight(), 0.0f);
        }

        if (al::isNerve(player, &NrvPlayerActorHakoniwa.Bind)) {
            bool isSeparateCap = puppet->isBindSeparateCapEnabled();
            isSeparateCapLocal = hasCeilingSpace && isSeparateCap;
            if (isSeparateCap)
                separateCapFlag->setSeparateCap(true);
            else
                separateCapFlag->setSeparateCap(false);
        } else {
            separateCapFlag->setSeparateCap(true);
            isSeparateCapLocal = hasCeilingSpace;
        }
    }

    if (isSeparateCapLocal)
        separateCapFlag->setSeparateCapLocal(true);
    else
        separateCapFlag->setSeparateCapLocal(false);

    cap->updateSeparateMode(separateCapFlag);

    sead::Vector3f targetOffset = {0.0f, 0.0f, 0.0f};
    u32 flags = separateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) == 0 && (flags & 0xFF) != 0)
        collider->calcSeparateCapLocalOffset(&targetOffset);

    sead::Vector3f localOffset = {0.0f, 0.0f, 0.0f};
    al::lerpVec(&localOffset, *separateCapFlag->getSeparateCapLocalOffset(), targetOffset,
                playerConst->getSeparateOffsetLerpRate());
    sead::Vector3f* currentOffset = separateCapFlag->getSeparateCapLocalOffset();
    f32* currentValues = &currentOffset->x;
    const f32* localValues = &localOffset.x;
    currentValues[0] = localValues[0];
    currentValues[1] = localValues[1];
    currentValues[2] = localValues[2];

    syncSeparateCapVisibility(animator, cap, modelChanger, separateCapFlag);
}

}  // namespace

namespace {

inline const al::IUseAreaObj* getAreaObjUser(const al::LiveActor* actor) {
    return actor;
}

}  // namespace

void PlayerActorHakoniwa::control() {
    const al::IUseAreaObj* areaObjUser = getAreaObjUser(this);
    if (al::AreaObjGroup* group = al::tryFindAreaObjGroup(areaObjUser, "HackerCheckKeepOnArea")) {
        s32 keepOnHackActorType = 0;
        const s32 groupSize = group->getSize();
        for (s32 i = 0; i < groupSize; i++) {
            al::AreaObj* area = group->getAreaObj(i);
            if (!area->isInVolume(al::getTrans(this)))
                continue;
            al::tryGetAreaObjArg(&keepOnHackActorType, area, "HackActorType");
            if (rs::isPlayerHackType(this, keepOnHackActorType))
                al::onStageSwitch(area, "SwitchAreaOn");
            else
                al::offStageSwitch(area, "SwitchAreaOn");
        }
    }

    if (al::AreaObj* area =
            al::tryFindAreaObj(areaObjUser, "HackerCheckArea", al::getTrans(this))) {
        s32 hackActorType = 0;
        al::tryGetAreaObjArg(&hackActorType, area, "HackActorType");
        if (rs::isPlayerHackType(this, hackActorType))
            al::onStageSwitch(area, "SwitchAreaOn");
    }

    if (al::isNerve(this, &NrvPlayerActorHakoniwa.Hack)) {
        if (!mHackKeeper->getHackSensor() || !mHackKeeper->isHackDemoStarted()) {
            mDamageKeeper->update(mModelChanger, rs::isJudge(mJudgeRecoveryLifeFast), true);
            if (mHackKeeper->getHackSensor()) {
                mHackKeeper->sendSyncDamageVisibility();
                if (mHackKeeper->getHackSensor() && !mHackKeeper->isHack())
                    mHackCap->syncHackDamageVisibility(rs::isDamageVisibilityHide(this));
            }
        }
    } else {
        mDamageKeeper->update(mModelChanger, rs::isJudge(mJudgeRecoveryLifeFast),
                              !al::isNerve(this, &NrvPlayerActorHakoniwa.Demo));
        if (!rs::isJudge(mInfo->getJudgeSafetyPointRecovery())) {
            const IUseDimension* dimension = this;
            mDimensionKeeper->update();
            mModelChanger->update(!PlayerFunction::isPlayerDeadStatus(this),
                                  mBindKeeper->getBindSensor());
            if (syncDimensionState(this, mDimensionKeeper, mCollider, mInput, dimension,
                                   mModelChanger, mBindKeeper->getBindSensor(), mPuppet, mConst)) {
                mDimensionKeeper->update();
                mModelChanger->update(!PlayerFunction::isPlayerDeadStatus(this),
                                      mBindKeeper->getBindSensor());
                syncDimensionState(this, mDimensionKeeper, mCollider, mInput, dimension,
                                   mModelChanger, mBindKeeper->getBindSensor(), mPuppet, mConst);
            }
        }

        updateCarry();
        PlayerModelChangerHakoniwa* modelChanger = mModelChanger;
        PlayerAnimator* animator = mAnimator;
        PlayerCarryKeeper* carryKeeper = mCarryKeeper;
        const bool isOnGroundRunAngle = rs::isOnGroundRunAngle(this, mCollider, mConst);
        animator->updateModel();
        if (animator->isSubAnimPlaying() &&
            (animator->isSubAnimEnd() || (animator->isSubAnimOnlyAir() && isOnGroundRunAngle)))
            animator->endSubAnim();

        if (!modelChanger->is2DModel() && !carryKeeper->isCarry() &&
            animator->isUpperBodyAnimAttached() && animator->isUpperBodyAnimEnd())
            animator->clearUpperBodyAnim();

        if (mModelChanger->isChange() && mModelChanger->is2DModel() &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.Wait) &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.Run) &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.Squat) &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.Fall) &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.Jump) &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.Bind) &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.Damage) &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss) &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.Dead) &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.Swim)) {
            if (rs::isOnGround(this, mCollider))
                al::setNerve(this, &NrvPlayerActorHakoniwa.Wait);
            else
                al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        }
    }

    u32 reduceOxygen;
    if (rs::isPlayerInWater(this)) {
        reduceOxygen = true;
    } else {
        PlayerStateDamageSwim* stateDamageSwim = mStateDamageSwim;
        reduceOxygen = mStateSwim->isReduceOxygen() || stateDamageSwim->isReduceOxygen();
    }

    const bool wasReduceOxygen = mIsReduceOxygen;
    if (!reduceOxygen || wasReduceOxygen) {
        if (!reduceOxygen && wasReduceOxygen)
            al::endBgmSituation(this, "InWater", false);
    } else {
        al::startBgmSituation(this, "InWater", false, true);
    }
    mIsReduceOxygen = reduceOxygen;
}

namespace {

// NON_MATCHING: exact 0x218-byte body except target 0x420BC4 encodes `AND W8, W25, W0`
// while Clang commutes the source operands to `AND W8, W0, W25`; multiplication, reversed source
// operands, compound assignment, and explicit result temporaries preserve that order or worsen
// codegen. Next hypothesis: recover the original boolean producer/lifetime that keeps W25 first.
bool syncDimensionState(
    al::LiveActor* player, ActorDimensionKeeper* dimensionKeeper, PlayerColliderHakoniwa* collider,
    PlayerInput* input, const IUseDimension* dimension, PlayerModelChangerHakoniwa* modelChanger,
    const al::HitSensor* bindSensor, const PlayerPuppet* puppet, const PlayerConst* playerConst) {
    const bool isChangeEnabled = !bindSensor || puppet->isBindDimensionChangeEnabled();
    if (!modelChanger->is2DModel()) {
        if (modelChanger->isChange()) {
            if (isChangeEnabled)
                rs::pushOutFrom2DArea(player, dimension, collider->getColliderRadius(),
                                      collider->getColliderDiskHalfHeight());
            al::setGravity(player, -sead::Vector3f::ey);
            dimensionKeeper->invalidate();
        }
        return false;
    }

    const s32 dimensionState = isChangeEnabled & rs::isIn2DArea(dimension);
    if (dimensionState != 1) {
        al::setGravity(player, -sead::Vector3f::ey);
        return false;
    }

    const f32 diskHalfHeight = collider->getColliderDiskHalfHeight() + 0.5f;
    sead::Vector3f rotateCenter = {0.0f, 0.0f, 0.0f};
    al::calcUpDir(&rotateCenter, player);
    rotateCenter *= playerConst->getTall() * 0.5f;
    rotateCenter += al::getTrans(player);
    bool isSnapped = false;
    if (rs::snap2DGravityPoseWithRotateCenter(player, collider, dimension, diskHalfHeight, 0.0f,
                                              rotateCenter)) {
        isSnapped = true;
        input->startDimensionSnap();
    }
    return isSnapped;
}

}  // namespace

void PlayerActorHakoniwa::updateCarry() {
    PlayerStateSwim* stateSwim = mStateSwim;
    PlayerStateSandSink* stateSandSink = mStateSandSink;
    PlayerSandSinkAffect* sandSinkAffect = mSandSinkAffect;

    bool isCarryActionEnabled =
        !isCarryActionInvalid(this, mBindKeeper, mStateWallAir, stateSwim, mStatePoleClimb);
    if (isCarryActionEnabled && al::isNerve(this, &NrvPlayerActorHakoniwa.Swim))
        isCarryActionEnabled = stateSwim->isEnableCarryAction();
    else if (isCarryActionEnabled && al::isNerve(this, &NrvPlayerActorHakoniwa.HipDrop))
        isCarryActionEnabled = false;

    if (isCarryActionEnabled && al::isNerve(this, &NrvPlayerActorHakoniwa.SandSink))
        isCarryActionEnabled = stateSandSink->isJump() && sandSinkAffect->isEnableCapThrow();

    if (isCarryActionEnabled) {
        PlayerCarryKeeper* carryKeeper = mCarryKeeper;
        PlayerInput* input = mInput;
        if (carryKeeper->isCarry() && input->isTriggerCarryRelease() && !carryKeeper->isThrow() &&
            carryKeeper->startThrow(input->isTriggerCarryReleaseBySwing())) {
            al::startHitReaction(this, "もの投げ開始");
            rs::resetJudge(mJudgePreInputCapThrow);
            if (mInput->isMove()) {
                sead::Vector3f up = {0.0f, 0.0f, 0.0f};
                rs::calcGroundNormalOrUpDir(&up, this, mCollider);
                sead::Vector3f move = {0.0f, 0.0f, 0.0f};
                mInput->calcMoveDirection(&move, up);
                sead::Quatf quat = sead::Quatf::unit;
                al::makeQuatUpFront(&quat, up, move);
                al::updatePoseQuat(this, quat);
            }
        }
    }

    mCarryKeeper->update();
    if (mCarryKeeper->isCarry() && mAreaChecker->isInCarryBan(al::getTrans(this)))
        mCarryKeeper->startCancelAndRelease();
}

// NON_MATCHING: exact 0x748 body with 71/71 semantic calls. The first structural mismatch is the
// persistent velocity scratch at SP+0x20 versus target SP+0x10 at 0x420FCC. A direct target-shaped
// WaterSurfaceShadow CFG restores the target call order but grows to 0x750; caching the shadow
// pointer instead shrinks to 0x740 and loses that order. Next hypothesis: recover the adjacent
// temporary lifetime that swaps the two 12-byte stack colors without changing the exact frame.
void PlayerActorHakoniwa::updateCollider() {
    al::updatePoseTrans(this, al::getTrans(this));
    syncSensorAndCollision();

    PlayerStateHack* stateHack = mStateHack;
    const bool skipCollider = al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) ||
                              al::isNerve(this, &NrvPlayerActorHakoniwa.Dead) ||
                              (al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) ?
                                   stateHack->isIgnoreUpdateCollider() :
                                   PlayerFunction::isPlayerDeadStatus(this) &&
                                       !al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss));

    if (skipCollider) {
        if (mWaterSurfaceShadow->isAppearShadow())
            mWaterSurfaceShadow->disappearShadow();

        al::WaterSurfaceFinder* waterSurfaceFinder = mWaterSurfaceFinder;
        PlayerEffect* effect = mEffect;
        const sead::Vector3f& waterSurfaceTrans = al::getTrans(this);
        const sead::Vector3f up = -al::getGravity(this);
        waterSurfaceFinder->update(waterSurfaceTrans, up, 200.0f);
        effect->updateWaterSurfaceMtx(waterSurfaceFinder);
        const sead::Vector3f heightUp = -al::getGravity(this);
        const sead::Vector3f& heightTrans = al::getTrans(this);
        mCollider->updateHeightCheck(heightTrans, heightUp, false);
        mCollider->updateCeilingCheck(heightTrans, heightUp, 0.0f, 0.0f);
        mExternalVelocity->resetSnapForce();

        if (al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) && mDemoActionFlag->isDemoAction()) {
            f32 groundHeight = 0.0f;
            if (mCollider->isAboveGround())
                groundHeight = mCollider->getGroundHeight();
            f32 move = mConst->getGravityMove();
            if (groundHeight >= move) {
                move = groundHeight;
                if (groundHeight > 100.0f)
                    move = 100.0f;
            }
            mCollider->updateCollider(al::getGravity(this) * move);
            mDemoActionFlag->clearDemoAction();
        }
        return;
    }

    sead::Vector3f velocity = {0.0f, 0.0f, 0.0f};
    if (PlayerFunction::isPlayerDeadStatus(this) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.Camera) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.PoleClimb)) {
        velocity = al::getVelocity(this);
    } else {
        sead::Vector3f cutDir = {0.0f, 0.0f, 0.0f};
        PlayerStateWallAir* stateWallAir = mStateWallAir;
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.WallAir)) {
            stateWallAir->calcSnapMoveCutDir(&cutDir);
            PlayerColliderHakoniwa* snapCollider = mCollider;
            rs::calcSnapVelocitySnapMoveAreaWithCutDir(&velocity, this, snapCollider,
                                                       al::getVelocity(this), 5.0f, cutDir);
        } else {
            PlayerColliderHakoniwa* snapCollider = mCollider;
            rs::calcSnapVelocitySnapMoveArea(&velocity, this, snapCollider, al::getVelocity(this),
                                             5.0f);
        }
    }

    {
        PlayerPushReceiver* pushReceiver = mPushReceiver;
        sead::Vector3f pushedVelocity = velocity + mExternalVelocity->getTotalVelocity();
        pushReceiver->calcPushedVelocityWithCollide(&velocity, pushedVelocity, mCollider,
                                                    mConst->getCollisionRadius());
    }
    WorldEndBorderKeeper* worldEndBorderKeeper = mWorldEndBorderKeeper;
    const sead::Vector3f& worldBorderTrans = al::getTrans(this);
    const bool isSwimOrAir =
        al::isNerve(this, &NrvPlayerActorHakoniwa.Swim) || !rs::isCollidedGround(mCollider);
    worldEndBorderKeeper->update(worldBorderTrans, velocity, isSwimOrAir);
    velocity += mWorldEndBorderKeeper->getVelocity();

    if (isEnableCollisionSnap(this, mStateSpinCap, mStateGrabCeil, mStateDamageFire) &&
        mExternalVelocity->isExistSnapForce()) {
        velocity = mExternalVelocity->getSnapForce();
        mExternalVelocity->resetSnapForce();
    }

    if (al::isNoCollide(this) || mPuppet->isNoCollide()) {
        rs::resetCollision(mCollider);
        sead::Vector3f* trans = al::getTransPtr(this);
        *trans += velocity;
    } else {
        sead::Vector3f* trans = al::getTransPtr(this);
        *trans += mCollider->updateCollider(velocity);
    }

    const sead::Vector3f& gravity = al::getGravity(this);
    const sead::Vector3f& trans = al::getTrans(this);
    al::updatePoseTrans(this, trans);

    {
        al::WaterSurfaceFinder* waterSurfaceFinder = mWaterSurfaceFinder;
        PlayerEffect* effect = mEffect;
        const sead::Vector3f& waterSurfaceTrans = al::getTrans(this);
        const sead::Vector3f up = -al::getGravity(this);
        waterSurfaceFinder->update(waterSurfaceTrans, up, 200.0f);
        effect->updateWaterSurfaceMtx(waterSurfaceFinder);
    }
    mCollider->updateHeightCheck(trans, -gravity, true);

    sead::Vector3f ceilingPos = trans;
    if (al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch) && mStateWallCatch->isWallCatchForm())
        ceilingPos = mStateWallCatch->getCeilingCheckPos();
    mCollider->updateCeilingCheck(ceilingPos, -gravity, 0.0f,
                                  mCarryKeeper->isCarryUp() ? 150.0f : 0.0f);

    PlayerStateSwim* stateSwim = mStateSwim;
    PlayerPuppet* puppet = mPuppet;
    WaterSurfaceShadow* waterSurfaceShadow = nullptr;
    bool hideSurfaceShadow = false;
    if (rs::isPlayer2D(this)) {
        waterSurfaceShadow = mWaterSurfaceShadow;
        hideSurfaceShadow = true;
    } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Swim)) {
        const bool isEnableSurfaceShadow = stateSwim->isEnableSurfaceShadow();
        waterSurfaceShadow = mWaterSurfaceShadow;
        hideSurfaceShadow = !isEnableSurfaceShadow;
    } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Bind)) {
        waterSurfaceShadow = mWaterSurfaceShadow;
        hideSurfaceShadow = !puppet->isWaterSurfaceShadowEnabled();
    } else {
        waterSurfaceShadow = mWaterSurfaceShadow;
    }

    if (hideSurfaceShadow) {
        if (waterSurfaceShadow->isAppearShadow())
            mWaterSurfaceShadow->disappearShadow();
    } else {
        waterSurfaceShadow->update(trans, -gravity, mCollider->getGroundHeight());
    }

    mCollider->updateFallDistanceCheck(trans, velocity, gravity, mConst->getFallSpeedMax());
    if (rs::isCollidedGround(mCollider))
        mComboCounter->reset();
    if (mCarryKeeper->updateCollideLockUp(mCollider, mPushReceiver))
        mCarryKeeper->startCancelAndRelease();
}

namespace {

bool isEnableCollisionSnap(const al::LiveActor* player, const PlayerStateSpinCap* stateSpinCap,
                           const PlayerStateGrabCeil* stateGrabCeil,
                           const PlayerStateDamageFire* stateDamageFire) {
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.Damage))
        return false;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.SpinCap)) {
        if (!stateSpinCap->isEnableCancelAir())
            return stateSpinCap->isEnableCancelGround();
    } else {
        if (al::isNerve(player, &NrvPlayerActorHakoniwa.GrabCeil))
            return stateGrabCeil->isEnableSnapForce();
        if (al::isNerve(player, &NrvPlayerActorHakoniwa.DamageFire))
            return al::isGreaterEqualStep(player,
                                          stateDamageFire->getEnableCancelCollisionSnapFrame());
    }
    return true;
}

}  // namespace

bool PlayerActorHakoniwa::isEnableDemo() {
    return !al::isNerve(this, &NrvPlayerActorHakoniwa.Press) &&
           !PlayerFunction::isPlayerDeadStatus(this);
}

bool PlayerActorHakoniwa::isDamageStopDemo() const {
    return mModelChanger->isDamageStopDemo();
}

void PlayerActorHakoniwa::startDemo() {
    if (mDamageKeeper->isDamageInvalid()) {
        mModelChanger->showModel();
        if (mHackKeeper->getHackSensor()) {
            mHackKeeper->sendSyncDamageVisibility();
            mHackCap->syncHackDamageVisibility(false);
        } else {
            mModelChanger->syncHost(true);
        }
    }

    if (al::isNerve(this, &NrvPlayerActorHakoniwa.Camera)) {
        mStateCameraSubjective->interruptByDemo();
        mModelChanger->syncHost(true);
        setNerveOnGround();
    }

    mInput->resetDemoInput();
}

void PlayerActorHakoniwa::setNerveOnGround() {
    if (rs::updateJudgeAndResult(mJudgeStartWaterSurfaceRun)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Run);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeInWater[0])) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
        return;
    }

    PlayerColliderHakoniwa* collider = mCollider;
    if (mCounterForceRun->getCounter() >= 1) {
        if (!rs::isOnGround(this, collider)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
            return;
        }
    } else if (!rs::isOnGroundRunAngle(this, collider, mConst)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        return;
    }

    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeForceRolling)) {
        setNerveRollingFromGround(this, mTrigger, mCollider);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeSandSink)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SandSink);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
        mStatePoleClimb->setup(mJudgePoleClimb->getCollidedParts(), mJudgePoleClimb->getPosition(),
                               mJudgePoleClimb->getUp(), mJudgePoleClimb->getFront(),
                               mJudgePoleClimb->getAngleOffsetWall(),
                               mJudgePoleClimb->getPoleHeight(),
                               mJudgePoleClimb->getCollisionCode());
        al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
        mStateGrabCeil->setup(mJudgeGrabCeil->getCollidedParts(), mJudgeGrabCeil->getPosition(),
                              mJudgeGrabCeil->getUp(), mJudgeGrabCeil->getFront());
        al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
        return;
    }

    carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeEnableStandUp)) {
        if (rs::judgeAndResetReturnTrue(mJudgePreInputJump)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
            return;
        }
        if (rs::updateJudgeAndResult(mJudgeStartRun)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Run);
            return;
        }
        if (!rs::updateJudgeAndResult(mJudgeStartSquat) || rs::isJudge(mJudgeForceLand)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Wait);
            return;
        }
        if (!rs::isJustLand(mCollider) || al::isNerve(this, &NrvPlayerActorHakoniwa.LongJump)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
            return;
        }
    } else if (carryKeeper->isCarry()) {
        carryKeeper->startCancelAndRelease();
    }

    rs::cutVerticalVelocityGroundNormal(this, getPlayerCollision());
    al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
}

void PlayerActorHakoniwa::endDemo() {
    GaugeAir* gaugeAir = mGaugeAir;
    const al::IUseCamera* camera = this;
    const sead::Vector3f& headPos = mFormSensorCollisionArranger->getHeadPos();
    const f32 xHead = headPos.x;
    const f32 yHead = headPos.y;
    const f32 zHead = headPos.z;

    sead::Vector3f cameraSide;
    cameraSide.set(0.0f, 0.0f, 0.0f);
    al::calcCameraSideDir(&cameraSide, camera, 0);

    sead::Vector3f cameraUp;
    cameraUp.set(0.0f, 0.0f, 0.0f);
    al::calcCameraUpDir(&cameraUp, camera, 0);

    sead::Vector2f layoutPos;
    layoutPos.set(0.0f, 0.0f);
    const sead::Vector3f head(xHead, yHead, zHead);
    const sead::Vector3f worldPos = head + cameraSide * 30.0f + cameraUp * 25.0f;
    al::calcLayoutPosFromWorldPos(&layoutPos, camera, worldPos);
    al::setLocalTrans(gaugeAir, layoutPos);
}

void PlayerActorHakoniwa::startDemoPuppetable() {
    mInput->setPuppetableDemo(true);

    if (mCarryKeeper->isCarry())
        mCarryKeeper->startReleaseDemo();

    if (mHackKeeper->getHackSensor()) {
        if (mHackKeeper->isHackDemoStarted()) {
            startPlayerPuppet();
            return;
        }

        if (rs::isActiveDemoWarpToCheckpoint(this))
            mHackKeeper->sendMarioCheckpointFlagWarp();
        else
            mHackKeeper->sendMarioDemo();
    }

    startPlayerPuppet();
    al::setNerve(this, &NrvPlayerActorHakoniwa.Demo);
}

void PlayerActorHakoniwa::startPlayerPuppet() {
    PlayerAnimator* animator = mAnimator;
    if (animator->isSubAnimPlaying()) {
        mAnimator->endSubAnim();
        animator = mAnimator;
    }

    PlayerModelChangerHakoniwa* modelChanger = mModelChanger;
    if (!mCarryKeeper->isCarry() && !modelChanger->is2DModel() &&
        animator->isUpperBodyAnimAttached()) {
        mAnimator->clearUpperBodyAnim();
    }

    syncSeparateCapVisibility(mAnimator, mHackCap, mModelChanger, mSeparateCapFlag);

    if (!mHackKeeper->getHackSensor())
        mSpinCapAttack->tryCancelCapState(mAnimator);

    if (mBindKeeper->getBindSensor() && !mBindKeeper->isBindKeepDemo())
        mBindKeeper->cancelBindByDemo();

    if (al::isNerve(this, &NrvPlayerActorHakoniwa.Camera))
        mStateCameraSubjective->interruptByDemo();

    mCapManHeroEyesControl->startPuppet();
    mSeparateCapFlag->setPuppetable(true);
    updateSeparateCap(this, mSeparateCapFlag, mHackCap, mAnimator, mCollider, mModelChanger,
                      mCarryKeeper, mPuppet, mEquipmentUser, mConst);

    if (mHackCap->isPutOn()) {
        GameDataHolderAccessor accessor(this);
        if (GameDataFunction::isEnableCap(accessor))
            mAnimator->forceCapOn();
    }

    mModelChanger->syncModelBoneVisibility();
    mDemoActionFlag->reset();
    mWallActionHistory->reset();
    mCollider->resetFallDistance();

    if (mGaugeAir->isAlive())
        mGaugeAir->fastEnd();

    if (!mHackKeeper->getHackSensor() || !mHackKeeper->isHackDemoStarted())
        mDamageKeeper->reset(mModelChanger);

    if (mEquipmentUser->hasEquipment())
        mEquipmentUser->cancelEquip();

    mHackCap->startPuppet();
    mComboCounter->reset();
    al::setVelocityZero(this);
}

void PlayerActorHakoniwa::cancelHackPlayerPuppetDemo() {
    if (!mHackKeeper->getHackSensor())
        return;

    if (rs::isActiveDemoWarpToCheckpoint(this))
        mHackKeeper->sendMarioCheckpointFlagWarp();
    else
        mHackKeeper->sendMarioDemo();
}

void PlayerActorHakoniwa::endDemoPuppetable() {
    endPlayerPuppet();

    if (mHackKeeper->getHackSensor() && mHackKeeper->isHackDemoStarted())
        return;

    if (mBindKeeper->isKeepDemo()) {
        mBindKeeper->sendMsgBindKeepDemoEnd();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Bind);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeInWater[0])) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
        return;
    }

    rs::resetCollision(mCollider);
    PlayerColliderHakoniwa* collider = mCollider;
    const sead::Vector3f& gravity = al::getGravity(this);
    const f32 gravityMove = mConst->getGravityMove();
    const sead::Vector3f velocity = gravity * gravityMove;
    collider->updateCollider(velocity);
    if (rs::isCollidedGround(mCollider)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Wait);
        mTrigger->set(PlayerTrigger::EDemoEndTrigger_val0);
    } else {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
    }
    rs::resetCollision(mCollider);
}

void PlayerActorHakoniwa::endPlayerPuppet() {
    mJointControlKeeper->setPuppetMode(false);
    mCapActionHistory->clearLandLimit();
    mSeparateCapFlag->setPress(false);
    mBindKeeper->resetInvalidTimer();
    mHackCap->endPuppet();
    mCapManHeroEyesControl->endPuppet();

    if (!mModelChanger->isHiddenModel())
        mModelChanger->showSilhouette();

    if (!mHackKeeper->getHackSensor())
        mEffect->restartInvincibleEffect();

    if (!mDemoActionFlag->isInvalidateCapOn() && mHackCap->isPutOn()) {
        const GameDataHolderAccessor accessor(this);
        if (GameDataFunction::isEnableCap(accessor))
            mAnimator->forceCapOn();
    }

    mModelChanger->syncModelBoneVisibility();
    if (!mModelChanger->is2DModel()) {
        al::LiveActor* modelActor = mModelHolder->getCurrentModelActor();
        al::setDepthShadowMapLengthFromActorTransFlag(
            modelActor, true, PlayerFunction::getPlayerDepthGroundShadowName());
    }

    mAnimator->endDemoInvalidateModelAlpha();
}

void PlayerActorHakoniwa::startDemoShineGet() {
    if (mHackKeeper->getHackSensor()) {
        if (mCarryKeeper->isCarry())
            mCarryKeeper->startDemoShineGet();

        mHackCap->startHackShineGetDemo();
        sead::Vector3f followTrans = {0.0f, 0.0f, 0.0f};
        mHackCap->calcHackFollowTrans(&followTrans, false);
        al::resetPosition(this, followTrans);
        mAnimator->startAnim("GetShineSub");
        mModelChanger->showModel();
        mModelChanger->syncHost(true);
        mModelChanger->resetPosition();
        mHackKeeper->setHack(true);
        startPlayerPuppet();
        return;
    }

    mInput->setPuppetableDemo(true);
    if (mBindKeeper->getBindSensor())
        mBindKeeper->sendMsgBindKeepDemoStart();
    if (mCarryKeeper->isCarry())
        mCarryKeeper->startDemoShineGet();
    startPlayerPuppet();
    al::setNerve(this, &NrvPlayerActorHakoniwa.Demo);
}

void PlayerActorHakoniwa::endDemoShineGet() {
    if (mCarryKeeper->isCarry())
        mCarryKeeper->endDemoShineGet();

    if (!mHackKeeper->getHackSensor()) {
        endDemoPuppetable();
        return;
    }

    mModelChanger->hideModel();
    mModelChanger->syncHost(true);
    mHackCap->endHackShineGetDemo();
    mHackKeeper->setHack(false);
    endPlayerPuppet();
}

void PlayerActorHakoniwa::startDemoMainShineGet() {
    mStainControl->noticeMainShineGet();
    startDemoPuppetable();
}

void PlayerActorHakoniwa::endDemoMainShineGet() {
    endDemoPuppetable();
}

void PlayerActorHakoniwa::startDemoHack() {
    mHackKeeper->startDemo();
    al::setVelocityZero(this);
}

void PlayerActorHakoniwa::endDemoHack() {
    mHackKeeper->endDemo();
}

void PlayerActorHakoniwa::startDemoKeepBind() {
    mInput->setPuppetableDemo(true);

    if (mBindKeeper->getBindSensor())
        mBindKeeper->sendMsgBindKeepDemoStart();

    if (mCarryKeeper->isCarry())
        mCarryKeeper->startReleaseDemo();

    if (mHackKeeper->getHackSensor()) {
        if (rs::isActiveDemoWarpToCheckpoint(this))
            mHackKeeper->sendMarioCheckpointFlagWarp();
        else
            mHackKeeper->sendMarioDemo();
    }

    startPlayerPuppet();
    al::setNerve(this, &NrvPlayerActorHakoniwa.Demo);
}

void PlayerActorHakoniwa::noticeDemoKeepBindExecute() {
    mBindKeeper->sendMsgBindKeepDemoExecute();
}

void PlayerActorHakoniwa::endDemoKeepBind() {
    endDemoPuppetable();
}

void PlayerActorHakoniwa::startDemoKeepCarry() {
    if (!mCarryKeeper->isCarry()) {
        startDemoPuppetable();
        return;
    }

    mCarryKeeper->startDemoKeepCarry();
    if (mHackKeeper->getHackSensor()) {
        if (rs::isActiveDemoWarpToCheckpoint(this))
            mHackKeeper->sendMarioCheckpointFlagWarp();
        else
            mHackKeeper->sendMarioDemo();
    }

    startPlayerPuppet();
    al::setNerve(this, &NrvPlayerActorHakoniwa.Demo);
}

void PlayerActorHakoniwa::endDemoKeepCarry() {
    endDemoPuppetable();
}

al::DemoActor* PlayerActorHakoniwa::getDemoActor() {
    if (mHackKeeper->getHackSensor() && mHackKeeper->isStartedHacking())
        return static_cast<al::DemoActor*>(mHackKeeper->getHack());

    return static_cast<al::DemoActor*>(mModelHolder->getCurrentModelActor());
}

PlayerAnimator* PlayerActorHakoniwa::getDemoAnimator() {
    if (mHackKeeper->getHackSensor() && mHackKeeper->isStartedHacking())
        return nullptr;

    return mAnimator;
}

PlayerPuppet* PlayerActorHakoniwa::getPlayerPuppet() {
    return mPuppet;
}

IUsePlayerCollision* PlayerActorHakoniwa::getPlayerCollision() const {
    return mCollider;
}

u32 PlayerActorHakoniwa::getPortNo() const {
    if (rs::isSeparatePlay(this))
        return PlayerInput::getSeparatePlay1P();

    return PlayerActorBase::getPortNo();
}

// NON_MATCHING: exact 1164-byte body; remaining difference is the shared wall-normal temporary
// scheduling at target 0x42250C. Next hypothesis: recover the setup-argument/local lifetime that
// delays the final component negation/store.
void PlayerActorHakoniwa::exeWait() {
    if (al::isFirstStep(this)) {
        mCapActionHistory->clearLandLimitStandAngle();
        rs::resetJudge(mJudgeSpeedCheckFall);
        rs::resetJudge(mJudgeStartRun);
    }

    mExternalVelocity->requestApplyLastGroundInertia();
    tryActionCapReturn();
    al::updateNerveState(this);

    if (al::isFirstStep(this) && mStateWait->isLandStain()) {
        mStainControl->recordSandHeavyLand(rs::getMaterialCodeGround(mCollider));
        if (mTrigger->isOn(PlayerTrigger::EMaterialChangeTrigger_val0))
            mWetControl->recordHeavyLandPuddle();
    }

    if (rs::updateJudgeAndResult(mJudgeWallCatchInputDir)) {
        const sead::Vector3f& wallNormal = mJudgeWallCatchInputDir->getCollidedWallNormal();
        sead::Vector3f oppositeNormal(-wallNormal.x, -wallNormal.y, -wallNormal.z);
        mStateWallCatch->setup(mJudgeWallCatchInputDir->getCollidedWallPart(),
                               mJudgeWallCatchInputDir->getPosition(), oppositeNormal,
                               mJudgeWallCatchInputDir->getNormalAtPos());
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallCatch);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeSpeedCheckFall)) {
        mExternalVelocity->cancelAndFeedbackLastGroundInertia(this, mConst->getJumpInertiaRate(),
                                                              false);
        mJudgeWallCatchInputDir->validateFallJudge();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        return;
    }

    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeForceRolling)) {
        setNerveRollingFromGround(this, mTrigger, mCollider);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeSandSink)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SandSink);
        return;
    }

    carryKeeper = mCarryKeeper;
    if (!rs::updateJudgeAndResult(mJudgeEnableStandUp)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
        getPlayerCollision();
        mStatePoleClimb->setup(mJudgePoleClimb->getCollisionParts(),
                               mJudgePoleClimb->getContactPos(), mJudgePoleClimb->getPoleUp(),
                               mJudgePoleClimb->getPoleFront(), mJudgePoleClimb->getAngleOffset(),
                               mJudgePoleClimb->getStartHeight(), mJudgePoleClimb->getPoleCode());
        al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
        getPlayerCollision();
        mStateGrabCeil->setup(mJudgeGrabCeil->getCollisionParts(), mJudgeGrabCeil->getContactPos(),
                              mJudgeGrabCeil->getGrabUp(), mJudgeGrabCeil->getGrabFront());
        al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
        return;
    }

    const bool isEnableCancelAction = mStateWait->isEnableCancelAction();
    if (isEnableCancelAction) {
        if (rs::updateJudgeAndResult(mJudgeCameraSubjective)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Camera);
            return;
        }
        if (tryActionCapSpinAttackImpl(true)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
            return;
        }
        if (rs::updateJudgeAndResult(mJudgeStartRun)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Run);
            return;
        }
    }

    if (mStateWait->isEnableCancelHipDropJump()) {
        if (rs::judgeAndResetReturnTrue(mJudgePreInputJump)) {
            mJumpMessageRequest->jumpType = PlayerJumpType::SwimJumpHipDrop;
            al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
            return;
        }
        if (mJudgeStartRolling->judgeCancelHipDrop()) {
            mTrigger->set(PlayerTrigger::EActionTrigger_val17);
            al::setNerve(this, &NrvPlayerActorHakoniwa.Rolling);
            return;
        }
    }

    if (isEnableCancelAction) {
        if (rs::judgeAndResetReturnTrue(mJudgePreInputJump)) {
            mExternalVelocity->cancelAndFeedbackLastGroundInertia(
                this, mConst->getJumpInertiaRate(), false);
            mTrigger->set(PlayerTrigger::EActionTrigger_val21);
            al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
            return;
        }
        if (rs::updateJudgeAndResult(mJudgeStartSquat)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
            return;
        }
        if (rs::updateJudgeAndResult(mJudgeInWater[0])) {
            PlayerTrigger* trigger = mTrigger;
            if (mActionDiveInWater->isDiveInWaterAnim())
                trigger->set(PlayerTrigger::EActionTrigger_val9);
            al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
            return;
        }
    }

    const IUsePlayerCeilingCheck* ceilingCheck = mCollider;
    HackCap* hackCap = mHackCap;
    HackCapJudgePreInputHoveringJump* hoveringJudge = mHackCapJudgePreInputHoveringJump;
    if (ceilingCheck->isEnableStandUp() && rs::isPlayer2D(hackCap) &&
        hackCap->isEnableSpinAttack() && rs::isJudge(hoveringJudge)) {
        hoveringJudge = mHackCapJudgePreInputHoveringJump;
        PlayerJumpMessageRequest* request = mJumpMessageRequest;
        rs::resetJudge(hoveringJudge);
        hoveringJudge->setDisabled(true);
        request->jumpType = static_cast<PlayerJumpType>(18);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
        return;
    }

    if (mStateWait->tryConnectWait())
        rs::resetCollision(mCollider);
}

bool PlayerActorHakoniwa::tryActionCapReturn() {
    const GameDataHolderAccessor accessor(this);
    if (!GameDataFunction::isEnableCap(accessor) || rs::is2D(this) ||
        !mInput->isTriggerCapReturn() || !mHackCap->isRequestableReturn())
        return false;

    if (mCarryKeeper->isCarry()) {
        const u32 flags = mSeparateCapFlag->getRawFlags();
        if ((flags & 0xFF0000) != 0 || (flags & 0xFF) == 0)
            return false;
    }

    bool isReaction = false;
    if (mHackCap->requestReturn(&isReaction)) {
        const u32 flags = mSeparateCapFlag->getRawFlags();
        if ((flags & 0xFF0000) != 0 || (flags & 0xFF) == 0)
            rs::resetJudge(mJudgePreInputCapThrow);
        else
            rs::resetJudge(mHackCapJudgePreInputSeparateThrow);
        return true;
    }

    if (isReaction)
        al::startHitReaction(this, "帽子が戻せない");
    return false;
}

namespace {

void setNerveRollingFromGround(al::LiveActor* player, PlayerTrigger* trigger,
                               IUsePlayerCollision* collision) {
    trigger->set(PlayerTrigger::EActionTrigger_Rolling);
    sead::Vector3f groundNormal = {0.0f, 0.0f, 0.0f};
    rs::calcGroundNormalOrGravityDir(&groundNormal, player, collision);
    sead::Vector3f slideDir = {0.0f, 0.0f, 0.0f};
    if (rs::calcSlideDir(&slideDir, al::getGravity(player), groundNormal)) {
        sead::Quatf quat = sead::Quatf::unit;
        al::makeQuatFrontUp(&quat, slideDir, groundNormal);
        al::updatePoseQuat(player, quat);
    }
    al::setNerve(player, &NrvPlayerActorHakoniwa.Rolling);
}

}  // namespace

bool PlayerActorHakoniwa::tryActionCapSpinAttack() {
    return tryActionCapSpinAttackImpl(true);
}

void PlayerActorHakoniwa::exeSquat() {
    if (al::isFirstStep(this)) {
        mCapActionHistory->clearLandLimitStandAngle();
        rs::resetJudge(mJudgeSpeedCheckFall);
    }

    tryActionSeparateCapThrow();
    if (al::updateNerveState(this)) {
        tryActionCapReturn();
        setNerveOnGround();
        return;
    }

    mExternalVelocity->requestApplyLastGroundInertia();
    if (rs::updateJudgeAndResult(mJudgeSpeedCheckFall)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        return;
    }

    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeForceRolling)) {
        setNerveRollingFromGround(this, mTrigger, mCollider);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeSandSink)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SandSink);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
        (void)getPlayerCollision();
        mStatePoleClimb->setup(mJudgePoleClimb->getCollidedParts(), mJudgePoleClimb->getPosition(),
                               mJudgePoleClimb->getUp(), mJudgePoleClimb->getFront(),
                               mJudgePoleClimb->getAngleOffsetWall(),
                               mJudgePoleClimb->getPoleHeight(),
                               mJudgePoleClimb->getCollisionCode());
        al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
        (void)getPlayerCollision();
        mStateGrabCeil->setup(mJudgeGrabCeil->getCollidedParts(), mJudgeGrabCeil->getPosition(),
                              mJudgeGrabCeil->getUp(), mJudgeGrabCeil->getFront());
        al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeStartRolling)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Rolling);
        return;
    }

    tryActionCapReturn();
    if (rs::judgeAndResetReturnTrue(mJudgePreInputJump)) {
        if (mModelChanger->is2DModel()) {
            PlayerJumpMessageRequest* request = mJumpMessageRequest;
            const bool isEnableStandUp = rs::updateJudgeAndResult(mJudgeEnableStandUp);
            request->jumpType = static_cast<PlayerJumpType>(15);
            request->isEnableStandUp = isEnableStandUp;
            al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
            return;
        }

        if (rs::updateJudgeAndResult(mJudgeEnableStandUp)) {
            if (mStateSquat->isEnableLongJump()) {
                al::setNerve(this, &NrvPlayerActorHakoniwa.LongJump);
                return;
            }
            mJumpMessageRequest->jumpType = static_cast<PlayerJumpType>(14);
            al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
            return;
        }
    } else {
        if (rs::updateJudgeAndResult(mJudgeInWater[0])) {
            PlayerTrigger* trigger = mTrigger;
            if (mActionDiveInWater->isDiveInWaterAnim())
                trigger->set(PlayerTrigger::EActionTrigger_DiveInWater);
            al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
            return;
        }

        const IUsePlayerCeilingCheck* ceilingCheck = mCollider;
        HackCap* hackCap = mHackCap;
        HackCapJudgePreInputHoveringJump* hoveringJudge = mHackCapJudgePreInputHoveringJump;
        if (ceilingCheck->isEnableStandUp() && rs::isPlayer2D(hackCap) &&
            hackCap->isEnableSpinAttack() && rs::isJudge(hoveringJudge)) {
            hoveringJudge = mHackCapJudgePreInputHoveringJump;
            PlayerJumpMessageRequest* request = mJumpMessageRequest;
            rs::resetJudge(hoveringJudge);
            hoveringJudge->setDisabled(true);
            request->jumpType = static_cast<PlayerJumpType>(18);
            al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
            return;
        }
    }
}

bool PlayerActorHakoniwa::tryActionSeparateCapThrow() {
    const u32 flags = mSeparateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) != 0 || (flags & 0xFF) == 0 ||
        PlayerEquipmentFunction::isEquipmentNoCapThrow(mEquipmentUser))
        return false;

    const GameDataHolderAccessor accessor(this);
    if (!GameDataFunction::isEnableCap(accessor) || rs::is2D(this) ||
        mHackCap->isRequestableReturn() || !mHackCap->isEnableThrowSeparate() ||
        !mSandSinkAffect->isEnableCapThrow())
        return false;

    PlayerColliderHakoniwa* collider = mCollider;
    PlayerConst* playerConst = mConst;
    if (!al::isNerve(this, &NrvPlayerActorHakoniwa.GrabCeil) &&
        !(collider->getSafetyCeilSpace() >= playerConst->getSeparateEnableThrowHeight()))
        return false;

    if (rs::judgeAndResetReturnTrue(mHackCapJudgePreInputSeparateJump)) {
        sead::Vector3f moveDir(0.0f, 0.0f, 0.0f);
        sead::Vector3f groundNormal(0.0f, 0.0f, 0.0f);
        PlayerInput* input = mInput;
        rs::calcGroundNormalOrGravityDir(&groundNormal, this, mCollider);
        input->calcCapSeparateMoveInput(&moveDir, groundNormal);
        if (!al::tryNormalizeOrZero(&moveDir) &&
            !rs::calcAlongDirFront(&moveDir, this, groundNormal))
            al::calcFrontDir(&moveDir, this);
        mHackCap->startThrowSeparatePlayJump(moveDir, groundNormal, 1.0f);
        mJudgePreInputCapThrow->recordSeparateJudge();
        return true;
    }

    if (!rs::judgeAndResetReturnTrue(mHackCapJudgePreInputSeparateThrow))
        return false;

    sead::Vector3f moveDir(0.0f, 0.0f, 0.0f);
    sead::Vector3f groundNormal(0.0f, 0.0f, 0.0f);
    PlayerInput* input = mInput;
    rs::calcGroundNormalOrGravityDir(&groundNormal, this, mCollider);
    input->calcCapSeparateMoveInput(&moveDir, groundNormal);
    if (!al::tryNormalizeOrZero(&moveDir) && !rs::calcAlongDirFront(&moveDir, this, groundNormal))
        al::calcFrontDir(&moveDir, this);

    f32 speed = al::calcSpeedExceptDir(this, groundNormal);
    const f32 hideSpeed = mHackCap->calcSeparateHideSpeedH(groundNormal);
    speed = speed > hideSpeed ? speed : hideSpeed;
    const bool isFast = speed > mConst->getDashFastBorderSpeed();
    mHackCap->startThrowSeparatePlay(moveDir, groundNormal, 1.0f, isFast);
    mJudgePreInputCapThrow->recordSeparateJudge();
    return true;
}

// NON_MATCHING: exact 1508-byte body; remaining executable difference is the shared wall-normal
// temporary scheduling. Next hypothesis: recover the setup-argument/local lifetime that delays the
// final component negation/store.
void PlayerActorHakoniwa::exeRun() {
    if (al::isFirstStep(this)) {
        mCapActionHistory->clearLandLimitStandAngle();
        rs::resetJudge(mJudgeSpeedCheckFall);
    }

    tryActionCapReturn();
    if (al::updateNerveState(this)) {
        setNerveOnGround();
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.Fall)) {
            mTrigger->set(PlayerTrigger::EActionTrigger_val14);
            mJudgeWallCatchInputDir->validateFallJudge();
        }
        return;
    }

    if (mTrigger->isOn(PlayerTrigger::EMaterialChangeTrigger_val0) &&
        rs::isSubAnimHeadSlidingLand(mAnimator))
        mWetControl->recordPuddleRolling();

    mExternalVelocity->requestApplyLastGroundInertia();
    if (rs::updateJudgeAndResult(mJudgeWallCatchInputDir)) {
        const sead::Vector3f oppositeNormal = -mJudgeWallCatchInputDir->getCollidedWallNormal();
        mStateWallCatch->setup(mJudgeWallCatchInputDir->getCollidedWallPart(),
                               mJudgeWallCatchInputDir->getPosition(), oppositeNormal,
                               mJudgeWallCatchInputDir->getNormalAtPos());
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallCatch);
        return;
    }

    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeForceRolling)) {
        setNerveRollingFromGround(this, mTrigger, mCollider);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeSandSink)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SandSink);
        return;
    }

    carryKeeper = mCarryKeeper;
    if (!rs::updateJudgeAndResult(mJudgeEnableStandUp)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
        getPlayerCollision();
        mStatePoleClimb->setup(mJudgePoleClimb->getCollisionParts(),
                               mJudgePoleClimb->getContactPos(), mJudgePoleClimb->getPoleUp(),
                               mJudgePoleClimb->getPoleFront(), mJudgePoleClimb->getAngleOffset(),
                               mJudgePoleClimb->getStartHeight(), mJudgePoleClimb->getPoleCode());
        al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
        getPlayerCollision();
        mStateGrabCeil->setup(mJudgeGrabCeil->getCollisionParts(), mJudgeGrabCeil->getContactPos(),
                              mJudgeGrabCeil->getGrabUp(), mJudgeGrabCeil->getGrabFront());
        al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
        return;
    }

    if (tryActionCapSpinAttackImpl(true)) {
        if (mStateRunHakoniwa2D3D->isGroundSpin()) {
            const bool isClockwise = mStateRunHakoniwa2D3D->isSpinClockwise();
            if (isClockwise)
                mTrigger->set(PlayerTrigger::EActionTrigger_val28);
            else
                mTrigger->set(PlayerTrigger::EActionTrigger_val27);
        }
        if (mStateRunHakoniwa2D3D->isRunWaterSurface())
            mTrigger->set(PlayerTrigger::EActionTrigger_val33);
        al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
        return;
    }

    sead::Vector3f turnDirection{0.0f, 0.0f, 0.0f};
    if (mStateRunHakoniwa2D3D->tryTurnJump(mJudgePreInputJump, &turnDirection)) {
        rs::resetJudge(mJudgePreInputJump);
        mExternalVelocity->cancelAndFeedbackLastGroundInertia(this, mConst->getJumpInertiaRate(),
                                                              true);
        PlayerJumpMessageRequest* request = mJumpMessageRequest;
        request->jumpType = static_cast<PlayerJumpType>(16);
        request->turnJumpAngle = turnDirection;
        al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
        return;
    }

    if (rs::judgeAndResetReturnTrue(mJudgePreInputJump)) {
        mExternalVelocity->cancelAndFeedbackLastGroundInertia(this, mConst->getJumpInertiaRate(),
                                                              true);
        if (mStateRunHakoniwa2D3D->isGroundSpin()) {
            PlayerJumpMessageRequest* request = mJumpMessageRequest;
            const bool isClockwise = mStateRunHakoniwa2D3D->isSpinClockwise();
            request->jumpType = static_cast<PlayerJumpType>(12);
            request->isSpinClockwise = isClockwise;
        } else if (mStateRunHakoniwa2D3D->isRunDashFast()) {
            mJumpMessageRequest->jumpType = static_cast<PlayerJumpType>(3);
        } else {
            mTrigger->set(PlayerTrigger::EActionTrigger_val21);
        }

        if (mStateRunHakoniwa2D3D->isBrake2D()) {
            sead::Vector3f front{0.0f, 0.0f, 0.0f};
            sead::Vector3f up{0.0f, 0.0f, 0.0f};
            al::calcFrontDir(&front, this);
            al::calcUpDir(&up, this);
            sead::Quatf quat = sead::Quatf::unit;
            al::makeQuatFrontUp(&quat, -front, up);
            al::updatePoseQuat(this, quat);
            rs::resetCollisionPose(mCollider, quat);
        }
        al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeSpeedCheckFall) &&
        !mStateRunHakoniwa2D3D->isRunWaterSurface()) {
        mExternalVelocity->cancelAndFeedbackLastGroundInertia(this, mConst->getJumpInertiaRate(),
                                                              false);
        mJudgeWallCatchInputDir->validateFallJudge();
        mTrigger->set(PlayerTrigger::EActionTrigger_val14);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeStartSquat)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeInWater[0]) && !mStateRunHakoniwa2D3D->isRunWaterSurface()) {
        PlayerTrigger* trigger = mTrigger;
        if (mActionDiveInWater->isDiveInWaterAnim())
            trigger->set(PlayerTrigger::EActionTrigger_val9);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeWallHitDownForceRun)) {
        PlayerTrigger* trigger = mTrigger;
        PlayerEquipmentUser* equipmentUser = mEquipmentUser;
        PlayerCounterForceRun* counterForceRun = mCounterForceRun;
        trigger->set(PlayerTrigger::EActionTrigger_val10);
        if (equipmentUser->hasEquipment() && counterForceRun->getCounter() >= 1)
            equipmentUser->cancelEquip();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Damage);
        return;
    }

    const IUsePlayerCeilingCheck* ceilingCheck = mCollider;
    HackCap* hackCap = mHackCap;
    HackCapJudgePreInputHoveringJump* hoveringJudge = mHackCapJudgePreInputHoveringJump;
    if (ceilingCheck->isEnableStandUp() && rs::isPlayer2D(hackCap) &&
        hackCap->isEnableSpinAttack() && rs::isJudge(hoveringJudge)) {
        hoveringJudge = mHackCapJudgePreInputHoveringJump;
        PlayerJumpMessageRequest* request = mJumpMessageRequest;
        rs::resetJudge(hoveringJudge);
        hoveringJudge->setDisabled(true);
        request->jumpType = static_cast<PlayerJumpType>(18);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
    }
}

void PlayerActorHakoniwa::exeSlope() {
    tryActionSeparateCapThrow();
    tryActionCapReturn();
    if (al::updateNerveState(this)) {
        setNerveOnGround();
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeForceRolling)) {
        setNerveRollingFromGround(this, mTrigger, mCollider);
        return;
    }

    if (mStateSlope->isEnableCancelSandSink() && rs::updateJudgeAndResult(mJudgeSandSink)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SandSink);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeStartRolling)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Rolling);
        return;
    }

    if (rs::isCollidedGround(getPlayerCollision()) &&
        rs::judgeAndResetReturnTrue(mJudgePreInputJump)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeInWater[0])) {
        mTrigger->set(PlayerTrigger::EActionTrigger_StartSwim);
        PlayerTrigger* trigger = mTrigger;
        if (mActionDiveInWater->isDiveInWaterAnim())
            trigger->set(PlayerTrigger::EActionTrigger_DiveInWater);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeInvalidateInputFall)) {
        mInput->setupSceneStartFall();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
    }
}

void PlayerActorHakoniwa::exeRolling() {
    tryActionSeparateCapThrow();
    if (rs::isCollidedGround(mCollider)) {
        const char* materialCode = rs::getMaterialCodeGround(mCollider);
        mStainControl->recordSandMove(materialCode);
        mStainControl->recordSnowMove(materialCode);
        mCapActionHistory->clearLandLimitStandAngle();
    }
    if (mTrigger->isOn(PlayerTrigger::EMaterialChangeTrigger_val0))
        mWetControl->recordPuddleRolling();

    if (al::updateNerveState(this)) {
        if (mStateRolling->isEndStandUp())
            mTrigger->set(PlayerTrigger::EActionTrigger_val25);
        setNerveOnGround();
        return;
    }

    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeInvalidateInputFall)) {
        mInput->startSceneStartFall();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeSandSink)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SandSink);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
        getPlayerCollision();
        mStatePoleClimb->setup(mJudgePoleClimb->getCollidedParts(), mJudgePoleClimb->getPosition(),
                               mJudgePoleClimb->getUp(), mJudgePoleClimb->getFront(),
                               mJudgePoleClimb->getAngleOffsetWall(),
                               mJudgePoleClimb->getPoleHeight(),
                               mJudgePoleClimb->getCollisionCode());
        al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
        getPlayerCollision();
        mStateGrabCeil->setup(mJudgeGrabCeil->getCollidedParts(), mJudgeGrabCeil->getPosition(),
                              mJudgeGrabCeil->getUp(), mJudgeGrabCeil->getFront());
        al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
        return;
    }
    if (mStateRolling->isRollingJump() && tryActionCapSpinAttackImpl(true)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeInWater[0])) {
        PlayerTrigger* trigger = mTrigger;
        if (mActionDiveInWater->isDiveInWaterAnim())
            trigger->set(PlayerTrigger::EActionTrigger_val9);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeWallHitDownRolling)) {
        PlayerEquipmentUser* equipmentUser = mEquipmentUser;
        PlayerCounterForceRun* counterForceRun = mCounterForceRun;
        mTrigger->set(PlayerTrigger::EActionTrigger_val10);
        if (equipmentUser->hasEquipment() && counterForceRun->getCounter() >= 1)
            equipmentUser->cancelEquip();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Damage);
        return;
    }
    if (mStateRolling->isEndSquat())
        al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
}

// NON_MATCHING: exact 0x3F0 body; only wall-normal temporary instruction scheduling differs at
// target 0x423B34. Next hypothesis: recover the setup-argument/local lifetime that delays the final
// component negation/store.
void PlayerActorHakoniwa::exeSpinCap() {
    tryActionSeparateCapThrow();
    if (al::updateNerveState(this)) {
        setNerveOnGround();
        return;
    }

    PlayerStateSpinCap* stateSpinCap = mStateSpinCap;
    if (!stateSpinCap->isOnGround()) {
        mCapActionHistory->clearLandLimitStandAngle();
        stateSpinCap = mStateSpinCap;
    }
    if (stateSpinCap->isEnableCancelHipDrop() && rs::updateJudgeAndResult(mJudgeStartHipDrop)) {
        mStateSpinCap->cancelPoseInterpole();
        al::setNerve(this, &NrvPlayerActorHakoniwa.HipDrop);
        return;
    }
    if (mCarryKeeper->isCarry()) {
        setNerveOnGround();
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeInWater[0]) && mStateSpinCap->noticeInWater())
        mStateSwim->tryReactionWaterIn();

    if (mStateSpinCap->isEnableCancelAir()) {
        tryActionCapReturn();
        if (tryChangeNerveFromAir()) {
            if (al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch))
                mStateSpinCap->resetJoint();
            return;
        }
        if (rs::isLandGroundRunAngle(this, getPlayerCollision(), mConst)) {
            setNerveOnGround();
            return;
        }
        if (tryActionCapSpinAttackImpl(false))
            mSpinCapAttack->tryStartCapSpinAirMiss(mAnimator);
        return;
    }

    if (!mStateSpinCap->isEnableCancelGround())
        return;

    if (mStateSpinCap->isEnableReThrowCap() && tryActionCapSpinAttackImpl(true)) {
        if (mStateSpinCap->isWaterSurfaceRun())
            mTrigger->set(PlayerTrigger::EActionTrigger_val33);
        al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeWallCatchInputDir)) {
        PlayerJudgeWallCatchInputDir* judge = mJudgeWallCatchInputDir;
        const sead::Vector3f& normal = judge->getCollidedWallNormal();
        sead::Vector3f oppositeNormal = -normal;
        mStateWallCatch->setup(judge->getCollidedWallPart(), judge->getPosition(), oppositeNormal,
                               judge->getNormalAtPos());
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallCatch);
        return;
    }

    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeForceRolling)) {
        setNerveRollingFromGround(this, mTrigger, mCollider);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeSandSink)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SandSink);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
        getPlayerCollision();
        mStatePoleClimb->setup(mJudgePoleClimb->getCollidedParts(), mJudgePoleClimb->getPosition(),
                               mJudgePoleClimb->getUp(), mJudgePoleClimb->getFront(),
                               mJudgePoleClimb->getAngleOffsetWall(),
                               mJudgePoleClimb->getPoleHeight(),
                               mJudgePoleClimb->getCollisionCode());
        al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
        getPlayerCollision();
        mStateGrabCeil->setup(mJudgeGrabCeil->getCollidedParts(), mJudgeGrabCeil->getPosition(),
                              mJudgeGrabCeil->getUp(), mJudgeGrabCeil->getFront());
        al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
        return;
    }

    carryKeeper = mCarryKeeper;
    if (!rs::updateJudgeAndResult(mJudgeEnableStandUp)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeStartWaterSurfaceRun)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Run);
        return;
    }
    if (!mStateSpinCap->isWaterSurfaceRun() && rs::updateJudgeAndResult(mJudgeInWater[0])) {
        PlayerTrigger* trigger = mTrigger;
        if (mActionDiveInWater->isDiveInWaterAnim())
            trigger->set(PlayerTrigger::EActionTrigger_val9);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
        return;
    }
    if (rs::isOnGroundRunAngle(this, getPlayerCollision(), mConst) &&
        rs::judgeAndResetReturnTrue(mJudgePreInputJump))
        al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
}

// NON_MATCHING: exact 704-byte body; all recovered pointer lifetimes now match, with only the
// shared wall-normal temporary scheduling difference remaining. Next hypothesis: recover the
// setup-argument/local lifetime that delays the final component negation/store.
bool PlayerActorHakoniwa::tryChangeNerveFromAir() {
    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return true;
    }
    if (rs::updateJudgeAndResult(mJudgeInvalidateInputFall)) {
        mInput->startSceneStartFall();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        return true;
    }
    if (!al::isNerve(this, &NrvPlayerActorHakoniwa.SpinCap) && tryActionCapSpinAttackImpl(true)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
        return true;
    }
    if (!al::isNerve(this, &NrvPlayerActorHakoniwa.LongJump) &&
        rs::updateJudgeAndResult(mJudgeStartHipDrop)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.HipDrop);
        return true;
    }
    if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
        getPlayerCollision();
        mStatePoleClimb->setup(mJudgePoleClimb->getCollisionParts(),
                               mJudgePoleClimb->getContactPos(), mJudgePoleClimb->getPoleUp(),
                               mJudgePoleClimb->getPoleFront(), mJudgePoleClimb->getAngleOffset(),
                               mJudgePoleClimb->getStartHeight(), mJudgePoleClimb->getPoleCode());
        al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
        return true;
    }

    PlayerStateGrabCeil* stateGrabCeil = mStateGrabCeil;
    PlayerJudgeGrabCeil* judgeGrabCeil = mJudgeGrabCeil;
    if ((!al::isNerve(this, &NrvPlayerActorHakoniwa.GrabCeil) ||
         stateGrabCeil->isEnableNextGrabCeil()) &&
        rs::updateJudgeAndResult(judgeGrabCeil)) {
        getPlayerCollision();
        mStateGrabCeil->setup(mJudgeGrabCeil->getCollisionParts(), mJudgeGrabCeil->getContactPos(),
                              mJudgeGrabCeil->getGrabUp(), mJudgeGrabCeil->getGrabFront());
        al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
        return true;
    }

    if (al::isNerve(this, &NrvPlayerActorHakoniwa.LongJump) &&
        rs::updateJudgeAndResult(mJudgeWallHitDown)) {
        PlayerTrigger* trigger = mTrigger;
        PlayerEquipmentUser* equipmentUser = mEquipmentUser;
        PlayerCounterForceRun* counterForceRun = mCounterForceRun;
        trigger->set(PlayerTrigger::EActionTrigger_val10);
        if (equipmentUser->hasEquipment() && counterForceRun->getCounter() >= 1)
            equipmentUser->cancelEquip();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Damage);
        return true;
    }
    if (rs::updateJudgeAndResult(mJudgeWallCatch)) {
        const sead::Vector3f oppositeNormal = -mJudgeWallCatch->getCollidedWallNormal();
        mStateWallCatch->setup(mJudgeWallCatch->getCollidedWallPart(),
                               mJudgeWallCatch->getPosition(), oppositeNormal,
                               mJudgeWallCatch->getNormalAtPos());
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallCatch);
        return true;
    }
    if (!al::isNerve(this, &NrvPlayerActorHakoniwa.WallAir) &&
        rs::updateJudgeAndResult(mJudgeWallKeep)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallAir);
        return true;
    }
    if (rs::updateJudgeAndResult(mJudgeCapCatchPop)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.CapCatchPop);
        return true;
    }
    if (rs::updateJudgeAndResult(mJudgeStartWaterSurfaceRun)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Run);
        return true;
    }
    if (!rs::updateJudgeAndResult(mJudgeInWater[0]))
        return false;

    PlayerTrigger* trigger = mTrigger;
    if (mActionDiveInWater->isDiveInWaterAnim())
        trigger->set(PlayerTrigger::EActionTrigger_val9);
    al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
    return true;
}

bool PlayerActorHakoniwa::tryActionCapSpinAttackMiss() {
    return tryActionCapSpinAttackImpl(false);
}

// NON_MATCHING: exact 1096-byte body; remaining differences are the early jump-message request
// register choice and shared wall-normal temporary scheduling. Next hypothesis: recover the
// jump-request local lifetime first, then revisit the shared setup-argument scheduling.
void PlayerActorHakoniwa::exeJump() {
    if (al::isFirstStep(this)) {
        rs::resetJudge(mJudgeOutInWater);
        rs::resetJudge(mJudgePreInputJump);
    }

    tryActionCapReturn();
    const bool isFormSquat2D = mStateJump->isFormSquat2D();
    if (al::updateNerveState(this)) {
        mStateJump->tryCountUpContinuousJump(mContinuousJump);
        if (mStateJump->isEndJumpDownFallLand())
            mTrigger->set(PlayerTrigger::EActionTrigger_val11);
        setNerveOnGround();
        if (isFormSquat2D && rs::updateJudgeAndResult(mJudgeStartSquat)) {
            if (al::isNerve(this, &NrvPlayerActorHakoniwa.Run)) {
                al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
                return;
            }
            if (al::isNerve(this, &NrvPlayerActorHakoniwa.Jump)) {
                PlayerJumpMessageRequest* request = mJumpMessageRequest;
                const bool isEnableStandUp = rs::updateJudgeAndResult(mJudgeEnableStandUp);
                request->jumpType = static_cast<PlayerJumpType>(15);
                request->isEnableStandUp = isEnableStandUp;
            }
        }
        return;
    }

    if (mStateJump->isHovering())
        mHackCapJudgePreInputHoveringJump->setDisabled(true);
    if (mStateJump->isJumpBack() || mStateJump->isJumpSpinFlower())
        mTrigger->set(PlayerTrigger::EActionTrigger_val30);

    if (rs::updateJudgeAndResult(mJudgeInvalidateInputFall)) {
        mInput->startSceneStartFall();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        return;
    }
    if (tryActionCapSpinAttackImpl(true)) {
        if (mStateJump->isJumpSpinGround()) {
            const bool isClockwise = mStateJump->isJumpSpinGroundClockwise();
            if (isClockwise)
                mTrigger->set(PlayerTrigger::EActionTrigger_val28);
            else
                mTrigger->set(PlayerTrigger::EActionTrigger_val27);
        }
        al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
        return;
    }

    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return;
    }
    if (mStateJump->isEnableHipDropStart() && rs::updateJudgeAndResult(mJudgeStartHipDrop)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.HipDrop);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
        getPlayerCollision();
        mStatePoleClimb->setup(mJudgePoleClimb->getCollisionParts(),
                               mJudgePoleClimb->getContactPos(), mJudgePoleClimb->getPoleUp(),
                               mJudgePoleClimb->getPoleFront(), mJudgePoleClimb->getAngleOffset(),
                               mJudgePoleClimb->getStartHeight(), mJudgePoleClimb->getPoleCode());
        al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
        getPlayerCollision();
        mStateGrabCeil->setup(mJudgeGrabCeil->getCollisionParts(), mJudgeGrabCeil->getContactPos(),
                              mJudgeGrabCeil->getGrabUp(), mJudgeGrabCeil->getGrabFront());
        al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeWallCatch)) {
        const sead::Vector3f oppositeNormal = -mJudgeWallCatch->getCollidedWallNormal();
        mStateWallCatch->setup(mJudgeWallCatch->getCollidedWallPart(),
                               mJudgeWallCatch->getPosition(), oppositeNormal,
                               mJudgeWallCatch->getNormalAtPos());
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallCatch);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeWallKeep)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallAir);
        return;
    }
    if (!mStateJump->isJumpCapCatch() && rs::updateJudgeAndResult(mJudgeCapCatchPop)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.CapCatchPop);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeStartWaterSurfaceRun)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Run);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeOutInWater)) {
        if (mStateJump->isHoldDownFall())
            mTrigger->set(PlayerTrigger::EActionTrigger_val19);
        PlayerTrigger* trigger = mTrigger;
        if (mActionDiveInWater->isDiveInWaterAnim())
            trigger->set(PlayerTrigger::EActionTrigger_val9);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeWallHitDownForceRun)) {
        PlayerTrigger* trigger = mTrigger;
        PlayerEquipmentUser* equipmentUser = mEquipmentUser;
        PlayerCounterForceRun* counterForceRun = mCounterForceRun;
        trigger->set(PlayerTrigger::EActionTrigger_val10);
        if (equipmentUser->hasEquipment() && counterForceRun->getCounter() >= 1)
            equipmentUser->cancelEquip();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Damage);
        return;
    }
    if (mStateJump->isEnableCancelCarryThrow() && mCarryKeeper->isThrowRelease()) {
        sead::Vector3f* velocity = al::getVelocityPtr(this);
        al::verticalizeVec(velocity, al::getGravity(this), *velocity);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
    }
}

void PlayerActorHakoniwa::exeCapCatchPop() {
    if (al::isFirstStep(this))
        mCapActionHistory->invalidateCapCatchPop();

    if (al::updateNerveState(this)) {
        setNerveOnGround();
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeStartWaterSurfaceRun)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Run);
        return;
    }

    if (rs::isLandGroundRunAngle(this, getPlayerCollision(), mConst)) {
        setNerveOnGround();
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeInWater[0])) {
        PlayerTrigger* trigger = mTrigger;
        if (mActionDiveInWater->isDiveInWaterAnim())
            trigger->set(PlayerTrigger::EActionTrigger_DiveInWater);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
    }
}

// NON_MATCHING: exact 0x254 body; only wall-normal temporary instruction scheduling differs at
// target 0x424738. Passing the negated getter directly, and reconstructing pseudo's scalar Y/Z
// lifetimes, both move the first mismatch earlier to 0x424730. Next hypothesis: recover a source
// expression that preserves the explicit vector temporary while delaying only its final Z store.
void PlayerActorHakoniwa::exeWallAir() {
    if (al::isFirstStep(this))
        mCapActionHistory->clearWallAirLimit();

    tryActionCapReturn();
    if (al::updateNerveState(this)) {
        setNerveOnGround();
        return;
    }

    if (mStateWallAir->isAir()) {
        if (tryChangeNerveFromAir())
            return;
    } else {
        if (tryActionCapSpinAttackImpl(true)) {
            mInput->resetAlongWall();
            mTrigger->set(PlayerTrigger::EActionTrigger_val0);
            mStateWallAir->startSlideSpinAttack();
            al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
            return;
        }
        if (mStateWallAir->isJustJump()) {
            al::HitSensor* wallSensor = rs::tryGetCollidedWallSensor(mCollider);
            if (wallSensor)
                rs::sendMsgPlayerStartWallJump(wallSensor, mBodyHitSensor);
        }
    }

    if (rs::updateJudgeAndResult(mJudgeStartWaterSurfaceRun)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Run);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeInWater[0])) {
        PlayerTrigger* trigger = mTrigger;
        if (mActionDiveInWater->isDiveInWaterAnim())
            trigger->set(PlayerTrigger::EActionTrigger_val9);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
        getPlayerCollision();
        mStatePoleClimb->setup(mJudgePoleClimb->getCollidedParts(), mJudgePoleClimb->getPosition(),
                               mJudgePoleClimb->getUp(), mJudgePoleClimb->getFront(),
                               mJudgePoleClimb->getAngleOffsetWall(),
                               mJudgePoleClimb->getPoleHeight(),
                               mJudgePoleClimb->getCollisionCode());
        al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
        getPlayerCollision();
        mStateGrabCeil->setup(mJudgeGrabCeil->getCollidedParts(), mJudgeGrabCeil->getPosition(),
                              mJudgeGrabCeil->getUp(), mJudgeGrabCeil->getFront());
        al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeWallCatch)) {
        const sead::Vector3f& normal = mJudgeWallCatch->getCollidedWallNormal();
        sead::Vector3f oppositeNormal = -normal;
        mStateWallCatch->setup(mJudgeWallCatch->getCollidedWallPart(),
                               mJudgeWallCatch->getPosition(), oppositeNormal,
                               mJudgeWallCatch->getNormalAtPos());
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallCatch);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeInvalidateInputFall)) {
        mInput->startSceneStartFall();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
    }
}

// NON_MATCHING: exact 0x2CC body; only wall-recatch normal temporary scheduling differs. Next
// hypothesis: recover the setup-argument/local lifetime that delays the final component
// negation/store.
void PlayerActorHakoniwa::exeWallCatch() {
    if (al::isFirstStep(this))
        mCapActionHistory->clearLandLimit();

    tryActionSeparateCapThrow();
    bool isStateEnd = al::updateNerveState(this);
    PlayerStateWallCatch* stateWallCatch = mStateWallCatch;
    if (isStateEnd) {
        if (stateWallCatch->isFallEnd()) {
            mTrigger->set(PlayerTrigger::EActionTrigger_val32);
            rs::resetJudge(mJudgePreInputCapThrow);
        }
        setNerveOnGround();
        return;
    }

    if (stateWallCatch->isClimbJump()) {
        if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
            getPlayerCollision();
            mStatePoleClimb->setup(
                mJudgePoleClimb->getCollidedParts(), mJudgePoleClimb->getPosition(),
                mJudgePoleClimb->getUp(), mJudgePoleClimb->getFront(),
                mJudgePoleClimb->getAngleOffsetWall(), mJudgePoleClimb->getPoleHeight(),
                mJudgePoleClimb->getCollisionCode());
            al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
            return;
        }
        if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
            getPlayerCollision();
            mStateGrabCeil->setup(mJudgeGrabCeil->getCollidedParts(), mJudgeGrabCeil->getPosition(),
                                  mJudgeGrabCeil->getUp(), mJudgeGrabCeil->getFront());
            al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
            return;
        }
        if (mStateWallCatch->isClimbJumpFall()) {
            if (rs::updateJudgeAndResult(mJudgeWallCatch)) {
                const sead::Vector3f& normal = mJudgeWallCatch->getCollidedWallNormal();
                sead::Vector3f oppositeNormal = -normal;
                mStateWallCatch->setup(mJudgeWallCatch->getCollidedWallPart(),
                                       mJudgeWallCatch->getPosition(), oppositeNormal,
                                       mJudgeWallCatch->getNormalAtPos());
                al::setNerve(this, &NrvPlayerActorHakoniwa.WallCatch);
                return;
            }
            if (rs::updateJudgeAndResult(mJudgeWallKeep)) {
                al::setNerve(this, &NrvPlayerActorHakoniwa.WallAir);
                return;
            }
            if (mExternalVelocity->isExistForce()) {
                al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
                return;
            }
        }
    }

    if (!mStateWallCatch->isWallCatchForm()) {
        if (!rs::updateJudgeAndResult(mJudgeInWater[0]))
            return;
    } else if (rs::updateJudgeAndResult(mJudgeInWater[1])) {
        mStateWallCatch->endFallFromWall();
    } else {
        if (mExternalVelocity->isExistForce()) {
            PlayerStateWallCatch* stateWallCatchForce = mStateWallCatch;
            sead::Vector3f force = mExternalVelocity->getExternalForce();
            if (al::tryNormalizeOrZero(&force)) {
                sead::Vector3f wallFront = stateWallCatchForce->getWallCatchFront();
                f32 dot = force.x * wallFront.x + wallFront.y * force.y + wallFront.z * force.z;
                if (dot > -0.17365f) {
                    mStateWallCatch->endFallFromWall();
                    setNerveOnGround();
                    return;
                }
            }
            mExternalVelocity->reset();
        }
        PlayerPushReceiver* pushReceiver = mPushReceiver;
        sead::Vector3f wallFront = mStateWallCatch->getWallCatchFront();
        pushReceiver->cutPushVec(wallFront);
        return;
    }

    PlayerTrigger* trigger = mTrigger;
    if (mActionDiveInWater->isDiveInWaterAnim())
        trigger->set(PlayerTrigger::EActionTrigger_val9);
    al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
}

void PlayerActorHakoniwa::exeGrabCeil() {
    if (al::isFirstStep(this)) {
        mWallActionHistory->reset();
        mCapActionHistory->clearLandLimit();
    }

    tryActionSeparateCapThrow();
    if (al::updateNerveState(this))
        setNerveOnGround();
    else if (mStateGrabCeil->isJump())
        tryChangeNerveFromAir();
}

void PlayerActorHakoniwa::exePoleClimb() {
    if (al::isFirstStep(this)) {
        mWallActionHistory->reset();
        mCapActionHistory->clearLandLimit();
    }

    tryActionSeparateCapThrow();
    if (al::updateNerveState(this))
        setNerveOnGround();
    else if (mStatePoleClimb->isPoleJump())
        tryChangeNerveFromAir();
    else if (mStatePoleClimb->isForceFollowCap())
        mHackCap->requestForceFollowSeparateHide();
}

void PlayerActorHakoniwa::exeHipDrop() {
    if (al::isFirstStep(this))
        rs::resetJudge(mJudgeAirForceCount);
    if (rs::isCollidedGround(mCollider))
        mCapActionHistory->clearLandLimitStandAngle();

    if (mStateHipDrop->isLandTrigger()) {
        PlayerEquipmentUser* equipmentUser = mEquipmentUser;
        if (equipmentUser->hasEquipment() && mCounterForceRun->getCounter() >= 1)
            equipmentUser->cancelEquip();
        if (rs::isCollidedGround(mCollider)) {
            mStainControl->recordSandHeavyLand(rs::getMaterialCodeGround(mCollider));
            if (mTrigger->isOn(PlayerTrigger::EMaterialChangeTrigger_val0))
                mWetControl->recordHeavyLandPuddle();
        }
    }

    tryActionSeparateCapThrow();
    if (al::updateNerveState(this))
        setNerveOnGround();

    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeForceRolling)) {
        setNerveRollingFromGround(this, mTrigger, mCollider);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeSandSink)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SandSink);
        return;
    }

    carryKeeper = mCarryKeeper;
    if (!rs::updateJudgeAndResult(mJudgeEnableStandUp)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
        getPlayerCollision();
        mStateGrabCeil->setup(mJudgeGrabCeil->getCollidedParts(), mJudgeGrabCeil->getPosition(),
                              mJudgeGrabCeil->getUp(), mJudgeGrabCeil->getFront());
        al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeAirForceCount)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        return;
    }

    if (mStateHipDrop->isEnableLandCancel()) {
        if (rs::judgeAndResetReturnTrue(mJudgePreInputJump)) {
            mJumpMessageRequest->jumpType = PlayerJumpType::SwimJumpHipDrop;
            al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
            return;
        }
        if (mJudgeStartRolling->judgeCancelHipDrop()) {
            mTrigger->set(PlayerTrigger::EActionTrigger_val17);
            al::setNerve(this, &NrvPlayerActorHakoniwa.Rolling);
            return;
        }
        if (tryActionCapSpinAttackImpl(true)) {
            mTrigger->set(PlayerTrigger::EActionTrigger_val18);
            al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
            return;
        }
    }

    if (mStateHipDrop->isEnableMove()) {
        if (tryActionCapSpinAttackImpl(true)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
            return;
        }
        if (rs::updateJudgeAndResult(mJudgeStartRun)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Run);
            return;
        }
        if (rs::judgeAndResetReturnTrue(mJudgePreInputJump)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
            return;
        }
    }

    if (mStateHipDrop->isEnableHeadSliding() && !mCarryKeeper->isCarry()) {
        if (!rs::isOnGround(this, getPlayerCollision()) && mInput->isTriggerHeadSliding()) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.HeadSliding);
            return;
        }
    }

    if (mStateHipDrop->isEnableInWater() && rs::updateJudgeAndResult(mJudgeInWater[2])) {
        PlayerEquipmentUser* equipmentUser = mEquipmentUser;
        if (equipmentUser->hasEquipment() && mCounterForceRun->getCounter() >= 1)
            equipmentUser->cancelEquip();
        mCounterForceRun->reset();
        mTrigger->set(PlayerTrigger::EActionTrigger_val19);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
    }
}

void PlayerActorHakoniwa::exeHeadSliding() {
    if (al::isFirstStep(this))
        rs::resetJudge(mJudgeAirForceCount);
    if (rs::isCollidedGround(mCollider))
        mCapActionHistory->clearLandLimitStandAngle();

    tryActionSeparateCapThrow();
    if (al::updateNerveState(this)) {
        bool isDirectRolling = rs::updateJudgeAndResult(mJudgeDirectRolling);
        PlayerTrigger* trigger = mTrigger;
        if (isDirectRolling) {
            trigger->set(PlayerTrigger::EActionTrigger_Rolling);
            al::setNerve(this, &NrvPlayerActorHakoniwa.Rolling);
            return;
        }
        trigger->set(PlayerTrigger::EActionTrigger_val16);
        setNerveOnGround();
        return;
    }

    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeForceRolling)) {
        setNerveRollingFromGround(this, mTrigger, mCollider);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeSandSink)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SandSink);
        return;
    }

    carryKeeper = mCarryKeeper;
    if (!rs::updateJudgeAndResult(mJudgeEnableStandUp)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeInWater[0])) {
        bool isEnableDiveInWater = mStateHeadSliding->isEnableDiveInWater();
        PlayerTrigger* trigger = mTrigger;
        if (isEnableDiveInWater)
            trigger->set(PlayerTrigger::EActionTrigger_val9);
        else
            trigger->set(PlayerTrigger::EActionTrigger_val15);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeStartWaterSurfaceRun)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Run);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
        getPlayerCollision();
        mStatePoleClimb->setup(mJudgePoleClimb->getCollidedParts(), mJudgePoleClimb->getPosition(),
                               mJudgePoleClimb->getUp(), mJudgePoleClimb->getFront(),
                               mJudgePoleClimb->getAngleOffsetWall(),
                               mJudgePoleClimb->getPoleHeight(),
                               mJudgePoleClimb->getCollisionCode());
        al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
        getPlayerCollision();
        mStateGrabCeil->setup(mJudgeGrabCeil->getCollidedParts(), mJudgeGrabCeil->getPosition(),
                              mJudgeGrabCeil->getUp(), mJudgeGrabCeil->getFront());
        al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeWallHitDown)) {
        PlayerEquipmentUser* equipmentUser = mEquipmentUser;
        PlayerCounterForceRun* counterForceRun = mCounterForceRun;
        mTrigger->set(PlayerTrigger::EActionTrigger_val10);
        if (equipmentUser->hasEquipment() && counterForceRun->getCounter() >= 1)
            equipmentUser->cancelEquip();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Damage);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeAirForceCount))
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
}

void PlayerActorHakoniwa::exeLongJump() {
    tryActionCapReturn();
    if (al::updateNerveState(this)) {
        if (rs::updateJudgeAndResult(mJudgeDirectRolling)) {
            mTrigger->set(PlayerTrigger::EActionTrigger_Rolling);
            al::setNerve(this, &NrvPlayerActorHakoniwa.Rolling);
        } else {
            setNerveOnGround();
        }
    } else {
        tryChangeNerveFromAir();
    }
}

// NON_MATCHING: exact 1196-byte body; remaining executable difference is the shared wall-normal
// temporary scheduling used by the wall-catch setup. Next hypothesis: recover the setup-argument
// local lifetime that delays the final component negation/store.
void PlayerActorHakoniwa::exeFall() {
    al::updateNerveState(this);
    tryActionCapReturn();

    if (tryActionCapSpinAttackImpl(true)) {
        rs::isJustLand(mCollider);
        const bool isRunAngle = rs::isLandGroundRunAngle(this, mCollider, mConst);
        if (isRunAngle)
            mTrigger->set(PlayerTrigger::EActionTrigger_val1);
        else
            mTrigger->set(PlayerTrigger::EActionTrigger_val2);
        al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
        return;
    }

    if (mStateFallHakoniwa->isValidWallCatch() && mJudgeWallCatchInputDir->getTimeInAir() >= 1 &&
        rs::updateJudgeAndResult(mJudgeWallCatchInputDir)) {
        PlayerStateWallCatch* stateWallCatch = mStateWallCatch;
        const sead::Vector3f oppositeNormal = -mJudgeWallCatchInputDir->getCollidedWallNormal();
        stateWallCatch->setup(mJudgeWallCatchInputDir->getCollidedWallPart(),
                              mJudgeWallCatchInputDir->getPosition(), oppositeNormal,
                              mJudgeWallCatchInputDir->getNormalAtPos());
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallCatch);
        return;
    }

    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeForceRolling)) {
        setNerveRollingFromGround(this, mTrigger, mCollider);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeSandSink)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SandSink);
        return;
    }

    carryKeeper = mCarryKeeper;
    if (!rs::updateJudgeAndResult(mJudgeEnableStandUp)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
        return;
    }

    if (rs::isLandGroundRunAngle(this, getPlayerCollision(), mConst)) {
        if (mStateFallHakoniwa->isRunFall() && al::isFirstStep(this))
            mTrigger->set(PlayerTrigger::EActionTrigger_val24);
        setNerveOnGround();
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.Run) && !rs::isJustLand(mCollider)) {
            sead::Vector3f* velocity = al::getVelocityPtr(this);
            const sead::Vector3f& currentVelocity = al::getVelocity(this);
            const sead::Vector3f oppositeGravity = -al::getGravity(this);
            al::alongVectorNormalH(velocity, currentVelocity, oppositeGravity,
                                   rs::getCollidedGroundNormal(mCollider));
        }
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeStartHipDrop)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.HipDrop);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
        getPlayerCollision();
        mStatePoleClimb->setup(mJudgePoleClimb->getCollisionParts(),
                               mJudgePoleClimb->getContactPos(), mJudgePoleClimb->getPoleUp(),
                               mJudgePoleClimb->getPoleFront(), mJudgePoleClimb->getAngleOffset(),
                               mJudgePoleClimb->getStartHeight(), mJudgePoleClimb->getPoleCode());
        al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
        getPlayerCollision();
        mStateGrabCeil->setup(mJudgeGrabCeil->getCollisionParts(), mJudgeGrabCeil->getContactPos(),
                              mJudgeGrabCeil->getGrabUp(), mJudgeGrabCeil->getGrabFront());
        al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
        return;
    }

    if (!mStateFallHakoniwa->isInvalidInputFall() && !mStateFallHakoniwa->hasFallTargetArea() &&
        rs::updateJudgeAndResult(mJudgeWallKeep)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallAir);
        return;
    }
    if (mStateFallHakoniwa->isValidWallCatch() && rs::updateJudgeAndResult(mJudgeWallCatch)) {
        PlayerStateWallCatch* stateWallCatch = mStateWallCatch;
        const sead::Vector3f oppositeNormal = -mJudgeWallCatch->getCollidedWallNormal();
        stateWallCatch->setup(mJudgeWallCatch->getCollidedWallPart(),
                              mJudgeWallCatch->getPosition(), oppositeNormal,
                              mJudgeWallCatch->getNormalAtPos());
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallCatch);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeCapCatchPop)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.CapCatchPop);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeStartWaterSurfaceRun)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Run);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeInWater[0])) {
        PlayerTrigger* trigger = mTrigger;
        if (mActionDiveInWater->isDiveInWaterAnim())
            trigger->set(PlayerTrigger::EActionTrigger_val9);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
        return;
    }

    if (mStateFallHakoniwa->isInvalidInputFall() || mStateFallHakoniwa->hasFallTargetArea()) {
        mInput->startSceneStartFall();
        return;
    }

    const IUsePlayerCeilingCheck* ceilingCheck = mCollider;
    HackCap* hackCap = mHackCap;
    HackCapJudgePreInputHoveringJump* hoveringJudge = mHackCapJudgePreInputHoveringJump;
    if (ceilingCheck->isEnableStandUp() && rs::isPlayer2D(hackCap) &&
        hackCap->isEnableSpinAttack() && rs::isJudge(hoveringJudge)) {
        hoveringJudge = mHackCapJudgePreInputHoveringJump;
        PlayerJumpMessageRequest* request = mJumpMessageRequest;
        rs::resetJudge(hoveringJudge);
        hoveringJudge->setDisabled(true);
        request->jumpType = static_cast<PlayerJumpType>(18);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
    }
}

// NON_MATCHING: exact 0x240 body; only wall-normal temporary instruction scheduling differs at
// target 0x42588C. Next hypothesis: recover the setup-argument/local lifetime that delays the final
// component negation/store.
void PlayerActorHakoniwa::exeSandSink() {
    if (al::isFirstStep(this))
        mCapActionHistory->clearLandLimitStandAngle();

    PlayerStateSandSink* stateSandSink = mStateSandSink;
    stateSandSink->setSandSinkDeathRate(mSandSinkAffect->calcSandSinkDeathRate());

    bool isStateEnd = al::updateNerveState(this);
    bool isDeadStatus = mStateSandSink->isDeadStatus();
    if (isStateEnd) {
        if (isDeadStatus)
            al::setNerve(this, &NrvPlayerActorHakoniwa.Dead);
        else
            setNerveOnGround();
        return;
    }

    if (isDeadStatus && mCarryKeeper->isCarry())
        mCarryKeeper->startReleaseDead();

    if (mSandSinkAffect->isSink())
        mStainControl->recordSandMove("SandDesert");

    if (mStateSandSink->isJump()) {
        if (tryActionCapSpinAttackImpl(true)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
            return;
        }

        PlayerCarryKeeper* carryKeeper = mCarryKeeper;
        if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
            if (carryKeeper->isCarry())
                carryKeeper->startCancelAndRelease();
            al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
            return;
        }

        if (rs::updateJudgeAndResult(mJudgeForceRolling)) {
            setNerveRollingFromGround(this, mTrigger, mCollider);
            return;
        }

        if (rs::updateJudgeAndResult(mJudgeStartHipDrop)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.HipDrop);
            return;
        }

        if (rs::updateJudgeAndResult(mJudgeWallCatch)) {
            const sead::Vector3f& normal = mJudgeWallCatch->getCollidedWallNormal();
            sead::Vector3f oppositeNormal(-normal.x, -normal.y, -normal.z);
            mStateWallCatch->setup(mJudgeWallCatch->getCollidedWallPart(),
                                   mJudgeWallCatch->getPosition(), oppositeNormal,
                                   mJudgeWallCatch->getNormalAtPos());
            al::setNerve(this, &NrvPlayerActorHakoniwa.WallCatch);
            return;
        }

        if (rs::updateJudgeAndResult(mJudgeWallKeep)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.WallAir);
            return;
        }

        if (rs::updateJudgeAndResult(mJudgeCapCatchPop))
            al::setNerve(this, &NrvPlayerActorHakoniwa.CapCatchPop);
        return;
    }

    if (mStateSandSink->isDeadStatus())
        return;

    tryActionCapReturn();
    tryActionSeparateCapThrow();
    if (!rs::isOnGround(this, mCollider)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        return;
    }

    if (!rs::updateJudgeAndResult(mJudgeSandSink))
        setNerveOnGround();
}

void PlayerActorHakoniwa::exeSandGeyser() {
    if (al::isFirstStep(this))
        mAnimator->startAnim("Fall");

    mStainControl->recordSandHeavyLand("SandDesert");
    if (al::updateNerveState(this))
        setNerveOnGround();
}

// NON_MATCHING: exact 388-byte body; wall-normal temporary stores are scheduled before target
// argument loads. Component construction and a cached judge compile identically; copy-and-negate
// forms grow the body to 0x19C. Next hypothesis: recover surrounding setup-argument lifetimes
// while retaining the faithful direct temporary.
void PlayerActorHakoniwa::exeRise() {
    if (al::isFirstStep(this))
        mCapActionHistory->clearLandLimit();

    tryActionCapReturn();
    al::updateNerveState(this);
    if (rs::updateJudgeAndResult(mJudgeStartRise))
        return;

    if (tryActionCapSpinAttackImpl(true)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
        return;
    }

    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeStartHipDrop)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.HipDrop);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeWallCatch)) {
        const sead::Vector3f wallNormal = -mJudgeWallCatch->getCollidedWallNormal();
        mStateWallCatch->setup(mJudgeWallCatch->getCollidedWallPart(),
                               mJudgeWallCatch->getPosition(), wallNormal,
                               mJudgeWallCatch->getNormalAtPos());
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallCatch);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeWallKeep)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallAir);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeCapCatchPop)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.CapCatchPop);
        return;
    }

    if (rs::isOnGround(this, mCollider))
        setNerveOnGround();
}

void PlayerActorHakoniwa::exeSwim() {
    if (al::isFirstStep(this))
        mWallActionHistory->reset();
    mCapActionHistory->clearLandLimit();

    if (tryDamageIceWater(this, mDamageKeeper, mOxygen, mCounterIceWater, mConst, mTrigger,
                          mJudgeDeadWipeStart) &&
        PlayerFunction::isPlayerDeadStatus(this)) {
        if (mCarryKeeper->isCarry())
            mCarryKeeper->startReleaseDead();
        al::setNerve(this, &NrvPlayerActorHakoniwa.DamageSwim);
        return;
    }

    const bool isEnd = al::updateNerveState(this);
    if (isEnd) {
        if (mStateSwim->isEndSwimJump()) {
            mStateSwim->tryReactionWaterOut();
            al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
            return;
        }
        if (mStateSwim->isEndSwimJumpPop()) {
            const f32 verticalSpeed = al::calcSpeedV(this);
            const f32 speed = verticalSpeed < 0.0f ? 0.0f : verticalSpeed;
            const f32 candidatePower = speed + mConst->getSwimJumpHipDropPopJumpAdd();
            const f32 maxJumpPower = mConst->getJumpHipDropPower();
            const f32 jumpPower = candidatePower > maxJumpPower ? maxJumpPower : candidatePower;
            PlayerJumpMessageRequest* request = mJumpMessageRequest;
            request->jumpType = PlayerJumpType::SwimJumpPop;
            request->jumpPower = jumpPower;
            mStateSwim->tryReactionWaterOut();
            al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
            return;
        }
        if (mStateSwim->isEndSwimJumpHipDrop()) {
            mStateSwim->tryReactionWaterOut();
            mJumpMessageRequest->jumpType = PlayerJumpType::SwimJumpHipDrop;
            al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
            return;
        }
        setNerveOnGround();
        return;
    }

    if (mStateSwim->isSurface())
        mWetControl->recordWaterSurface();
    else
        mWetControl->recordInWater();

    if (!rs::updateJudgeAndResult(mJudgeInWater[3])) {
        mStateSwim->tryReactionWaterOut();
        setNerveOnGround();
        return;
    }

    if (PlayerEquipmentFunction::isEquipmentForceDash(mEquipmentUser))
        mEquipmentUser->cancelEquip();

    const bool isEnableCapThrowSurface = mStateSwim->isEnableCapThrowSurface();
    const bool isEnableCapThrow = mStateSwim->isEnableCapThrow();
    if (isEnableCapThrowSurface || isEnableCapThrow) {
        if (tryActionCapSpinAttackImpl(true)) {
            if (isEnableCapThrowSurface)
                mStateSwim->startCapThrowSurface();
            else
                mStateSwim->startCapThrow();
            return;
        }
    } else {
        tryActionSeparateCapThrow();
    }

    if (mStateSwim->isWaitGround() && rs::updateJudgeAndResult(mJudgeCameraSubjective)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Camera);
        return;
    }
    if (mStateSwim->isEnableWallHitDown() && rs::updateJudgeAndResult(mJudgeWallHitDown)) {
        mTrigger->set(PlayerTrigger::EActionTrigger_WallHitDown);
        al::setNerve(this, &NrvPlayerActorHakoniwa.DamageSwim);
    }
}

namespace {

bool tryDamageIceWater(const al::LiveActor* player, PlayerDamageKeeper* damageKeeper,
                       PlayerOxygen* oxygen, PlayerCounterIceWater* counterIceWater,
                       const PlayerConst* playerConst, PlayerTrigger* trigger,
                       PlayerJudgeDeadWipeStart* judgeDeadWipeStart) {
    if (counterIceWater->isTriggerDamage()) {
        al::startHitReaction(player, "氷水ダメージ");
        trigger->set(PlayerTrigger::EActionTrigger_IceWaterDamage);
    } else {
        if (!oxygen->isTriggerDamage())
            return false;
        al::startHitReaction(player, "酸素不足ダメージ");
        trigger->set(PlayerTrigger::EActionTrigger_NoOxygenDamage);
    }

    damageKeeper->damage(playerConst->getDamageInvalidCount());
    if (PlayerFunction::isPlayerDeadStatus(player)) {
        if (trigger->isOn(PlayerTrigger::EActionTrigger_IceWaterDamage))
            judgeDeadWipeStart->setDeathType(DeathType::IceWater);
        else if (trigger->isOn(PlayerTrigger::EActionTrigger_NoOxygenDamage))
            judgeDeadWipeStart->setDeathType(DeathType::NoOxygen);
    }
    return true;
}

}  // namespace

void PlayerActorHakoniwa::exeDamage() {
    if (al::isFirstStep(this)) {
        rs::resetJudge(mJudgeSpeedCheckFall);
        rs::resetJudge(mJudgeAirForceCount);
        mCapActionHistory->clearLandLimitStandAngle();
    }

    tryActionSeparateCapThrow();
    bool isStateEnd = al::updateNerveState(this);
    bool isDead = PlayerFunction::isPlayerDeadStatus(this);
    if (isStateEnd) {
        if (isDead)
            al::setNerve(this, &NrvPlayerActorHakoniwa.Dead);
        else
            setNerveOnGround();
        return;
    }
    if (isDead)
        return;

    if (rs::updateJudgeAndResult(mJudgeInWater[0])) {
        if (mStateDamageLife->isLand()) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
            return;
        }
        mTrigger->set(PlayerTrigger::EActionTrigger_val5);
        al::setNerve(this, &NrvPlayerActorHakoniwa.DamageSwim);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeStartWaterSurfaceRun)) {
        if (mStateDamageLife->isLand()) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Run);
            return;
        }
        mTrigger->set(PlayerTrigger::EActionTrigger_val5);
        al::setNerve(this, &NrvPlayerActorHakoniwa.DamageSwim);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeAirForceCount)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        return;
    }

    if (mStateDamageLife->isLand()) {
        PlayerCarryKeeper* carryKeeper = mCarryKeeper;
        if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
            if (carryKeeper->isCarry())
                carryKeeper->startCancelAndRelease();
            al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
            return;
        }

        if (rs::updateJudgeAndResult(mJudgeForceRolling)) {
            sead::Vector3f groundNormal = rs::getCollidedGroundNormal(mCollider);
            sead::Vector3f slideDir = {0.0f, 0.0f, 0.0f};
            if (!rs::calcSlideDir(&slideDir, al::getGravity(this), groundNormal))
                al::calcFrontDir(&slideDir, this);
            sead::Quatf quat = sead::Quatf::unit;
            al::makeQuatFrontUp(&quat, slideDir, groundNormal);
            al::updatePoseQuat(this, quat);
            setNerveRollingFromGround(this, mTrigger, mCollider);
            return;
        }

        carryKeeper = mCarryKeeper;
        if (!rs::updateJudgeAndResult(mJudgeEnableStandUp)) {
            if (carryKeeper->isCarry())
                carryKeeper->startCancelAndRelease();
            mTrigger->set(PlayerTrigger::EActionTrigger_val12);
            al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
            return;
        }

        if (rs::updateJudgeAndResult(mJudgeSpeedCheckFall)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
            return;
        }
        if (rs::updateJudgeAndResult(mJudgeSandSink)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.SandSink);
            return;
        }
        if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
            getPlayerCollision();
            mStatePoleClimb->setup(
                mJudgePoleClimb->getCollidedParts(), mJudgePoleClimb->getPosition(),
                mJudgePoleClimb->getUp(), mJudgePoleClimb->getFront(),
                mJudgePoleClimb->getAngleOffsetWall(), mJudgePoleClimb->getPoleHeight(),
                mJudgePoleClimb->getCollisionCode());
            al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
            return;
        }
        if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
            getPlayerCollision();
            mStateGrabCeil->setup(mJudgeGrabCeil->getCollidedParts(), mJudgeGrabCeil->getPosition(),
                                  mJudgeGrabCeil->getUp(), mJudgeGrabCeil->getFront());
            al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
            return;
        }
    }

    if (mStateDamageLife->isEnableCancel() && rs::judgeAndResetReturnTrue(mJudgePreInputJump))
        al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
}

void PlayerActorHakoniwa::exeDamageSwim() {
    if (al::isFirstStep(this)) {
        mCapActionHistory->clearLandLimit();
        mWallActionHistory->reset();
    }

    mWetControl->recordInWater();
    tryActionSeparateCapThrow();
    if (mStateDamageSwim->isNoDamageDown() &&
        tryDamageIceWater(this, mDamageKeeper, mOxygen, mCounterIceWater, mConst, mTrigger,
                          mJudgeDeadWipeStart) &&
        PlayerFunction::isPlayerDeadStatus(this)) {
        if (mCarryKeeper->isCarry())
            mCarryKeeper->startReleaseDead();
        al::setNerve(this, &NrvPlayerActorHakoniwa.DamageSwim);
        return;
    }

    const bool isEnd = al::updateNerveState(this);
    const bool isDead = PlayerFunction::isPlayerDeadStatus(this);
    if (isEnd) {
        if (isDead) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Dead);
            return;
        }
        if (mStateDamageSwim->isEndGround()) {
            mTrigger->set(PlayerTrigger::EActionTrigger_val6);
            al::setNerve(this, &NrvPlayerActorHakoniwa.Damage);
            return;
        }
        if (mStateDamageSwim->isEndInWater())
            mTrigger->set(PlayerTrigger::EActionTrigger_DamageSwim);
        setNerveOnGround();
        return;
    }

    if (!isDead && mStateDamageSwim->isEnableCancel() && mInput->isTriggerPaddle()) {
        mTrigger->set(PlayerTrigger::EActionTrigger_DamageSwimCancel);
        mTrigger->set(PlayerTrigger::EActionTrigger_DamageSwim);
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
    }
}

void PlayerActorHakoniwa::exeDamageFire() {
    bool isDead = PlayerFunction::isPlayerDeadStatus(this);
    if (al::isFirstStep(this)) {
        rs::resetJudge(mJudgeOutInWater);
        if (isDead)
            mStainControl->recordDamageFireDead();
    }

    tryActionSeparateCapThrow();
    if (rs::isCollidedGround(mCollider))
        mCapActionHistory->clearLandLimitStandAngle();

    bool isStateEnd = al::updateNerveState(this);
    if (isDead && isStateEnd) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Dead);
        return;
    }
    if (isDead)
        return;

    mStainControl->recordDamageFire();
    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeSandSink)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SandSink);
        return;
    }

    if (!mTrigger->isOn(PlayerTrigger::EActionTrigger_val13)) {
        carryKeeper = mCarryKeeper;
        if (!rs::updateJudgeAndResult(mJudgeEnableStandUp)) {
            if (carryKeeper->isCarry())
                carryKeeper->startCancelAndRelease();
            rs::cutVerticalVelocityGroundNormal(this, getPlayerCollision());
            al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
            return;
        }
    }

    if (al::isGreaterEqualStep(this, mStateDamageFire->getEnableCancelCollisionSnapFrame())) {
        mTrigger->set(PlayerTrigger::EActionTrigger_val31);
        if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
            getPlayerCollision();
            mStatePoleClimb->setup(
                mJudgePoleClimb->getCollidedParts(), mJudgePoleClimb->getPosition(),
                mJudgePoleClimb->getUp(), mJudgePoleClimb->getFront(),
                mJudgePoleClimb->getAngleOffsetWall(), mJudgePoleClimb->getPoleHeight(),
                mJudgePoleClimb->getCollisionCode());
            al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
            return;
        }
        if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
            getPlayerCollision();
            mStateGrabCeil->setup(mJudgeGrabCeil->getCollidedParts(), mJudgeGrabCeil->getPosition(),
                                  mJudgeGrabCeil->getUp(), mJudgeGrabCeil->getFront());
            al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
            return;
        }
    }

    if (isStateEnd) {
        if (rs::updateJudgeAndResult(mJudgeSpeedCheckFall))
            al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        else
            al::setNerve(this, &NrvPlayerActorHakoniwa.Run);
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeOutInWater)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
        return;
    }

    if (mStateDamageFire->isEndFirstLand()) {
        if (tryActionCapSpinAttackImpl(true)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
            return;
        }
        if (rs::isCollidedGround(mCollider) &&
            mTrigger->isOn(PlayerTrigger::EMaterialChangeTrigger_val0)) {
            al::startHitReaction(this, "アチチ走り浅瀬キャンセル");
            setNerveOnGround();
            return;
        }
    }

    if (mStateDamageFire->isEnableJump() && rs::judgeAndResetReturnTrue(mJudgePreInputJump))
        al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
}

void PlayerActorHakoniwa::exePress() {
    if (al::isFirstStep(this))
        mSeparateCapFlag->setPress(true);

    if (al::updateNerveState(this))
        al::setNerve(this, &NrvPlayerActorHakoniwa.Dead);
}

void PlayerActorHakoniwa::exeHack() {
    if (al::isFirstStep(this)) {
        mWallActionHistory->reset();
        if (mGaugeAir->isAlive()) {
            mGaugeAir->kill();
            rs::recoveryPlayerOxygen(this);
            mGaugeAir->setRate(mOxygen->getRate());
        }
    }

    if (al::updateNerveState(this) || !mStateHack->isEnableChangeState())
        return;

    mHackCap->updateCapEyeShowHide(rs::isJudge(mJudgeRecoveryLifeFast),
                                   mConst->getCapManHeroEyesWaitAppearFrame());
    const u32 separateCapFlags = mSeparateCapFlag->getRawFlags();
    if ((separateCapFlags & 0xFF0000) == 0 && (separateCapFlags & 0xFF) != 0 &&
        rs::judgeAndResetReturnTrue(mHackCapJudgePreInputSeparateThrow)) {
        bool isReaction = false;
        if (mHackCap->isEnableHackThrow(&isReaction)) {
            sead::Vector3f up = {0.0f, 0.0f, 0.0f};
            if (!PlayerCapFunction::tryCalcHackCapThrowInputNormal(&up, this))
                up = -al::getGravity(this);

            sead::Vector3f direction = {0.0f, 0.0f, 0.0f};
            mInput->calcCapSeparateMoveInput(&direction, up);
            if (!al::tryNormalizeOrZero(&direction) && !rs::calcAlongDirFront(&direction, this, up))
                al::calcFrontDir(&direction, this);
            mHackCap->startThrowSeparatePlayHack(mBodyHitSensor, direction, up, 1.0f);
        } else if (isReaction) {
            al::startHitReaction(this, "帽子が戻せない");
        }
    }

    const sead::Vector3f& trans = al::getTrans(this);
    const sead::Vector3f up = -al::getGravity(this);
    mWaterSurfaceFinder->update(trans, up, 200.0f);
    mCollider->updateHeightCheck(trans, up, false);
    if (rs::updateJudgeAndResult(mJudgeInWater[0]) &&
        (!mHackKeeper->sendMarioInWater() || !mHackKeeper->getHackSensor()))
        return;

    if (mStateHack->isEnableCancelHack()) {
        if (mAreaChecker->isInHackCancel(al::getTrans(this)) && mHackKeeper->cancelHackArea())
            return;

        if (HackFunction::isTriggerHackEnd(this)) {
            bool canEscape = false;
            if (mHackKeeper->isHackCancelCeilingCheck()) {
                sead::Vector3f followTrans = {0.0f, 0.0f, 0.0f};
                mHackCap->calcHackFollowTrans(&followTrans, true);
                PlayerColliderHakoniwa* collider = mCollider;
                const sead::Vector3f ceilingUp = -al::getGravity(this);
                collider->updateCeilingCheck(followTrans, ceilingUp, 50.0f, 0.0f);
                canEscape = mCollider->isEnableStandUp();
            } else {
                canEscape = !mHackKeeper->sendMsgSelfCeilingCheckMiss();
            }

            if (canEscape) {
                if (mHackKeeper->tryEscapeHack())
                    return;
            } else {
                al::startHitReaction(this, "ひょうい解除失敗");
                return;
            }
        }
    }

    WorldEndBorderKeeper* worldEndBorderKeeper = mWorldEndBorderKeeper;
    const sead::Vector3f previousTrans = al::getTrans(this) - al::getVelocity(this);
    worldEndBorderKeeper->update(previousTrans, al::getVelocity(this),
                                 mWaterSurfaceFinder->isFoundSurface() ||
                                     !rs::isPlayerCollidedGround(this));
    if (mWorldEndBorderKeeper->getVelocity().length() > 0.0f) {
        mHackKeeper->pushWorldEndBorder(mWorldEndBorderKeeper->getVelocity());
        al::copyPose(this, mHackKeeper->getHack());
    }
}

// NON_MATCHING: exact 600-byte body; remaining executable difference is the shared wall-normal
// temporary scheduling. Next hypothesis: recover the setup-argument/local lifetime that delays the
// final component negation/store.
void PlayerActorHakoniwa::exeEndHack() {
    if (al::isFirstStep(this))
        mCapActionHistory->invalidateCapJump();

    if (al::updateNerveState(this)) {
        setNerveOnGround();
        return;
    }

    if (rs::updateJudgeAndResult(mJudgeInvalidateInputFall)) {
        mInput->startSceneStartFall();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        return;
    }

    tryActionCapReturn();
    if (al::isGreaterEqualStep(this, 20) && tryActionCapSpinAttackImpl(true)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
        return;
    }

    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeStartHipDrop)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.HipDrop);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
        getPlayerCollision();
        mStatePoleClimb->setup(mJudgePoleClimb->getCollisionParts(),
                               mJudgePoleClimb->getContactPos(), mJudgePoleClimb->getPoleUp(),
                               mJudgePoleClimb->getPoleFront(), mJudgePoleClimb->getAngleOffset(),
                               mJudgePoleClimb->getStartHeight(), mJudgePoleClimb->getPoleCode());
        al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
        getPlayerCollision();
        mStateGrabCeil->setup(mJudgeGrabCeil->getCollisionParts(), mJudgeGrabCeil->getContactPos(),
                              mJudgeGrabCeil->getGrabUp(), mJudgeGrabCeil->getGrabFront());
        al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeWallCatch)) {
        const sead::Vector3f oppositeNormal = -mJudgeWallCatch->getCollidedWallNormal();
        mStateWallCatch->setup(mJudgeWallCatch->getCollidedWallPart(),
                               mJudgeWallCatch->getPosition(), oppositeNormal,
                               mJudgeWallCatch->getNormalAtPos());
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallCatch);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeWallKeep)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallAir);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeCapCatchPop)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.CapCatchPop);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeInWater[0]))
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
}

// NON_MATCHING: exact 876-byte body; remaining executable difference is the shared wall-normal
// temporary scheduling. Next hypothesis: recover the setup-argument/local lifetime that delays the
// final component negation/store.
void PlayerActorHakoniwa::exeBind() {
    if (al::isFirstStep(this)) {
        mInput->startBind();
        mCapActionHistory->clearLandLimit();
        mWallActionHistory->reset();
    }

    if (mPuppet->isBindSeparateCapEnabled())
        tryActionSeparateCapThrow();

    al::updateNerveState(this);
    if (mStateBind->isEndCapThrow()) {
        const bool isSpin = tryActionCapSpinAttackBindEnd();
        al::addVelocityToGravity(this, mConst->getGravityAir());
        updateCollider();
        if (!isSpin) {
            if (rs::isCollidedGround(mCollider))
                mSpinCapAttack->tryStartCapSpinGroundMiss(mAnimator);
            else
                mSpinCapAttack->tryStartCapSpinAirMiss(mAnimator);
            setNerveOnGround();
            return;
        }
        al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
        return;
    }

    if (mStateBind->isEndOnGround()) {
        setNerveOnGround();
        return;
    }
    if (!mStateBind->isEndAir())
        return;

    const bool isInvalidInput = mStateBind->isInvalidInput();
    tryActionCapReturn();

    PlayerCarryKeeper* carryKeeper = mCarryKeeper;
    if (rs::updateJudgeAndResult(mJudgeForceSlopeSlide)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Slope);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeInvalidateInputFall)) {
        mInput->startSceneStartFall();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        return;
    }

    carryKeeper = mCarryKeeper;
    if (!rs::updateJudgeAndResult(mJudgeEnableStandUp)) {
        if (carryKeeper->isCarry())
            carryKeeper->startCancelAndRelease();
        al::setNerve(this, &NrvPlayerActorHakoniwa.Squat);
        return;
    }
    if (!isInvalidInput && tryActionCapSpinAttackImpl(true)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.SpinCap);
        return;
    }
    if (rs::isLandGroundRunAngle(this, getPlayerCollision(), mConst)) {
        setNerveOnGround();
        return;
    }
    if (!isInvalidInput && rs::updateJudgeAndResult(mJudgeStartHipDrop)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.HipDrop);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeWallKeep)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallAir);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgePoleClimb)) {
        getPlayerCollision();
        mStatePoleClimb->setup(mJudgePoleClimb->getCollisionParts(),
                               mJudgePoleClimb->getContactPos(), mJudgePoleClimb->getPoleUp(),
                               mJudgePoleClimb->getPoleFront(), mJudgePoleClimb->getAngleOffset(),
                               mJudgePoleClimb->getStartHeight(), mJudgePoleClimb->getPoleCode());
        al::setNerve(this, &NrvPlayerActorHakoniwa.PoleClimb);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeGrabCeil)) {
        getPlayerCollision();
        mStateGrabCeil->setup(mJudgeGrabCeil->getCollisionParts(), mJudgeGrabCeil->getContactPos(),
                              mJudgeGrabCeil->getGrabUp(), mJudgeGrabCeil->getGrabFront());
        al::setNerve(this, &NrvPlayerActorHakoniwa.GrabCeil);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeWallCatch)) {
        const sead::Vector3f oppositeNormal = -mJudgeWallCatch->getCollidedWallNormal();
        mStateWallCatch->setup(mJudgeWallCatch->getCollidedWallPart(),
                               mJudgeWallCatch->getPosition(), oppositeNormal,
                               mJudgeWallCatch->getNormalAtPos());
        al::setNerve(this, &NrvPlayerActorHakoniwa.WallCatch);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeCapCatchPop)) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.CapCatchPop);
        return;
    }
    if (rs::updateJudgeAndResult(mJudgeInWater[0]))
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
}

bool PlayerActorHakoniwa::tryActionCapSpinAttackBindEnd() {
    if (PlayerEquipmentFunction::isEquipmentNoCapThrow(mEquipmentUser))
        return false;

    const GameDataHolderAccessor accessor(this);
    if (!GameDataFunction::isEnableCap(accessor) || mCarryKeeper->isCarry() || rs::is2D(this))
        return false;

    const u32 flags = mSeparateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) == 0 && (flags & 0xFF) != 0)
        return false;

    if (mHackCap->isRequestableReturn() || !mHackCap->isEnableSpinAttack())
        return false;

    mJudgePreInputCapThrow->recordJudgeAndReset();
    mCapActionHistory->recordLimitHeight();
    return true;
}

void PlayerActorHakoniwa::exeDemo() {}

void PlayerActorHakoniwa::exeCamera() {
    if (al::isFirstStep(this))
        rs::resetJudge(mJudgeSpeedCheckFall);

    bool shouldEnd = al::updateNerveState(this);
    shouldEnd |= mExternalVelocity->isExistForce();
    bool isDamage;
    if (mCounterIceWater->isTriggerDamage())
        isDamage = tryDamageIceWater(this, mDamageKeeper, mOxygen, mCounterIceWater, mConst,
                                     mTrigger, mJudgeDeadWipeStart);
    else
        isDamage = false;
    shouldEnd |= isDamage;
    shouldEnd |= rs::isSeparatePlay(this);

    if (shouldEnd | !mJudgeCameraSubjective->isEnableKeepSubjectiveCamera()) {
        if (rs::updateJudgeAndResult(mJudgeInWater[0])) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
            return;
        }
        if (rs::isCollidedGround(mCollider)) {
            al::setNerve(this, &NrvPlayerActorHakoniwa.Wait);
            return;
        }
        al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
        return;
    }

    if (!rs::updateJudgeAndResult(mJudgeSpeedCheckFall))
        return;
    if (rs::updateJudgeAndResult(mJudgeInWater[0])) {
        al::setNerve(this, &NrvPlayerActorHakoniwa.Swim);
        return;
    }
    al::setNerve(this, &NrvPlayerActorHakoniwa.Fall);
}

void PlayerActorHakoniwa::exeAbyss() {
    if (al::isFirstStep(this) && rs::isShowCapMsgPlayerFallDead(this))
        mJudgeDeadWipeStart->setDeathType(DeathType::AbyssWithCapMsg);

    if (al::updateNerveState(this)) {
        mDamageKeeper->invalidateIncludePush(mConst->getDamageInvalidCountAbyss());
        setNerveOnGround();
    }
}

void PlayerActorHakoniwa::exeDead() {}

bool PlayerActorHakoniwa::checkDeathArea() {
    if (PlayerFunction::isPlayerDeadStatus(this))
        return false;
    if (al::isNerve(this, &NrvPlayerActorHakoniwa.Demo))
        return false;
    if (al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss) && !mStateAbyss->isRecoveryLandFall())
        return false;
    if (mHackKeeper->getHackSensor() && mHackKeeper->isHackDemoStarted())
        return false;

    sead::Vector3f recoveryPos = {0.0f, 0.0f, 0.0f};
    sead::Vector3f recoveryNormal = {0.0f, 0.0f, 0.0f};
    const al::AreaObj* recoveryArea = nullptr;
    bool shouldRecover = false;
    PlayerDamageKeeper* damageKeeper;
    PlayerConst* playerConst;

    if (mAreaChecker->isInForceRecovery(&recoveryPos, &recoveryNormal, &recoveryArea,
                                        al::getTrans(this))) {
        mRecoverySafetyPoint->setRecoveryArea(recoveryArea);
        mRecoverySafetyPoint->setSafetyPoint(recoveryPos, recoveryNormal, recoveryArea);
        damageKeeper = mDamageKeeper;
        playerConst = mConst;
        if (mRecoverySafetyPoint->isValid() && !PlayerFunction::isPlayerHitPointOne(this))
            shouldRecover = true;
        else
            mRecoverySafetyPoint->setRecoveryArea(nullptr);
    }

    if (!shouldRecover) {
        if (!al::isInDeathArea(this))
            return false;
        if (mHackKeeper->getHackSensor() && !mHackKeeper->sendMarioDeathArea())
            return false;

        damageKeeper = mDamageKeeper;
        playerConst = mConst;
        if (mRecoverySafetyPoint->isValid() && !PlayerFunction::isPlayerHitPointOne(this)) {
            shouldRecover = true;
        } else {
            mDamageKeeper->dead();
            if (mBindKeeper->getBindSensor())
                mBindKeeper->cancelBind();
            if (mRecoverySafetyPoint->isValid())
                mRecoverySafetyPoint->reset();
            if (mHackCap->isCatched())
                rs::showCapMsgPlayerFallDead(this, 15);
            mJudgeDeadWipeStart->setDeathType(DeathType::Abyss);
            al::setNerve(this, &NrvPlayerActorHakoniwa.Abyss);
            return true;
        }
    }

    damageKeeper->damageForce(playerConst->getDamageInvalidCountRecovery());
    al::startHitReaction(this, "泡復帰ダメージ");
    startRecoveryFromDeathArea(this, mHackCap, mCarryKeeper, mBindKeeper, mEquipmentUser,
                               mStateAbyss);
    return true;
}

namespace {

void startRecoveryFromDeathArea(al::LiveActor* player, HackCap* hackCap,
                                PlayerCarryKeeper* carryKeeper,
                                PlayerBindKeeper* bindKeeper,
                                PlayerEquipmentUser* equipmentUser,
                                PlayerStateAbyss* stateAbyss) {
    if (hackCap->isEnableRescuePlayer())
        hackCap->startRescuePlayer();
    if (bindKeeper->getBindSensor())
        bindKeeper->cancelBind();
    if (carryKeeper->isCarry())
        carryKeeper->startReleaseDead();
    if (equipmentUser->hasEquipment() &&
        !PlayerEquipmentFunction::isEquipmentNoCapThrow(equipmentUser))
        equipmentUser->cancelEquip();
    stateAbyss->prepareRecovery();
    al::setNerve(player, &NrvPlayerActorHakoniwa.Abyss);
}

}  // namespace

void PlayerActorHakoniwa::sendCollisionMsg() {
    mTrigger->clearCollisionTrigger();
    mJumpMessageRequest->clear();
    if (mBindKeeper->getBindSensor() || al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss) ||
        PlayerFunction::isPlayerDeadStatus(this))
        return;

    if (!mHackKeeper->getHackSensor()) {
        rs::sendPlayerCollisionTouchMsg(this, mBodyHitSensor, mCollider);

        if (al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch) &&
            mStateWallCatch->isWallCatchForm()) {
            al::HitSensor* wallSensor = rs::tryGetCollidedWallSensor(mCollider);
            if (wallSensor)
                al::sendMsgPlayerFloorTouch(wallSensor, mBodyHitSensor);
        }

        if (al::isNerve(this, &NrvPlayerActorHakoniwa.PoleClimb) &&
            mStatePoleClimb->isAttachPole()) {
            al::HitSensor* poleSensor = mStatePoleClimb->getPoleSensor();
            if (poleSensor) {
                al::sendMsgPlayerFloorTouch(poleSensor, mBodyHitSensor);
                rs::sendMsgPlayerPoleClimbKeep(poleSensor, mBodyHitSensor);
                if (mStatePoleClimb->isTriggerReaction())
                    rs::sendMsgPlayerPoleClimbReaction(poleSensor, mBodyHitSensor);
            }
        }

        if (rs::sendPlayerCollisionUpperPunchMsg(this, mBodyHitSensor, mCollider, mModelChanger))
            mTrigger->set(PlayerTrigger::ECollisionTrigger_val0);

        PlayerStateHipDrop* stateHipDrop = mStateHipDrop;
        PlayerStateSwim* stateSwim = mStateSwim;
        if (mStateJump->isEnableTrampleByHipDropAttack() || stateHipDrop->isEnableHipDropAttack() ||
            stateSwim->isEnableHipDropAttack()) {
            const s32 numGrounds = rs::getGroundHitInfoNum(mCollider);
            for (s32 i = 0; i < numGrounds; i++) {
                if (rs::sendMsgPlayerHipDropDemoTrigger(rs::getGroundHitInfoSensor(mCollider, i),
                                                        mBodyHitSensor)) {
                    mInput->startHipDropDemoTrigger();
                } else if (rs::sendMsgPlayerHipDropHipDropSwitch(
                               rs::getGroundHitInfoSensor(mCollider, i), mBodyHitSensor)) {
                    mInput->startHipDropSwitch();
                } else if (al::sendMsgPlayerHipDrop(rs::getGroundHitInfoSensor(mCollider, i),
                                                    mBodyHitSensor, mComboCounter)) {
                    mTrigger->set(PlayerTrigger::ECollisionTrigger_val1);
                }
            }
        }

        PlayerJudgeWallHitDownRolling* wallHitDownRolling = mJudgeWallHitDownRolling;
        PlayerJudgeWallHitDown* wallHitDown = mJudgeWallHitDown;
        PlayerJudgeWallHitDownForceRun* wallHitDownForceRun = mJudgeWallHitDownForceRun;
        bool isWallHitDown = false;
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.Rolling)) {
            isWallHitDown = rs::updateJudgeAndResult(wallHitDownRolling);
        } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.HeadSliding) ||
                   al::isNerve(this, &NrvPlayerActorHakoniwa.LongJump)) {
            isWallHitDown = rs::updateJudgeAndResult(wallHitDown);
        } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Run) ||
                   al::isNerve(this, &NrvPlayerActorHakoniwa.Jump)) {
            isWallHitDown = rs::updateJudgeAndResult(wallHitDownForceRun);
        }

        if (isWallHitDown) {
            al::HitSensor* wallSensor = rs::tryGetCollidedWallSensor(mCollider);
            if (wallSensor) {
                if (rs::sendMsgPlayerRollingWallHitMove(wallSensor, mBodyHitSensor))
                    mTrigger->set(PlayerTrigger::ECollisionTrigger_val9);
                else
                    rs::sendMsgPlayerRollingWallHitDown(wallSensor, mBodyHitSensor);
            }
        }

        if (mTrigger->isOn(PlayerTrigger::EActionTrigger_val22)) {
            al::HitSensor* groundSensor = mCollider->getGroundSensor();
            if (groundSensor && mCollider->getNoGroundFrames() <= 5)
                rs::sendMsgPlayerJumpTakeOffFloor(groundSensor, mBodyHitSensor);
        }
        return;
    }

    if (mHackKeeper->isHackNoCollisionMsg())
        return;

    al::LiveActor* hackActor = mHackKeeper->getHack();
    if (mHackKeeper->isHackUsePlayerCollision()) {
        rs::sendPlayerCollisionTouchMsg(hackActor, mHackKeeper->getHackSensor(),
                                        mHackKeeper->getPlayerCollision());
    } else if (al::isExistActorCollider(hackActor)) {
        rs::sendPlayerCollisionTouchMsg(hackActor, mHackKeeper->getHackSensor(), nullptr);
    }
}

// NON_MATCHING: current is 0xff4 versus target 0xff8 after recovering the corpus-pseudo cached
// PlayerJumpMessageRequest and nested Hack/Abyss tail control flow. The remaining structural gap is
// the force-dash zero initialization: target keeps forceRunFrames at SP+0x10 and forceRunSpeed at
// SP+0x2C and emits two STRs, while current colors speed at SP+0xC and fuses them into one STP. A
// named GameDataHolderAccessor recovers 0xff8 but collapses the target 0x80 frame to 0x70. Moving
// the f32 lifetime earlier only swaps the adjacent SP+0xC/SP+0x10 colors. Next hypothesis: recover
// the source lifetime that places the accessor at target SP+8 without allowing it to share the
// SafeString slot, which should free SP+0x2C for forceRunSpeed.
void PlayerActorHakoniwa::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isSensorPlayerEye(self)) {
        if (mEyeSensorHitHolder->isEnableRecordLookAt(other, self)) {
            mEyeSensorHitHolder->tryRecordLookAtTarget(other, self);
            if (!al::isSensorLookAt(other)) {
                mEyeSensorHitHolder->tryRecordPriorityTarget(this, other, self);
                if (mEyeSensorHitHolder->tryRecordHomingAttack(other, self))
                    mEyeSensorHitHolder->tryRecordTargetMarker(other, self);
            }
        }
        return;
    }

    if (al::isSensorName(self, "SpinAttack")) {
        if (!PlayerFunction::isPlayerDeadStatus(this) &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss))
            mSpinCapAttack->attackSpinMsg(self, other);
        return;
    }

    if (al::isSensorName(self, "HipDropKnockDown")) {
        if (!PlayerFunction::isPlayerDeadStatus(this) &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss) &&
            !mStateHipDrop->attackHipDropKnockDown(self, other) &&
            !mStateWait->attackHipDropKnockDown(self, other))
            mStateSwim->attackHipDropKnockDown(self, other);
        return;
    }

    if (al::isSensorName(self, "Carry")) {
        PlayerInput* input = mInput;
        PlayerBindKeeper* bindKeeper = mBindKeeper;
        PlayerStateWallAir* stateWallAir = mStateWallAir;
        PlayerStateSwim* stateSwim = mStateSwim;
        PlayerStatePoleClimb* statePoleClimb = mStatePoleClimb;
        PlayerStateSpinCap* stateSpinCap = mStateSpinCap;
        if (!mModelChanger->is2DModel() &&
            !isCarryActionInvalid(this, bindKeeper, stateWallAir, stateSwim, statePoleClimb) &&
            (!al::isNerve(this, &NrvPlayerActorHakoniwa.SpinCap) ||
             stateSpinCap->isEnableCancelCarry()) &&
            input->isTriggerCarryStart()) {
            PlayerCarryKeeper* carryKeeper = mCarryKeeper;
            if (!carryKeeper->isCarry() && carryKeeper->startCarry(other)) {
                al::startHitReaction(this, "ものつかみ開始");
                mJointControlKeeper->noticeCarryStart();
                mStateWait->noticeCarryStart();
            }
        }
        return;
    }

    PlayerBindKeeper* bindKeeperRabbit = mBindKeeper;
    if (al::isSensorPlayer(self) &&
        (!isPlayerActionInvalid(this, bindKeeperRabbit) || bindKeeperRabbit->getBindSensor()) &&
        rs::sendMsgPlayerRabbitGet(other, self)) {
        if (!mModelChanger->is2DModel() && !mAnimator->isSubAnimPlaying())
            mAnimator->startSubAnim("RabbitGet");
        return;
    }

    if (!PlayerFunction::isPlayerDeadStatus(this) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss) && !al::isSensorPlayerFoot(self) &&
        al::isSensorPlayer(self)) {
        if (mModelChanger->is2DModel()) {
            if (rs::sendMsgPlayerItemGetAll2D(other, self))
                return;
        } else if (rs::sendMsgPlayerItemGetAll(other, self)) {
            return;
        }

        if (!GameDataFunction::isEnableCap(GameDataHolderAccessor(this)) &&
            rs::sendMsgPlayerCapRecovery(other, self)) {
            CapFunction::putOnCapPlayer(static_cast<HackCap*>(mHackCap), mAnimator);
            return;
        }
    }

    PlayerStateSwim* stateSwimTrample = mStateSwim;
    PlayerStatePoleClimb* statePoleClimbTrample = mStatePoleClimb;
    PlayerStateGrabCeil* stateGrabCeil = mStateGrabCeil;
    PlayerStateRolling* stateRollingTrample = mStateRolling;
    PlayerStateWallCatch* stateWallCatchTrample = mStateWallCatch;
    PlayerBindKeeper* bindKeeperTrample = mBindKeeper;
    if (al::isSensorPlayerFoot(self) && !isPlayerActionInvalid(this, bindKeeperTrample) &&
        (!al::isNerve(this, &NrvPlayerActorHakoniwa.Swim) || stateSwimTrample->isEnableTrample()) &&
        (!al::isNerve(this, &NrvPlayerActorHakoniwa.PoleClimb) ||
         statePoleClimbTrample->isEnableTrample()) &&
        (!al::isNerve(this, &NrvPlayerActorHakoniwa.GrabCeil) ||
         stateGrabCeil->isEnableTrample()) &&
        (!al::isNerve(this, &NrvPlayerActorHakoniwa.Rolling) ||
         stateRollingTrample->isEnableTrample(other, self)) &&
        (!al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch) ||
         stateWallCatchTrample->isEnableTrample()) &&
        rs::isEnableSendTrampleMsg(this, self, other)) {
        if (!mModelChanger->is2DModel() && rs::sendMsgPlayerObjLeapFrog(other, self)) {
            mTrigger->set(PlayerTrigger::EAttackSensorTrigger_val1);
            PlayerStateSwim* stateSwimLeapFrog = mStateSwim;
            if (!PlayerFunction::isPlayerDeadStatus(this) &&
                (!al::isNerve(this, &NrvPlayerActorHakoniwa.Swim) ||
                 stateSwimLeapFrog->isSurface())) {
                mJumpMessageRequest->jumpType = static_cast<PlayerJumpType>(7);
                al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
                return;
            }
        } else {
            PlayerStateHipDrop* stateHipDropAttack = mStateHipDrop;
            PlayerStateSwim* stateSwimHipDropAttack = mStateSwim;
            if (mStateJump->isEnableTrampleByHipDropAttack() ||
                stateHipDropAttack->isEnableHipDropAttack() ||
                stateSwimHipDropAttack->isEnableHipDropAttack()) {
                bool reflected = al::sendMsgPlayerObjHipDropReflect(other, self, mComboCounter);
                if (!reflected && rs::sendMsgPlayerCapHipDrop(other, self)) {
                    mCapActionHistory->invalidateCapJump();
                    mHackCap->recordCapJump(mWallActionHistory);
                    reflected = true;
                }
                if (reflected) {
                    mTrigger->set(PlayerTrigger::EAttackSensorTrigger_val3);
                    PlayerStateSwim* stateSwimAfterHipDrop = mStateSwim;
                    if (!PlayerFunction::isPlayerDeadStatus(this) &&
                        (!al::isNerve(this, &NrvPlayerActorHakoniwa.Swim) ||
                         stateSwimAfterHipDrop->isSurface())) {
                        mJumpMessageRequest->jumpType = static_cast<PlayerJumpType>(6);
                        al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
                        return;
                    }
                }
                if (al::sendMsgPlayerObjHipDrop(other, self, mComboCounter)) {
                    mTrigger->set(PlayerTrigger::EAttackSensorTrigger_val3);
                    return;
                }
            } else {
                bool capTrample = false;
                bool trampleSuccess =
                    mModelChanger->is2DModel() ?
                        rs::sendMsgPlayerTrample2D(other, self) :
                        al::sendMsgPlayerReflectOrTrample(other, self, mComboCounter);
                PlayerStateSwim* stateSwimCapTrample = mStateSwim;
                PlayerStateSpinCap* stateSpinCapTrample = mStateSpinCap;
                PlayerColliderHakoniwa* colliderCapTrample = mCollider;
                if (!trampleSuccess &&
                    (!al::isNerve(this, &NrvPlayerActorHakoniwa.Swim) ||
                     stateSwimCapTrample->isEnableTrample()) &&
                    (!al::isNerve(this, &NrvPlayerActorHakoniwa.SpinCap) ||
                     !stateSpinCapTrample->isSpinAttackAir()) &&
                    (!al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch) ||
                     colliderCapTrample->isEnableStandUp()) &&
                    rs::sendMsgPlayerCapTrample(other, self)) {
                    mCapActionHistory->invalidateCapJump();
                    mHackCap->recordCapJump(mWallActionHistory);
                    capTrample = true;
                    trampleSuccess = true;
                    mTrigger->set(PlayerTrigger::EAttackSensorTrigger_val1);
                    mTrigger->set(PlayerTrigger::EAttackSensorTrigger_val2);
                }

                if (trampleSuccess) {
                    if (!capTrample)
                        mTrigger->set(PlayerTrigger::EAttackSensorTrigger_val1);
                    PlayerStateSwim* stateSwimAfterTrample = mStateSwim;
                    if (!PlayerFunction::isPlayerDeadStatus(this) &&
                        (!al::isNerve(this, &NrvPlayerActorHakoniwa.Swim) ||
                         stateSwimAfterTrample->isSurface())) {
                        const bool isHeadSliding =
                            al::isNerve(this, &NrvPlayerActorHakoniwa.HeadSliding);
                        PlayerJumpMessageRequest* jumpRequest = mJumpMessageRequest;
                        if (isHeadSliding) {
                            const sead::Vector3f& trans = al::getTrans(this);
                            jumpRequest->jumpType = static_cast<PlayerJumpType>(10);
                            jumpRequest->actorTrans = trans;
                        } else if (capTrample) {
                            jumpRequest->jumpType = static_cast<PlayerJumpType>(11);
                        } else {
                            jumpRequest->jumpType = static_cast<PlayerJumpType>(4);
                        }
                        mHackCapJudgePreInputHoveringJump->setDisabled(false);
                        al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
                        return;
                    }
                }
            }
        }
    }

    if (!al::isSensorPlayerFoot(self) && al::isSensorPlayer(self)) {
        PlayerStateSwim* stateSwimTouch = mStateSwim;
        PlayerStateSpinCap* stateSpinCapTouch = mStateSpinCap;
        PlayerColliderHakoniwa* colliderTouch = mCollider;
        HackCap* hackCapTouch = mHackCap;
        if (!isPlayerActionInvalid(this, mBindKeeper)) {
            bool enableTouchJump = true;
            if (al::isNerve(this, &NrvPlayerActorHakoniwa.Swim)) {
                enableTouchJump = hackCapTouch->isForceCapTouchJump() ?
                                      stateSwimTouch->isEnableTrample() :
                                      stateSwimTouch->isEnableCapTouchJump();
            }
            if (enableTouchJump &&
                (!al::isNerve(this, &NrvPlayerActorHakoniwa.SpinCap) ||
                 !stateSpinCapTouch->isSpinAttackAir()) &&
                !al::isNerve(this, &NrvPlayerActorHakoniwa.Slope) &&
                ((!al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch) &&
                  !rs::isCollidedGround(colliderTouch)) ||
                 colliderTouch->isEnableStandUp()) &&
                rs::sendMsgPlayerCapTouchJump(other, self)) {
                mTrigger->set(PlayerTrigger::EAttackSensorTrigger_val1);
                mTrigger->set(PlayerTrigger::EAttackSensorTrigger_val2);
                if (!rs::isOnGroundLessAngle(this, mCollider, mConst->getStandAngleMin()))
                    mCapActionHistory->invalidateCapJump();
                mHackCap->recordCapJump(mWallActionHistory);

                PlayerStateSwim* stateSwimAfterCapTouch = mStateSwim;
                if (PlayerFunction::isPlayerDeadStatus(this) ||
                    (al::isNerve(this, &NrvPlayerActorHakoniwa.Swim) &&
                     !stateSwimAfterCapTouch->isSurface()))
                    return;
                if (al::isNerve(this, &NrvPlayerActorHakoniwa.PoleClimb))
                    mStatePoleClimb->updateLeavePoleTrans();

                PlayerJumpMessageRequest* jumpRequest;
                const sead::Vector3f* actorTrans;
                PlayerJumpType jumpType;
                if (rs::isCollidedGround(mCollider)) {
                    jumpRequest = mJumpMessageRequest;
                    actorTrans = &al::getTrans(this);
                    jumpType = static_cast<PlayerJumpType>(8);
                } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.HeadSliding)) {
                    jumpRequest = mJumpMessageRequest;
                    actorTrans = &al::getTrans(this);
                    jumpType = static_cast<PlayerJumpType>(10);
                } else {
                    const bool forceCapTouchJump = mHackCap->isForceCapTouchJump();
                    jumpRequest = mJumpMessageRequest;
                    if (!forceCapTouchJump) {
                        jumpRequest->jumpType = static_cast<PlayerJumpType>(11);
                        al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
                        return;
                    }
                    actorTrans = &al::getTrans(this);
                    jumpType = static_cast<PlayerJumpType>(9);
                }
                jumpRequest->jumpType = jumpType;
                jumpRequest->actorTrans = *actorTrans;
                al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
                return;
            }
        }
        if (!al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) &&
            rs::sendMsgPlayerCapCatch(other, self)) {
            const bool enableReaction =
                isEnableReactionCapCatch(this, mCarryKeeper, mModelChanger, mStateWait, mStateJump,
                                         mStateWallAir, mStateSwim);
            PlayerTrigger* trigger = mTrigger;
            if (enableReaction) {
                HackCap* hackCap = mHackCap;
                PlayerAnimator* animator = mAnimator;
                PlayerColliderHakoniwa* collider = mCollider;
                PlayerConst* playerConst = mConst;
                const sead::Vector3f& sensorPos = al::getSensorPos(other);
                const char* animName;
                if (animator->isSubAnimPlaying())
                    animator->endSubAnim();
                if (al::isNerve(this, &NrvPlayerActorHakoniwa.Wait)) {
                    animName = "CatchCap";
                    animator->startSubAnim("CatchCap");
                } else if (rs::isOnGroundRunAngle(this, collider, playerConst)) {
                    animName = "CatchCapParts";
                } else {
                    animName = "CatchCapJumpParts";
                }
                animator->startUpperBodyAnimAndHeadVisKeep(animName);
                hackCap->startCatch(animName, true, sensorPos);
                trigger->set(PlayerTrigger::EAttackSensorTrigger_val0);
                return;
            }
            trigger->set(PlayerTrigger::EAttackSensorTrigger_val0);
            CapFunction::putOnCapPlayer(static_cast<HackCap*>(mHackCap), mAnimator);
            return;
        }

        if (rs::sendMsgPlayerCapPush(other, self))
            return;
    }

    if (!isPlayerActionInvalid(this, mBindKeeper) && al::isSensorPlayerFoot(self) &&
        al::isNerve(this, &NrvPlayerActorHakoniwa.Slope) &&
        al::sendMsgPlayerSlidingAttack(other, self, nullptr))
        return;

    PlayerStateRolling* stateRollingObjectHit = mStateRolling;
    if (!isPlayerActionInvalid(this, mBindKeeper) && !al::isSensorPlayerFoot(self) &&
        al::isSensorPlayer(self) && al::isNerve(this, &NrvPlayerActorHakoniwa.Rolling) &&
        stateRollingObjectHit->isRolling() && rs::sendMsgPlayerRollingObjHit(other, self))
        return;

    if (al::isSensorPlayer(self)) {
        PlayerInput* input = mInput;
        PlayerBindKeeper* bindKeeper = mBindKeeper;
        PlayerStateWallAir* stateWallAir = mStateWallAir;
        PlayerStateSwim* stateSwim = mStateSwim;
        PlayerStatePoleClimb* statePoleClimb = mStatePoleClimb;
        PlayerStateSpinCap* stateSpinCap = mStateSpinCap;
        if (!mModelChanger->is2DModel() &&
            !isCarryActionInvalid(this, bindKeeper, stateWallAir, stateSwim, statePoleClimb) &&
            (!al::isNerve(this, &NrvPlayerActorHakoniwa.SpinCap) ||
             stateSpinCap->isEnableCancelCarry()) &&
            input->isEnableCarry()) {
            PlayerCarryKeeper* carryKeeper = mCarryKeeper;
            if (!carryKeeper->isCarry() && carryKeeper->startCarry(other)) {
                al::startHitReaction(this, "ものつかみ開始");
                mJointControlKeeper->noticeCarryStart();
                mStateWait->noticeCarryStart();
                return;
            }
        }
    }

    if (!al::isSensorPlayerFoot(self) && al::isSensorPlayer(self) &&
        (al::isNerve(this, &NrvPlayerActorHakoniwa.Rolling) ||
         al::isNerve(this, &NrvPlayerActorHakoniwa.LongJump) ||
         al::isNerve(this, &NrvPlayerActorHakoniwa.HeadSliding)) &&
        rs::sendMsgPlayerObjectWallHit(other, self))
        return;

    if (!mEquipmentUser->hasEquipment() && !isPlayerActionInvalid(this, mBindKeeper) &&
        !al::isSensorPlayerFoot(self) && al::isSensorPlayer(self) &&
        al::sendMsgPlayerPutOnEquipment(other, self)) {
        const PlayerEquipmentUser* equipmentUser = mEquipmentUser;
        PlayerCounterForceRun* counterForceRun = mCounterForceRun;
        s32 forceRunFrames = 0;
        f32 forceRunSpeed = 0.0f;
        if (PlayerEquipmentFunction::tryGetEquipmentForceDashInfo(&forceRunFrames, &forceRunSpeed,
                                                                  equipmentUser)) {
            if (counterForceRun->getCounter() <= 0)
                al::startHitReaction(this, "強制走り開始");
            counterForceRun->setupForceRun(forceRunFrames, forceRunSpeed);
        }
        return;
    }

    PlayerBindKeeper* bindKeeperUpperPunch = mBindKeeper;
    PlayerFormSensorCollisionArranger* sensorArranger = mFormSensorCollisionArranger;
    PlayerStateRolling* stateRollingUpperPunch = mStateRolling;
    if (!al::isSensorPlayerFoot(self) && al::isSensorPlayer(self) &&
        al::isSensorName(self, sensorArranger->getHeadSensorName()) &&
        !isPlayerActionInvalid(this, bindKeeperUpperPunch) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Squat) &&
        (!al::isNerve(this, &NrvPlayerActorHakoniwa.Rolling) ||
         stateRollingUpperPunch->isRollingJump()) &&
        rs::isEnableSendUpperPunchMsg(this, self, other)) {
        if (!mModelChanger->is2DModel() && rs::sendMsgPlayerBallToss(other, self)) {
            PlayerAnimator* animator = mAnimator;
            if (!animator->isSubAnimPlaying() && !animator->isUpperBodyAnimAttached()) {
                const bool isGround = rs::isOnGround(this, mCollider);
                PlayerAnimator* animPlayer = mAnimator;
                if (isGround)
                    animPlayer->startSubAnim("BallTossGround");
                else
                    animPlayer->startSubAnimOnlyAir("BallToss");
            }
            mTrigger->set(PlayerTrigger::EAttackSensorTrigger_val4);
        }
        const sead::Vector3f& gravity = al::getGravity(this);
        const sead::Vector3f& velocity = al::getVelocity(this);
        if (gravity.dot(velocity) < 0.0f) {
            const bool punched = mModelChanger->is2DModel() ?
                                     rs::sendMsgPlayerObjUpperPunch2D(other, self) :
                                     al::sendMsgPlayerObjUpperPunch(other, self);
            if (punched)
                mTrigger->set(PlayerTrigger::EAttackSensorTrigger_val4);
        }
    }

    PlayerBindKeeper* bindKeeperKick = mBindKeeper;
    if (al::isSensorPlayerFoot(self) && !isPlayerActionInvalid(this, bindKeeperKick) &&
        al::sendMsgPlayerKick(other, self)) {
        if (!mModelChanger->is2DModel()) {
            PlayerAnimator* animator = mAnimator;
            if (!animator->isSubAnimPlaying())
                animator->startSubAnim("Kick");
        }
        return;
    }

    PlayerHackKeeper* hackKeeper = mHackKeeper;
    if (!PlayerFunction::isPlayerDeadStatus(this) &&
        !al::isNerve(this, &NrvPlayerActorHakoniwa.Demo)) {
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.Hack)) {
            if (hackKeeper->getHackSensor())
                return;
        } else if (al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss)) {
            return;
        }
        if ((!al::isSensorPlayerFoot(self) && al::isSensorPlayer(self)) ||
            al::isSensorPlayerFoot(self))
            mBindKeeper->collectBindableSensor(self, other);
    }
}

// NON_MATCHING: exact 0xBA0-byte body; after restoring the target unordered floating-point rejection and
// jump-request store order, the only aligned difference is three commutative FMUL operand
// reversals. Scalar-first vector multiplication and an explicit scaled-vector constructor still
// canonicalize to the current operand order. Next hypothesis: recover the original escape-scale
// lifetime/producer that keeps S8 as the first FMUL operand without scalarizing the vector.
bool PlayerActorHakoniwa::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                                     al::HitSensor* self) {
    bool jumpResult = false;
    if (al::isSensorPlayerAttack(self))
        return false;

    if (mBindKeeper->receiveEndMsg(message)) {
        mInput->startBind();
        mStateBind->endBind();
        return true;
    }

    if (rs::isMsgTransferHack(message))
        return mHackKeeper->sendTransferHack();

    if (rs::isMsgInitHack(message)) {
        mStateHack->prepareStartHack(other, self);
        mEffect->suspendInvincibleEffect();
        if (mCarryKeeper->isCarry())
            mCarryKeeper->startCancelAndRelease();
        mComboCounter->reset();
        mStainControl->noticeStartHack();
        mSandSinkAffect->clear();
        return true;
    }

    const HackEndParam* endParam;
    if (rs::isMsgEndHack(&endParam, message)) {
        al::updatePoseQuat(this, endParam->quat);
        al::resetPosition(this, endParam->targetPos);
        mModelChanger->resetPosition();
        mCollider->prepareCollisionEndHack();
        mStateHack->prepareEndHack();
        mEffect->restartInvincibleEffect();
        mStainControl->noticeEndHack();
        al::setVelocityZero(this);
        updateCollider();
        rs::resetCollision(mCollider);

        if (!PlayerFunction::isPlayerDeadStatus(this)) {
            if (endParam->isDamage) {
                al::setVelocityZero(this);
                PlayerTrigger* trigger = mTrigger;
                if (endParam->hasVelocity) {
                    al::setVelocity(this, endParam->vel);
                    trigger->set(PlayerTrigger::EReceiveSensorTrigger_val3);
                } else {
                    trigger->set(PlayerTrigger::EReceiveSensorTrigger_val2);
                }
            } else if (endParam->hasVelocity) {
                f32 velocityLength = 0.0f;
                sead::Vector3f velocityDirection = {0.0f, 0.0f, 0.0f};
                al::separateScalarAndDirection(&velocityLength, &velocityDirection, endParam->vel);
                mStateEndHack->setEndVelocity(endParam->vel, endParam->delayFrames);
            } else {
                sead::Vector3f horizontal = {0.0f, 0.0f, 0.0f};
                if (!al::isParallelDirection(al::getGravity(this), endParam->dir)) {
                    al::verticalizeVec(&horizontal, al::getGravity(this), endParam->dir);
                    al::tryNormalizeOrZero(&horizontal);
                }

                const sead::Vector3f horizontalVelocity = horizontal * 5.0f;
                const f32 escapeScale = endParam->escapeScale;
                const sead::Vector3f velocity =
                    horizontalVelocity * escapeScale - (al::getGravity(this) * 20.0f) * escapeScale;
                mStateEndHack->setEndVelocity(velocity, endParam->delayFrames);
            }
        }
        return true;
    }

    if (mExternalVelocity->receiveMsgPlayer(message, other, self)) {
        PlayerBindKeeper* bindKeeper = mBindKeeper;
        if (PlayerFunction::isPlayerDeadStatus(this) || bindKeeper->getBindSensor() ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss)) {
            mExternalVelocity->reset();
            return false;
        }
        if (rs::isMsgObjSnapForce(message) &&
            !isEnableCollisionSnap(this, mStateSpinCap, mStateGrabCeil, mStateDamageFire) &&
            (!al::isNerve(this, &NrvPlayerActorHakoniwa.DamageFire) ||
             !al::isGreaterEqualStep(this,
                                     mStateDamageFire->getEnableCancelCollisionSnapFrame()))) {
            mExternalVelocity->resetSnapForce();
            return false;
        }
        return true;
    }

    if (receivePushMsg(message, other, self, mConst->getPushPower()))
        return true;

    if (al::isMsgHit(message) || al::isMsgHitStrong(message) || al::isMsgHitVeryStrong(message) ||
        rs::isMsgPlayerDamageBlowDown(message) || rs::isMsgCapObjHipDropReflect(message)) {
        if (PlayerFunction::isPlayerDeadStatus(this) ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss) ||
            (al::isNerve(this, &NrvPlayerActorHakoniwa.EndHack) && al::isLessStep(this, 20)) ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.Damage))
            return false;

        sead::Vector3f pushDir = {0.0f, 0.0f, 0.0f};
        if (mModelChanger->is2DModel() || al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch) ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.PoleClimb)) {
            rs::calcFrontVerticalGravity(&pushDir, this);
        } else {
            al::calcDirBetweenSensorsNormal(&pushDir, self, other, al::getGravity(this));
        }
        if (al::isNearZero(pushDir))
            rs::calcFrontVerticalGravity(&pushDir, this);

        if (al::isMsgHit(message) || rs::isMsgCapObjHipDropReflect(message))
            mHitPush->setHitPush(pushDir);
        if (al::isMsgHitStrong(message))
            mHitPush->setHitPushStrong(pushDir);
        if (al::isMsgHitVeryStrong(message))
            mHitPush->setHitPushVeryStrong(pushDir);
        if (rs::isMsgPlayerDamageBlowDown(message)) {
            if (mBindKeeper->getBindSensor())
                mBindKeeper->cancelBind();
            if (!mDamageKeeper->isDamageInvalid()) {
                sead::Vector3f blowDir = {0.0f, 0.0f, 0.0f};
                al::calcDirOnPlane(&blowDir, al::getSensorPos(self),
                                   al::getTrans(al::getSensorHost(other)), al::getGravity(this));
                mHitPush->setHitPushBlowDown(blowDir);
            }
        }
        return true;
    }

    do {
        if (rs::isMsgPlayerJumpRequestAll(message)) {
            if (isPlayerActionInvalid(this, mBindKeeper))
                return false;

            PlayerStateSwim* stateSwim = mStateSwim;
            if (PlayerFunction::isPlayerDeadStatus(this) ||
                (al::isNerve(this, &NrvPlayerActorHakoniwa.Swim) && !stateSwim->isSurface()))
                return false;

            f32 power = 0.0f;
            const bool isJumpRequest = rs::tryGetRequestPlayerJumpInfo(&power, message);
            if (isJumpRequest) {
                jumpResult = isJumpRequest;
                PlayerJumpMessageRequest* jumpRequest = mJumpMessageRequest;
                jumpRequest->jumpPower = power;
                jumpRequest->jumpType = static_cast<PlayerJumpType>(jumpResult);
                jumpRequest->extendFrame = 0;
                al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
                break;
            }
            if (rs::tryGetRequestPlayerTrampleJumpInfo(&power, message)) {
                PlayerJumpMessageRequest* jumpRequest = mJumpMessageRequest;
                jumpRequest->jumpPower = power;
                jumpRequest->jumpType = static_cast<PlayerJumpType>(5);
                al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
                return true;
            }
            if (rs::tryGetRequestPlayerSpinJumpInfo(&power, message)) {
                PlayerJumpMessageRequest* jumpRequest = mJumpMessageRequest;
                jumpRequest->jumpPower = power;
                jumpRequest->jumpType = static_cast<PlayerJumpType>(13);
                al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
                return true;
            }
            if (rs::isMsgRequestPlayerJumpBreakFloor(message)) {
                PlayerJumpMessageRequest* jumpRequest = mJumpMessageRequest;
                const f32 jumpPower = mConst->getJumpPowerMin();
                jumpRequest->jumpType = static_cast<PlayerJumpType>(5);
                jumpRequest->jumpPower = jumpPower;
                al::setNerve(this, &NrvPlayerActorHakoniwa.Jump);
                return true;
            }
        }
    } while (false);

    if (jumpResult)
        return jumpResult;

    if (rs::isMsgPlayerDamage(message)) {
        if (PlayerFunction::isPlayerDeadStatus(this) ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss) ||
            (al::isNerve(this, &NrvPlayerActorHakoniwa.EndHack) && al::isLessStep(this, 20)))
            return false;
        if (!mModelChanger->is2DModel() || rs::isMsgPlayerDamage2D(message)) {
            if (!mModelChanger->is2DModel() && rs::isMsgPlayerDamage2D(message))
                return false;
        } else {
            return false;
        }
        if (rs::isMsgEnemyAttackFireCollision(message) &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.Swim) &&
            !(al::getVelocity(this).dot(al::getGravity(this)) >= 0.0f))
            return false;
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch) &&
            !mStateWallCatch->isEnableDamage())
            return false;

        if (rs::isMsgFireDamageAll(message)) {
            if (mDamageKeeper->isDamageInvalid())
                return false;
            PlayerTrigger* damageTrigger = mTrigger;
            if (al::isMsgEnemyAttackFire(message)) {
                const char* materialCode = nullptr;
                if (al::tryGetEnemyAttackFireMaterialCode(&materialCode, message))
                    damageTrigger->setRecMaterialTrigger(materialCode);
            }
            damageTrigger->set(PlayerTrigger::EReceiveSensorTrigger_val1);
        } else if (rs::isMsgEnemyAttackFireCollision(message)) {
            mTrigger->set(PlayerTrigger::ECollisionTrigger_val4);
        } else {
            if (mDamageKeeper->isDamageInvalid())
                return false;
            mTrigger->set(PlayerTrigger::EReceiveSensorTrigger_val0);
        }

        if (al::isMsgExplosion(message))
            mStainControl->recordExplosion();
        if (rs::isMsgPoisonDamageAll(message))
            mStainControl->recordPoison();
        PlayerPainPartsKeeper* painPartsKeeper = mPainPartsKeeper;
        if (rs::isMsgAttachCactusNeedle(message) && painPartsKeeper->isEnableNosePain())
            mPainPartsKeeper->appearNeedle();
        if (!mModelChanger->is2DModel() && !al::isNerve(this, &NrvPlayerActorHakoniwa.WallCatch) &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.PoleClimb))
            rs::faceToAttackSensor(this, other);
        return true;
    }

    if (al::isMsgHoldCancel(message) && mCarryKeeper->isCarry()) {
        mCarryKeeper->startRelease();
        return true;
    }

    if (rs::isMsgSandGeyserRaise(message)) {
        PlayerHackKeeper* sandGeyserHackKeeper = mHackKeeper;
        if (!PlayerFunction::isPlayerDeadStatus(this) &&
            !al::isNerve(this, &NrvPlayerActorHakoniwa.Demo)) {
            if (al::isNerve(this, &NrvPlayerActorHakoniwa.Hack)) {
                if (!sandGeyserHackKeeper->getHackSensor())
                    return mActorStateSandGeyser->receiveMsgSandGeyser(message, other);
            } else if (!al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss)) {
                return mActorStateSandGeyser->receiveMsgSandGeyser(message, other);
            }
        }
    }

    const char* waitAnim = nullptr;
    if (rs::tryGetRequestPlayerWaitAnim(&waitAnim, message)) {
        if (al::isNerve(this, &NrvPlayerActorHakoniwa.Wait))
            return mStateWait->requestAnimName(waitAnim);
    } else {
        s32 stainType = 0;
        if (!rs::isMsgRequestPlayerStainWet(message, &stainType))
            return false;

        PlayerBindKeeper* stainBindKeeper = mBindKeeper;
        if (PlayerFunction::isPlayerDeadStatus(this) || stainBindKeeper->getBindSensor() ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) ||
            al::isNerve(this, &NrvPlayerActorHakoniwa.Abyss))
            return false;
        if (stainType == 2)
            mStainControl->recordSandMoonBySensor();
        if (stainType == 0)
            mWetControl->recordWetBySensor();
        if (stainType == 1)
            mStainControl->recordSnowBySensor();
        return true;
    }

    return false;
}

bool PlayerActorHakoniwa::receivePushMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                         al::HitSensor* self, f32 maxPush) {
    if (mModelChanger->is2DModel() || al::isNerve(this, &NrvPlayerActorHakoniwa.Hack) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.Demo) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.PoleClimb) ||
        al::isNerve(this, &NrvPlayerActorHakoniwa.GrabCeil) ||
        rs::isPlayerSafetyPointRecovery(this) || mDamageKeeper->isAbyssDamageInvalid() ||
        (mBindKeeper->getBindSensor() && mPuppet->isBindPushDisabled()))
        return false;

    if (mPushReceiver->receivePushMsg(msg, other, self, maxPush))
        return true;

    return mPushReceiver->receiveCollidePushMsg(msg);
}

bool PlayerActorHakoniwa::tryActionCapSpinAttackImpl(bool isNormal) {
    if (PlayerEquipmentFunction::isEquipmentNoCapThrow(mEquipmentUser) || rs::is2D(this))
        return false;

    const GameDataHolderAccessor accessor(this);
    if (!GameDataFunction::isEnableCap(accessor)) {
        if (!mCarryKeeper->isCarry()) {
            const GameDataHolderAccessor meetCapAccessor(this);
            if (GameDataFunction::isMeetCap(meetCapAccessor) &&
                rs::isJudge(mJudgePreInputCapThrow)) {
                rs::resetJudge(mJudgePreInputCapThrow);
                al::startHitReaction(this, "帽子がつかえない");
            }
        }
        return false;
    }

    const u32 separateFlags = mSeparateCapFlag->getRawFlags();
    const u32 isSeparate = ((separateFlags & 0xFF0000) == 0) &
                           ((separateFlags & 0xFF) != 0);
    bool separateResult;
    if (isSeparate == 1) {
        if (tryActionSeparateCapThrow() || !mInput->isTriggerSpinAttackSeparate())
            return false;
        mCapActionHistory->recordLimitHeight();
        separateResult = 1;
    } else {
        separateResult = 0;
    }

    if ((!isNormal && !mSpinCapAttack->isEnablePlaySpinCapMiss(mAnimator)) ||
        mCarryKeeper->isCarry() || !mSandSinkAffect->isEnableCapThrow())
        return false;

    const u32 inputFlags = mSeparateCapFlag->getRawFlags();
    if (((inputFlags & 0xFF0000) != 0 || (inputFlags & 0xFF) == 0) &&
        !rs::isJudge(mJudgePreInputCapThrow))
        return false;

    if (separateResult)
        mJudgePreInputCapThrow->recordJudgeAndReset();

    if (!mHackCap->isRequestableReturn() && mHackCap->isEnableSpinAttack()) {
        if (isSeparate) {
            if (isNormal) {
                mHackCap->prepareCooperateThrow();
                mJudgePreInputCapThrow->recordCooperateAndReset();
            }
        } else {
            mJudgePreInputCapThrow->recordJudgeAndReset();
        }
        mCapActionHistory->recordLimitHeight();
        return true;
    }

    return separateResult;
}

namespace {

bool processPlayerDamage(const al::LiveActor* player, PlayerDamageKeeper* damageKeeper,
                         PlayerCarryKeeper* carryKeeper, PlayerTrigger* trigger,
                         const PlayerConst* playerConst,
                         PlayerRecoverySafetyPoint* recoverySafetyPoint, const PlayerInput* input) {
    if (PlayerFunction::isPlayerDeadStatus(player) || rs::isActiveDemo(player) ||
        rs::isPlayerSafetyPointRecovery(player) || input->isDamageInputLocked())
        return false;

    const s32 damageInvalidCount = playerConst->getDamageInvalidCount();
    if (trigger->isOn(PlayerTrigger::ECollisionTrigger_val6)) {
        if (!recoverySafetyPoint->isValid() || PlayerFunction::isPlayerHitPointOne(player)) {
            damageKeeper->dead();
            recoverySafetyPoint->reset();
        } else {
            damageKeeper->damageForce(playerConst->getDamageInvalidCountRecovery());
            al::startHitReaction(player, "泡復帰ダメージ");
            trigger->set(PlayerTrigger::EPreMovementTrigger_val1);
        }
    } else if (trigger->isOn(PlayerTrigger::ECollisionTrigger_val3)) {
        if (!recoverySafetyPoint->isValid() || PlayerFunction::isPlayerHitPointOne(player)) {
            damageKeeper->damageForce(damageInvalidCount);
            recoverySafetyPoint->reset();
        } else {
            damageKeeper->damageForce(playerConst->getDamageInvalidCountRecovery());
            al::startHitReaction(player, "泡復帰ダメージ");
            trigger->set(PlayerTrigger::EPreMovementTrigger_val1);
        }
    } else if (trigger->isOn(PlayerTrigger::ECollisionTrigger_val4)) {
        damageKeeper->damageForce(damageInvalidCount);
    } else {
        damageKeeper->damage(damageInvalidCount);
    }

    trigger->set(PlayerTrigger::EPreMovementTrigger_val0);
    if (carryKeeper->isCarry()) {
        if (PlayerFunction::isPlayerDeadStatus(player))
            carryKeeper->startReleaseDead();
        else
            carryKeeper->startReleaseDamage();
    }
    return true;
}

}  // namespace

namespace {

void syncSeparateCapVisibility(PlayerAnimator* animator, HackCap* cap,
                               PlayerModelChangerHakoniwa* modelChanger,
                               const PlayerSeparateCapFlag* separateCapFlag) {
    if (modelChanger->is2DModel())
        return;

    GameDataHolderAccessor accessor(cap);
    if (!GameDataFunction::isEnableCap(accessor)) {
        animator->forceCapOff();
        return;
    }

    u32 flags = separateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) != 0 || (flags & 0xFF) == 0 ||
        ((!separateCapFlag->isSeparateCapLocal() || separateCapFlag->isPuppetable()) &&
         cap->isHide())) {
        if (cap->isNoPutOnHide()) {
            using CapFunction::putOnCapPlayer;
            putOnCapPlayer(cap, animator);
        }
    } else {
        animator->forceCapOff();
    }
}

}  // namespace

namespace {

void PlayerActorHakoniwaNrvFall::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeFall();
}

void PlayerActorHakoniwaNrvWait::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeWait();
}

void PlayerActorHakoniwaNrvSquat::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeSquat();
}

void PlayerActorHakoniwaNrvRun::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeRun();
}

void PlayerActorHakoniwaNrvSlope::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeSlope();
}

void PlayerActorHakoniwaNrvRolling::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeRolling();
}

void PlayerActorHakoniwaNrvSpinCap::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeSpinCap();
}

void PlayerActorHakoniwaNrvJump::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeJump();
}

void PlayerActorHakoniwaNrvCapCatchPop::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeCapCatchPop();
}

void PlayerActorHakoniwaNrvWallAir::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeWallAir();
}

void PlayerActorHakoniwaNrvWallCatch::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeWallCatch();
}

void PlayerActorHakoniwaNrvGrabCeil::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeGrabCeil();
}

void PlayerActorHakoniwaNrvPoleClimb::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exePoleClimb();
}

void PlayerActorHakoniwaNrvHipDrop::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeHipDrop();
}

void PlayerActorHakoniwaNrvHeadSliding::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeHeadSliding();
}

void PlayerActorHakoniwaNrvLongJump::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeLongJump();
}

void PlayerActorHakoniwaNrvSandSink::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeSandSink();
}

void PlayerActorHakoniwaNrvSandGeyser::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeSandGeyser();
}

void PlayerActorHakoniwaNrvRise::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeRise();
}

void PlayerActorHakoniwaNrvSwim::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeSwim();
}

void PlayerActorHakoniwaNrvDamage::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeDamage();
}

void PlayerActorHakoniwaNrvDamageSwim::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeDamageSwim();
}

void PlayerActorHakoniwaNrvDamageFire::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeDamageFire();
}

void PlayerActorHakoniwaNrvPress::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exePress();
}

void PlayerActorHakoniwaNrvHack::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeHack();
}

void PlayerActorHakoniwaNrvEndHack::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeEndHack();
}

void PlayerActorHakoniwaNrvBind::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeBind();
}

void PlayerActorHakoniwaNrvCamera::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeCamera();
}

void PlayerActorHakoniwaNrvAbyss::execute(al::NerveKeeper* keeper) const {
    keeper->getParent<PlayerActorHakoniwa>()->exeAbyss();
}

void PlayerActorHakoniwaNrvDemo::execute(al::NerveKeeper* keeper) const {}

void PlayerActorHakoniwaNrvDead::execute(al::NerveKeeper* keeper) const {}

bool isCarryActionInvalid(const al::LiveActor* player, const PlayerBindKeeper* bindKeeper,
                          const PlayerStateWallAir* stateWallAir,
                          const PlayerStateSwim* stateSwim,
                          const PlayerStatePoleClimb* statePoleClimb) {
    if (isPlayerActionInvalid(player, bindKeeper))
        return true;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.Squat))
        return true;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.Rolling))
        return true;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.GrabCeil))
        return true;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.WallAir))
        return !stateWallAir->isAir();
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.WallCatch))
        return true;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.Swim))
        return !stateSwim->isEnableCarryAction();
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.PoleClimb))
        return !statePoleClimb->isPoleJump();
    return al::isNerve(player, &NrvPlayerActorHakoniwa.HeadSliding);
}

bool isPlayerActionInvalid(const al::LiveActor* player, const PlayerBindKeeper* bindKeeper) {
    if (PlayerFunction::isPlayerDeadStatus(player))
        return true;
    if (bindKeeper->getBindSensor())
        return true;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.Demo))
        return true;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.Hack))
        return true;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.Abyss))
        return true;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.Damage))
        return true;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.DamageSwim))
        return true;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.Camera))
        return true;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.Abyss))
        return true;
    if (al::isNerve(player, &NrvPlayerActorHakoniwa.EndHack) && al::isLessStep(player, 20))
        return true;
    return false;
}

}  // namespace
