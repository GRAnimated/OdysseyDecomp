#include "Player/PlayerStateWait.h"

#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>

#include "Library/Area/AreaObj.h"
#include "Library/Area/AreaObjUtil.h"
#include "Library/Base/StringUtil.h"
#include "Library/Bgm/BgmLineFunction.h"
#include "Library/Collision/PartsConnector.h"
#include "Library/Collision/PartsConnectorUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Shadow/ActorShadowUtil.h"
#include "Library/Stage/StageSwitchUtil.h"

#include "Npc/BirdPlayerGlideCtrl.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerCapManHeroEyesControl.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerFunction.h"
#include "Player/PlayerJointControlKeeper.h"
#include "Player/PlayerJointParamCenterDynamics.h"
#include "Player/PlayerJointParamGroundPose.h"
#include "Player/PlayerModelChangerHakoniwa.h"
#include "Player/PlayerTrigger.h"
#include "Sequence/GameSequenceInfo.h"
#include "System/GameDataFunction.h"
#include "Util/JudgeUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerUtil.h"

namespace {
void updateAreaAnimInfo(al::LiveActor* actor, const PlayerModelChangerHakoniwa* modelChanger,
                        PlayerAnimator* animator, bool* isIgnoreArea, al::AreaObj** areaAnimArea,
                        const char** areaAnimName, s32* areaAnimStartFrame,
                        sead::Vector3f* areaAnimSnapFront, bool* isInvalidateInput,
                        bool* isUseAreaValid, bool* isAreaWaitSitDown, bool* isIgnoreLifeOne);
void calcGroundMovePowerLocal(sead::Vector3f* output, const al::LiveActor* actor,
                              const IUsePlayerCollision* collision);

NERVE_IMPL(PlayerStateWait, Wait);
NERVE_IMPL(PlayerStateWait, LandStiffen);
NERVE_IMPL(PlayerStateWait, Land);
NERVE_IMPL(PlayerStateWait, WaitConnect2D);
NERVE_IMPL(PlayerStateWait, WaitSnoozeStart);
NERVE_IMPL(PlayerStateWait, WaitSnooze);
NERVE_IMPL(PlayerStateWait, WaitSleepStart);
NERVE_IMPL(PlayerStateWait, WaitSleep);
NERVE_IMPL(PlayerStateWait, AreaAnimWait);
NERVE_IMPL(PlayerStateWait, WaitRelaxStart);
NERVE_IMPL(PlayerStateWait, WaitRelax);
NERVE_IMPL(PlayerStateWait, AreaAnimTurn);
NERVE_IMPL(PlayerStateWait, RequestAnimWait);

NERVES_MAKE_STRUCT(PlayerStateWait, Wait, LandStiffen, Land, WaitConnect2D, WaitSnoozeStart,
                   WaitSnooze, WaitSleepStart, WaitSleep, AreaAnimWait, WaitRelaxStart, AreaAnimTurn);

PlayerStateWaitNrvWaitRelax WaitRelaxInstance;
PlayerStateWaitNrvRequestAnimWait RequestAnimWaitInstance;
}  // namespace
// NON_MATCHING: target groups contiguous zero-initialization stores differently; next test is recovering the original default-member initialization boundaries.
PlayerStateWait::PlayerStateWait(
    al::LiveActor* player, const PlayerConst* pConst, const IUsePlayerCollision* collision,
    const PlayerModelChangerHakoniwa* modelChanger,
    const PlayerJointControlKeeper* jointControlKeeper,
    const al::WaterSurfaceFinder* waterSurfaceFinder, const IUsePlayerHeightCheck* heightCheck,
    const IJudge* judgeInWater, PlayerAnimator* animator, PlayerTrigger* trigger,
    PlayerCapManHeroEyesControl* capManHeroEyesControl,
    PlayerJointParamCenterDynamics* centerDynamics, PlayerJointParamGroundPose* groundPose)
    : al::ActorStateBase("待機", player), mConst(pConst), mCollision(collision),
      mModelChanger(modelChanger), mJointControlKeeper(jointControlKeeper),
      mWaterSurfaceFinder(waterSurfaceFinder), mHeightCheck(heightCheck), mJudgeInWater(judgeInWater),
      mAnimator(animator), mTrigger(trigger), mCapManHeroEyesControl(capManHeroEyesControl),
      mCenterDynamics(centerDynamics), mGroundPose(groundPose),
      mMtxConnector(al::createCollisionPartsConnector(player, sead::Quatf::unit)) {
    initNerve(&NrvPlayerStateWait.Wait, 0);
}

// NON_MATCHING: source behavior follows the corpus; stack-local vector layout and helper-call register allocation remain to be refined.
void PlayerStateWait::appear() {
    _e4 = mTrigger->isOn(PlayerTrigger::ECollisionTrigger_val0);
    al::disconnectMtxConnector(mMtxConnector);
    mIsInvalidateInput = false;
    updateAreaAnimInfo(mActor, mModelChanger, mAnimator, &_80, &mAreaAnimArea, &mAreaAnimName,
                       &mAreaAnimStartFrame, &mAreaAnimSnapFront,
                       reinterpret_cast<bool*>(&mIsInvalidateInput), &mIsUseAreaValid,
                       &mIsAreaWaitSitDown, &mIsIgnoreLifeOne);
    _a8 = mAreaAnimStartFrame;
    mRequestAnimName = nullptr;
    al::NerveStateBase::appear();
    mGroundPose->isEnable = false;

    const bool isForceLand = rs::isJudge(mJudgeInWater);
    if (!isForceLand) {
        if (!mTrigger->isOn(PlayerTrigger::ECollisionTrigger_val8) &&
            !mTrigger->isOn(PlayerTrigger::EActionTrigger_val24)) {
            rs::startHitReactionLandIfLanding(mActor, mCollision, mModelChanger->is2DModel());
        }

        _cc = 0;
        if (!rs::isJustLand(mCollision) || mModelChanger->is2DModel() ||
            mTrigger->isOn(PlayerTrigger::EActionTrigger_val24)) {
            mCenterDynamics->isUseBalance =
                mTrigger->isOn(PlayerTrigger::EActionTrigger_val25);
            if (mCenterDynamics->isUseBalance) {
                al::LiveActor* actor = mActor;
                rs::startCenterJointControl(mCenterDynamics, actor, mCollision,
                                            al::getVelocity(actor), false);
                sead::Vector3f groundNormal(0.0f, 0.0f, 0.0f);
                rs::calcGroundNormalOrGravityDir(&groundNormal, actor, mCollision);
                sead::Vector3f groundPoseUp(0.0f, 0.0f, 0.0f);
                mJointControlKeeper->calcGroundPoseUp(&groundPoseUp);
                al::alongVectorNormalH(&mCenterDynamics->balanceDirection,
                                       mCenterDynamics->balanceDirection, groundNormal,
                                       groundPoseUp);
            }

            calcGroundMovePowerLocal(&_c0, mActor, mCollision);
            if (al::isInAreaObj(mActor, "PlayerMoveSmallPlanetArea"))
                al::setNerve(this, &NrvPlayerStateWait.WaitConnect2D);
            else
                al::setNerve(this, &NrvPlayerStateWait.Wait);
            return;
        }
    } else {
        _cc = 0;
    }

    sead::Vector3f groundNormal(0.0f, 0.0f, 0.0f);
    rs::calcGroundNormalOrGravityDir(&groundNormal, mActor, mCollision);
    sead::Vector3f velocityH(0.0f, 0.0f, 0.0f);
    const sead::Vector3f& velocity = al::getVelocity(mActor);
    const sead::Vector3f& actorGravity = al::getGravity(mActor);
    sead::Vector3f resultVelocity(-actorGravity.x, -actorGravity.y, -actorGravity.z);
    al::alongVectorNormalH(&velocityH, velocity, resultVelocity, groundNormal);
    al::LiveActor* actor = mActor;
    const f32 gravity = mConst->getGravity();
    resultVelocity.set(velocityH.x - gravity * groundNormal.x,
                       velocityH.y - gravity * groundNormal.y,
                       velocityH.z - gravity * groundNormal.z);
    al::setVelocity(actor, resultVelocity);

    if (isForceLand) {
        al::setNerve(this, &NrvPlayerStateWait.LandStiffen);
    } else {
        if (!mAnimator->isAnim("Jump3"))
            rs::startCenterJointControl(mCenterDynamics, mActor, mCollision,
                                        al::getVelocity(mActor), true);
        al::setNerve(this, &NrvPlayerStateWait.Land);
    }
    calcGroundMovePowerLocal(&_c0, mActor, mCollision);
}

namespace {
// NON_MATCHING: target is 0x2E0 bytes versus current 0x2F0; register allocation diverges in the temperature/costume tail. Next hypothesis: simplify area/temp locals and preserve target output-pointer lifetimes.
void updateAreaAnimInfo(al::LiveActor* actor, const PlayerModelChangerHakoniwa* modelChanger,
                        PlayerAnimator* animator, bool* isIgnoreArea,
                        al::AreaObj** areaAnimArea, const char** areaAnimName,
                        s32* areaAnimStartFrame, sead::Vector3f* areaAnimSnapFront,
                        bool* isInvalidateInput, bool* isUseAreaValid,
                        bool* isAreaWaitSitDown, bool* isIgnoreLifeOne) {
    al::AreaObj* area = al::tryFindAreaObj(actor, "PlayerAnimArea", al::getTrans(actor));
    if (!*isIgnoreArea || *areaAnimArea != area || !*areaAnimArea) {
        *isIgnoreArea = false;
        *isInvalidateInput = false;
        *areaAnimArea = nullptr;
        *areaAnimName = nullptr;
        *areaAnimStartFrame = 0;
        *isUseAreaValid = false;
        *isAreaWaitSitDown = false;
        *isIgnoreLifeOne = false;
        *areaAnimSnapFront = sead::Vector3f::zero;

        if (area) {
            *areaAnimArea = area;
            al::tryGetAreaObjStringArg(areaAnimName, area, "PlayerAnimName");
            al::tryGetAreaObjArg(isUseAreaValid, area, "IsUseAreaValid");
            al::tryGetAreaObjArg(areaAnimStartFrame, area, "AnimStartFrame");
            const char* snapFrontName = "None";
            al::tryGetAreaObjStringArg(&snapFrontName, area, "PlayerAnimNameSnapFront");
            if (!al::isEqualString(snapFrontName, "None")) {
                *areaAnimName = snapFrontName;
                al::getAreaObjDirFront(areaAnimSnapFront, area);
            }
            if (*areaAnimName)
                *isAreaWaitSitDown = al::isEqualString(*areaAnimName, "AreaWaitSitDown");
            al::tryGetAreaObjArg(isInvalidateInput, area, "IsInvalidateInput");
            al::tryGetAreaObjArg(isIgnoreLifeOne, area, "IsIgnoreLifeOne");
            if (al::isValidStageSwitch(area, "SwitchPlayerAnimEndOn"))
                *isInvalidateInput &= !al::isOnStageSwitch(area, "SwitchPlayerAnimEndOn");
        } else {
            s32 temperature = 0;
            al::AreaObj* temperatureArea =
                al::tryFindAreaObj(actor, "TemperatureArea", al::getTrans(actor));
            if (temperatureArea) {
                al::tryGetAreaObjArg(&temperature, temperatureArea, "Temperature");
                if (temperature == -4)
                    temperature = al::findIsInShade(actor, al::getTrans(actor)) ? -2 : 0;
            }

            switch (modelChanger->calcCostumeWarmLevel(temperature)) {
            case -3:
                *areaAnimName = "WaitVeryCold";
                *areaAnimStartFrame = 0;
                break;
            case -2:
                *areaAnimName = "WaitCold";
                *areaAnimStartFrame = 0;
                break;
            case -1:
            case 0:
            case 1:
                *areaAnimName = nullptr;
                *areaAnimStartFrame = 0;
                break;
            case 2:
            case 3: {
                *areaAnimName = "WaitHot";
                f32 frame = animator->getMario3DWaitFrameMax();
                *areaAnimStartFrame = static_cast<s32>(frame + (frame >= 0.0f ? 0.5f : -0.5f));
                break;
            }
            }
        }
    }
}


// NON_MATCHING: target is 0x1A0 bytes versus current 0x1CC; vector/quaternion expression lowering differs before verticalizeVec. Next hypothesis: recover the original in-place quat/vector helper sequence.
void calcGroundMovePowerLocal(sead::Vector3f* output, const al::LiveActor* actor,
                              const IUsePlayerCollision* collision) {
    sead::Vector3f movePower = sead::Vector3f::zero;
    if (rs::isCollidedGround(collision))
        rs::calcMovePowerGround(&movePower, collision, al::getTrans(actor));

    sead::Quatf inverseQuat = sead::Quatf::unit;
    al::calcQuat(&inverseQuat, actor);
    inverseQuat.inverse();
    output->setRotated(inverseQuat, movePower);
    al::verticalizeVec(output, al::getGravity(actor), *output);
}
}  // namespace


void PlayerStateWait::kill() {
    mCenterDynamics->isUseTilt = true;
    mGroundPose->isEnable = false;
    if (mAreaAnimArea)
        al::tryOffStageSwitch(mAreaAnimArea, "SwitchPlayerAnimKeepOn");
    rs::invalidateGlideBirdOnPlayerNose(mActor);
    al::endBgmSituation(mActor, "MarioSleep", false);
    al::NerveStateBase::kill();
}

// NON_MATCHING: high-level behavior is recovered; quaternion/vector expression ordering remains to be tuned against the target's inlined math.
void PlayerStateWait::control() {
    const al::LiveActor* actor = mActor;
    sead::Vector3f movePower(0.0f, 0.0f, 0.0f);
    if (rs::isCollidedGround(mCollision))
        rs::calcMovePowerGround(&movePower, mCollision, al::getTrans(actor));

    sead::Quatf quat = sead::Quatf::unit;
    al::calcQuat(&quat, actor);
    sead::Quatf inverseQuat = quat;
    inverseQuat.inverse();

    sead::Vector3f currentMovePower;
    currentMovePower.setRotated(inverseQuat, movePower);
    al::verticalizeVec(&currentMovePower, al::getGravity(actor), currentMovePower);
    sead::Vector3f difference = currentMovePower - _c0;
    _c0 = currentMovePower;

    if (difference.length() >= 3.0f) {
        const f32 rate = al::easeOut(al::calcRate01(static_cast<f32>(_cc), 0.0f, 10.0f));
        difference *= -al::lerpValue(0.5f, 0.01f, rate);
        mCenterDynamics->isReset = true;
        mCenterDynamics->balanceUp.setRotated(quat, difference);
        al::limitLength(&mCenterDynamics->balanceUp, mCenterDynamics->balanceUp, 4.0f);
        _cc = 10;
    } else {
        _cc = al::converge(_cc, 0, 1);
    }
}

bool PlayerStateWait::isWait() const {
    if (isDead())
        return false;
    return !al::isNerve(this, &NrvPlayerStateWait.LandStiffen) && !al::isNerve(this, &NrvPlayerStateWait.Land);
}

bool PlayerStateWait::isEnableLookAt() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvPlayerStateWait.LandStiffen))
        return false;
    if (al::isNerve(this, &NrvPlayerStateWait.Land))
        return false;
    if (al::isNerve(this, &NrvPlayerStateWait.WaitSnoozeStart))
        return false;
    if (al::isNerve(this, &NrvPlayerStateWait.WaitSnooze))
        return false;
    if (al::isNerve(this, &NrvPlayerStateWait.WaitSleepStart))
        return false;
    return !al::isNerve(this, &NrvPlayerStateWait.WaitSleep);
}

bool PlayerStateWait::isEnableCancelAction() const {
    const bool isUseAreaValid = mIsInvalidateInput;
    if (al::isNerve(this, &NrvPlayerStateWait.AreaAnimTurn)) {
        if (isUseAreaValid)
            return false;
    } else {
        const u8 isAreaAnimWait = al::isNerve(this, &NrvPlayerStateWait.AreaAnimWait);
        if (isUseAreaValid) {
            if (isAreaAnimWait)
                return false;
        }
    }

    if (al::isNerve(this, &NrvPlayerStateWait.LandStiffen)) {
        if (_b0 >= 0)
            return al::isGreaterEqualStep(this, _b0);
        return false;
    }
    return true;
}

bool PlayerStateWait::isEnableCancelHipDropJump() const {
    const bool isUseAreaValid = mIsInvalidateInput;
    if (al::isNerve(this, &NrvPlayerStateWait.AreaAnimTurn)) {
        if (isUseAreaValid)
            return false;
    } else {
        const u8 isAreaAnimWait = al::isNerve(this, &NrvPlayerStateWait.AreaAnimWait);
        if (isUseAreaValid) {
            if (isAreaAnimWait)
                return false;
        }
    }

    if (al::isNerve(this, &NrvPlayerStateWait.LandStiffen) && _b0 >= 0)
        return al::isInRangeStep(this, mConst->getJumpHipDropPermitBeginFrame(),
                                 mConst->getJumpHipDropPermitEndFrame());
    return false;
}

bool PlayerStateWait::isEnableRecoveryLife() const {
    if (isDead())
        return false;
    const bool isUseAreaValid = mIsInvalidateInput;
    if (al::isNerve(this, &NrvPlayerStateWait.AreaAnimTurn)) {
        if (isUseAreaValid)
            return false;
    } else {
        const u8 isAreaAnimWait = al::isNerve(this, &NrvPlayerStateWait.AreaAnimWait);
        if (isUseAreaValid) {
            if (isAreaAnimWait)
                return false;
        }
    }

    if (!al::isNerve(this, &NrvPlayerStateWait.LandStiffen))
        return true;
    if (_b0 >= 0)
        return al::isGreaterEqualStep(this, _b0);
    return false;
}

bool PlayerStateWait::isEnableReactionCapCatch() const {
    if (isDead())
        return true;
    return !mAreaAnimArea || !al::isValidStageSwitch(mAreaAnimArea, "SwitchPlayerAnimEndOn");
}

bool PlayerStateWait::isAreaAnimCapManHeroEyes() const {
    return !isDead() && al::isNerve(this, &NrvPlayerStateWait.AreaAnimWait) && mIsInvalidateInput &&
           al::isGreaterEqualStep(this, 20);
}

bool PlayerStateWait::isPlaySwitchOnAreaAnim() const {
    if (isDead())
        return false;
    const bool isUseAreaValid = mIsInvalidateInput;
    if (al::isNerve(this, &NrvPlayerStateWait.AreaAnimTurn))
        return isUseAreaValid;
    return isUseAreaValid & al::isNerve(this, &NrvPlayerStateWait.AreaAnimWait);
}

bool PlayerStateWait::isLandStain() const {
    return !isDead() && al::isNerve(this, &NrvPlayerStateWait.LandStiffen) && rs::isCollidedGround(mCollision);
}

bool PlayerStateWait::isSleep() const {
    return !isDead() && al::isNerve(this, &NrvPlayerStateWait.WaitSleep) && !mAnimator->isSubAnimPlaying();
}

bool PlayerStateWait::attackHipDropKnockDown(al::HitSensor* self, al::HitSensor* other) {
    if (isDead() || !al::isNerve(this, &NrvPlayerStateWait.LandStiffen) || _b0 < 0 || !al::isFirstStep(this))
        return false;
    return al::sendMsgPlayerHipDropKnockDown(other, self);
}

bool PlayerStateWait::tryClearIgnoreSwitchOnAreaAnim() {
    if (!_80)
        return true;
    if (al::tryFindAreaObj(mActor, "PlayerAnimArea", al::getTrans(mActor)) != mAreaAnimArea) {
        _80 = false;
        return true;
    }
    return false;
}

bool PlayerStateWait::requestAnimName(const char* animationName) {
    if (mRequestAnimName)
        return false;
    mRequestAnimName = animationName;
    return true;
}

void PlayerStateWait::noticeCarryStart() {
    if (!isDead() && !mAnimator->isAnim("Wait")) {
        mAnimator->startAnim("Wait");
        al::setNerve(this, &NrvPlayerStateWait.Wait);
    }
}

void PlayerStateWait::initSceneStartAnim() {
    al::LiveActor* actor = mActor;
    s32 temperature = 0;
    al::AreaObj* area = al::tryFindAreaObj(actor, "TemperatureArea", al::getTrans(actor));
    if (area) {
        al::tryGetAreaObjArg(&temperature, area, "Temperature");
        if (temperature == -4) {
            if (al::findIsInShade(actor, al::getTrans(actor)))
                temperature = -2;
            else
                temperature = 0;
        }
    }

    const s32 warmLevel = mModelChanger->calcCostumeWarmLevel(temperature);
    if (warmLevel == -3)
        mAnimator->startAnim("WaitVeryCold");
    else if (warmLevel == -2)
        mAnimator->startAnim("WaitCold");
    mAnimator->clearInterpolation();
}

bool PlayerStateWait::tryConnectWait() {
    if (!al::isNerve(this, &NrvPlayerStateWait.WaitConnect2D) || !al::isMtxConnectorConnecting(mMtxConnector))
        return false;
    al::calcConnectTrans(al::getTransPtr(mActor), mMtxConnector, mConnectTrans);
    return true;
}

f32 PlayerStateWait::getInverseKinematicsRate() const {
    f32 rate = 0.0f;
    if (!isDead() && !al::isNerve(this, &NrvPlayerStateWait.WaitSleep)) {
        if (al::isNerve(this, &NrvPlayerStateWait.WaitSleepStart)) {
            rate = sead::Mathf::clamp(1.0f - al::calcNerveRate(this, 120), 0.0f, 1.0f);
        } else {
            rate = 1.0f;
        }
    }
    return rate;
}

void PlayerStateWait::exeLandStiffen() {
    al::LiveActor* actor = mActor;
    f32 snap = 0.0f;
    if (al::isFirstStep(this)) {
        _b0 = mConst->getHipDropLandCancelFrame();
        if (mAnimator->isAnim("SpinJumpDownFall")) {
            mAnimator->startAnim("LandDownFall");
        } else if (mAnimator->isAnim("SpinJumpDownFallL")) {
            mAnimator->startAnim("LandDownFall");
        } else if (mAnimator->isAnim("SpinJumpDownFallR")) {
            mAnimator->startAnim("LandDownFall");
        } else {
            al::startHitReaction(actor, "高所着地");
            mAnimator->startAnim("LandStiffen");
            _b0 = -1;
        }
        rs::landGroundPoseAndSnap(actor, mTrigger, &snap, mCollision, mConst);
    }

    f32 gravity = mConst->getGravity() + snap;
    rs::waitGround(actor, mCollision, gravity, mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());
    if (mAnimator->isAnimEnd())
        al::setNerve(this, &NrvPlayerStateWait.Wait);
}

void PlayerStateWait::exeLand() {
    al::LiveActor* actor = mActor;
    f32 snap = 0.0f;
    if (al::isFirstStep(this)) {
        if (mAnimator->isAnim("JumpTurn"))
            mAnimator->startAnim("LandTurn");
        else if (mAnimator->isAnim("JumpBroad"))
            mAnimator->startAnim("LandRolling");
        else if (mAnimator->isAnim("Jump3"))
            mAnimator->startAnim("LandJump3");
        else
            mAnimator->startAnim("Land");

        mGroundPose->isEnable = mAnimator->isAnim("LandJump3");
        rs::landGroundPoseAndSnap(actor, mTrigger, &snap, mCollision, mConst);
    }

    f32 gravity = mConst->getGravity() + snap;
    rs::waitGround(actor, mCollision, gravity, mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());
    if (mAnimator->isAnimEnd()) {
        mGroundPose->isEnable = false;
        al::setNerve(this, &NrvPlayerStateWait.Wait);
    }
}

// NON_MATCHING: first-step animation-selection register allocation differs; next test is preserving the selected SafeString temporary.
void PlayerStateWait::exeWait() {
    al::LiveActor* actor = mActor;
    tryUpdateAreaAnim();
    if (tryChangeRequestAnim() || tryChangeAreaAnim())
        return;

    if (al::isFirstStep(this)) {
        const char* animationName = "Wait";
        if (!mModelChanger->is2DModel()) {
            const al::LiveActor* player = mActor;
            if (!PlayerFunction::isPlayerHitPointOne(player) ||
                (mAreaAnimArea && mIsIgnoreLifeOne)) {
                animationName = "BattleWait";
                if (!GameDataFunction::isRemovedCapByJango(player) &&
                    !rs::isSceneStatusBossBattleForPlayerAnim(player))
                    animationName = "Wait";
            } else {
                animationName = "DamageWait";
            }
        }

        if (!_e4 || !mAnimator->isAnim(animationName))
            mAnimator->startAnim(animationName);
    }

    sead::Vector3f groundNormal(0.0f, 0.0f, 0.0f);
    rs::calcGroundNormalOrGravityDir(&groundNormal, actor, mCollision);
    rs::waitGround(actor, mCollision, mConst->getGravity(), mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());

    if (mAnimator->isAnim("DamageWait") && !PlayerFunction::isPlayerHitPointOne(actor)) {
        al::setNerve(this, &NrvPlayerStateWait.Wait);
        return;
    }

    if (al::isLessStep(this, 180))
        return;
    if (rs::isPlayerCarrySomething(actor))
        return;
    if (mModelChanger->is2DModel())
        return;
    if (mAnimator->isAnim("Wait"))
        al::setNerve(this, &NrvPlayerStateWait.WaitRelaxStart);
}

bool PlayerStateWait::tryUpdateAreaAnim() {
    if (mAreaAnimArea)
        return false;

    al::LiveActor* actor = mActor;
    al::AreaObj* area = al::tryFindAreaObj(actor, "PlayerAnimArea", al::getTrans(actor));
    if (!area)
        return false;

    bool isUseAreaValid = false;
    al::tryGetAreaObjArg(&isUseAreaValid, area, "IsUseAreaValid");
    if (!isUseAreaValid)
        return false;

    updateAreaAnimInfo(actor, mModelChanger, mAnimator, &_80, &mAreaAnimArea, &mAreaAnimName,
                       &mAreaAnimStartFrame, &mAreaAnimSnapFront,
                       reinterpret_cast<bool*>(&mIsInvalidateInput),
                       &mIsUseAreaValid, &mIsAreaWaitSitDown, &mIsIgnoreLifeOne);
    _a8 = mAreaAnimStartFrame;
    return true;
}

bool PlayerStateWait::tryChangeRequestAnim() {
    if (mModelChanger->is2DModel())
        return false;
    if (!mRequestAnimName || rs::isPlayerCarrySomething(mActor))
        return false;

    rs::waitGround(mActor, mCollision, mConst->getGravity(), mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());
    PlayerAnimator* animator = mAnimator;
    const char* requestAnimName = mRequestAnimName;
    const sead::SafeString animationName(requestAnimName);
    animator->startAnim(animationName);
    al::setNerve(this, &RequestAnimWaitInstance);
    return true;
}

bool PlayerStateWait::tryChangeAreaAnim() {
    if (_80)
        return false;
    if (mModelChanger->is2DModel())
        return false;
    if (!mAreaAnimName || rs::isPlayerCarrySomething(mActor))
        return false;

    if (!mModelChanger->is2DModel()) {
        const al::LiveActor* actor = mActor;
        if ((PlayerFunction::isPlayerHitPointOne(actor) &&
             (!mAreaAnimArea || !mIsIgnoreLifeOne)) ||
            GameDataFunction::isRemovedCapByJango(actor) ||
            rs::isSceneStatusBossBattleForPlayerAnim(actor)) {
            if (!mAreaAnimArea)
                return false;
            if (!al::isValidStageSwitch(mAreaAnimArea, "SwitchPlayerAnimEndOn") &&
                !mIsAreaWaitSitDown)
                return false;
        }
    }

    if (mAreaAnimArea && mIsUseAreaValid && !mAreaAnimArea->isValid())
        return false;
    if (_a8 != 0) {
        _a8 = al::converge(_a8, 0, 1);
        return false;
    }

    rs::waitGround(mActor, mCollision, mConst->getGravity(), mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());

    al::LiveActor* actor = mActor;
    if (!al::isNearZero(mAreaAnimSnapFront, 0.001f)) {
        sead::Vector3f up(0.0f, 0.0f, 0.0f);
        al::calcUpDir(&up, actor);
        if (!al::turnToDirectionAxis(actor, mAreaAnimSnapFront, up, 10.0f)) {
            al::setNerve(this, &NrvPlayerStateWait.AreaAnimTurn);
            return true;
        }
    }

    al::AreaObj* area = mAreaAnimArea;
    const char* animationName = mAreaAnimName;
    PlayerAnimator* animator = mAnimator;
    if (area)
        al::tryOnStageSwitch(area, "SwitchPlayerAnimKeepOn");
    const sead::SafeString animation(animationName);
    animator->startAnim(animation);
    al::setNerve(this, &NrvPlayerStateWait.AreaAnimWait);
    return true;
}

bool PlayerStateWait::tryGetSpecialStatusAnimName(const char** animationName) {
    if (mModelChanger->is2DModel())
        return false;
    const al::LiveActor* actor = mActor;
    if (PlayerFunction::isPlayerHitPointOne(actor) &&
        (!mAreaAnimArea || !mIsIgnoreLifeOne)) {
        *animationName = "DamageWait";
        return true;
    }
    if (GameDataFunction::isRemovedCapByJango(actor) ||
        rs::isSceneStatusBossBattleForPlayerAnim(actor)) {
        *animationName = "BattleWait";
        return true;
    }
    return false;
}

void PlayerStateWait::exeWaitConnect2D() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this))
        mAnimator->startAnim("Wait");

    sead::Vector3f groundNormal(0.0f, 0.0f, 0.0f);
    rs::calcGroundNormalOrGravityDir(&groundNormal, actor, mCollision);
    rs::waitGround(actor, mCollision, mConst->getGravityMove(), mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());

    if (!al::isMtxConnectorConnecting(mMtxConnector) && rs::isCollidedGround(mCollision)) {
        sead::Vector3f up(0.0f, 0.0f, 0.0f);
        al::calcUpDir(&up, actor);
        if (up.dot(rs::getCollidedGroundNormal(mCollision)) >=
            sead::Mathf::cos(sead::Mathf::deg2rad(1.0f))) {
            sead::Vector3f movePower(0.0f, 0.0f, 0.0f);
            const IUsePlayerCollision* collision = mCollision;
            rs::calcMovePowerGround(&movePower, collision,
                                    rs::getCollidedGroundPos(collision));
            const sead::Vector3f& groundPos = rs::getCollidedGroundPos(mCollision);
            mConnectTrans = groundPos + movePower;
            al::attachMtxConnectorToCollisionParts(
                mMtxConnector, rs::getCollidedGroundCollisionParts(mCollision));
        }
    }

    if (!mModelChanger->is2DModel())
        al::setNerve(this, &NrvPlayerStateWait.Wait);
}

void PlayerStateWait::exeWaitRelaxStart() {
    al::LiveActor* actor = mActor;
    tryUpdateAreaAnim();
    if (tryChangeRequestAnim() || tryChangeAreaAnim())
        return;

    if (al::isFirstStep(this))
        mAnimator->startAnim("WaitRelaxStart");
    rs::waitGround(actor, mCollision, mConst->getGravity(), mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());
    if (mAnimator->isAnimEnd())
        al::setNerve(this, &WaitRelaxInstance);
}

void PlayerStateWait::exeWaitRelax() {
    al::LiveActor* actor = mActor;
    tryUpdateAreaAnim();
    if (tryChangeRequestAnim() || tryChangeAreaAnim())
        return;

    if (al::isFirstStep(this))
        mAnimator->startAnim("WaitRelax");
    rs::waitGround(actor, mCollision, mConst->getGravity(), mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());
    if (al::isGreaterEqualStep(this, 1200) &&
        !rs::isUnderWaterSurface(mWaterSurfaceFinder, mHeightCheck, 30.0f))
        al::setNerve(this, &NrvPlayerStateWait.WaitSnoozeStart);
}

void PlayerStateWait::exeWaitSnoozeStart() {
    al::LiveActor* actor = mActor;
    if (tryChangeRequestAnim() || tryChangeAreaAnim())
        return;

    if (al::isFirstStep(this)) {
        mAnimator->startAnim("SnoozeStart");
        mCapManHeroEyesControl->requestWaitAnimChange("SnoozeStart");
    }
    rs::waitGround(actor, mCollision, mConst->getGravity(), mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());
    if (mAnimator->isAnimEnd())
        al::setNerve(this, &NrvPlayerStateWait.WaitSnooze);
}

void PlayerStateWait::exeWaitSnooze() {
    al::LiveActor* actor = mActor;
    if (tryChangeRequestAnim() || tryChangeAreaAnim())
        return;

    if (al::isFirstStep(this)) {
        mAnimator->startAnim("Snooze");
        mCapManHeroEyesControl->requestWaitAnimChange("Snooze");
    }
    if (mAnimator->isAnim("Snooze") &&
        al::isNearZero(mAnimator->getAnimFrame() - mAnimator->getAnimFrameMax(), 0.001f))
        mCapManHeroEyesControl->requestWaitAnimChange("Snooze");

    rs::waitGround(actor, mCollision, mConst->getGravity(), mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());
    if (al::isGreaterEqualStep(this, 900))
        al::setNerve(this, &NrvPlayerStateWait.WaitSleepStart);
}

void PlayerStateWait::exeWaitSleepStart() {
    al::LiveActor* actor = mActor;
    if (tryChangeRequestAnim() || tryChangeAreaAnim())
        return;

    if (al::isFirstStep(this)) {
        mAnimator->startAnim("SleepStart");
        mCapManHeroEyesControl->requestWaitAnimChange("SleepStart");
        al::startBgmSituation(mActor, "MarioSleep", false, true);
    }
    rs::waitGround(actor, mCollision, mConst->getGravity(), mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());
    if (mAnimator->isAnimEnd())
        al::setNerve(this, &NrvPlayerStateWait.WaitSleep);
}

void PlayerStateWait::exeWaitSleep() {
    al::LiveActor* actor = mActor;
    if (tryChangeRequestAnim() || tryChangeAreaAnim()) {
        rs::invalidateGlideBirdOnPlayerNose(actor);
        return;
    }

    if (al::isFirstStep(this)) {
        mAnimator->startAnim("Sleep");
        mCapManHeroEyesControl->requestWaitAnimChange("Sleep");
    }
    if (mAnimator->isAnim("Sleep") &&
        al::isNearZero(mAnimator->getAnimFrame() - mAnimator->getAnimFrameMax(), 0.001f))
        mCapManHeroEyesControl->requestWaitAnimChange("Sleep");

    if (mAnimator->isSubAnimPlaying())
        rs::invalidateGlideBirdOnPlayerNose(actor);
    else
        rs::validateGlideBirdOnPlayerNose(actor);

    rs::waitGround(actor, mCollision, mConst->getGravity(), mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());
}

void PlayerStateWait::exeAreaAnimTurn() {
    if (al::isFirstStep(this))
        mAnimator->startAnim("TurnPoint");

    rs::waitGround(mActor, mCollision, mConst->getGravity(), mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());

    al::LiveActor* actor = mActor;
    if (!al::isNearZero(mAreaAnimSnapFront, 0.001f)) {
        sead::Vector3f up(0.0f, 0.0f, 0.0f);
        al::calcUpDir(&up, actor);
        if (!al::turnToDirectionAxis(actor, mAreaAnimSnapFront, up, 10.0f))
            return;
    }

    al::AreaObj* area = mAreaAnimArea;
    const char* animationName = mAreaAnimName;
    PlayerAnimator* animator = mAnimator;
    if (area)
        al::tryOnStageSwitch(area, "SwitchPlayerAnimKeepOn");
    const sead::SafeString animation(animationName);
    animator->startAnim(animation);
    al::setNerve(this, &NrvPlayerStateWait.AreaAnimWait);
}

void PlayerStateWait::exeAreaAnimWait() {
    rs::waitGround(mActor, mCollision, mConst->getGravity(), mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());
    if (mIsAreaWaitSitDown && al::isStep(mActor, 480))
        GameDataFunction::recoveryPlayerMax(mActor);

    if (mAreaAnimArea && mIsUseAreaValid && !mAreaAnimArea->isValid()) {
        al::setNerve(this, &NrvPlayerStateWait.Wait);
        return;
    }

    if (mAnimator->isCurrentAnimOneTime() && mAnimator->isAnimEnd()) {
        if (mAreaAnimArea) {
            al::tryOnStageSwitch(mAreaAnimArea, "SwitchPlayerAnimEndOn");
            al::tryOffStageSwitch(mAreaAnimArea, "SwitchPlayerAnimKeepOn");
        }
        _80 = true;
        mIsInvalidateInput = false;
        _a8 = mAreaAnimStartFrame;
        al::setNerve(this, &NrvPlayerStateWait.Wait);
    }
}

void PlayerStateWait::exeRequestAnimWait() {
    rs::waitGround(mActor, mCollision, mConst->getGravity(), mConst->getFallSpeedMax(),
                   mConst->getSlerpQuatRateWait(), mConst->getWaitPoseDegreeMax());
    if (mAnimator->isCurrentAnimOneTime() && mAnimator->isAnimEnd())
        al::setNerve(this, &NrvPlayerStateWait.Wait);
}

PlayerStateWait::~PlayerStateWait() = default;

