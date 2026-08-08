#include "Player/PlayerStateJump.h"

#include "Library/Base/StringUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/IPlayerModelChanger.h"
#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerContinuousJump.h"
#include "Player/PlayerCounterForceRun.h"
#include "Player/PlayerInput.h"
#include "Util/ActorDimensionUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(PlayerStateJump, Jump)
NERVE_IMPL(PlayerStateJump, HoveringJump2D)
NERVE_IMPL(PlayerStateJump, JumpSpinFlower)
NERVE_IMPL(PlayerStateJump, JumpBack)
NERVE_IMPL(PlayerStateJump, JumpTurn)
NERVE_IMPL(PlayerStateJump, JumpSpinGround)
NERVE_IMPL(PlayerStateJump, JumpSpinFlowerDownFall)
NERVE_IMPL(PlayerStateJump, JumpSpinGroundDownFall)
NERVE_IMPL(PlayerStateJump, Hovering2D)
NERVES_MAKE_STRUCT(PlayerStateJump, Jump, HoveringJump2D, JumpSpinFlower, JumpBack, JumpTurn,
                   JumpSpinGround, JumpSpinFlowerDownFall, JumpSpinGroundDownFall, Hovering2D)
}  // namespace

// NON_MATCHING: behavior and target size (0x18c) are recovered. The normalized instruction stream
// differs only in one zero-store scheduling move, immediate materialization, and one saved-argument
// reload/register-lifetime choice. Next source-level hypothesis is target-like constructor member
// initialization grouping around mAirMoveControl and the stack-passed tail parameters.
PlayerStateJump::PlayerStateJump(
    al::LiveActor* player, const PlayerConst* pConst, const IPlayerModelChanger* modelChanger,
    const IUseDimension* dimension, const PlayerInput* input,
    const PlayerJumpMessageRequest* jumpRequest, const IJudge* judgeStartRise,
    const PlayerCounterForceRun* counterForceRun,
    const PlayerJointControlKeeper* jointControlKeeper, const IUsePlayerCollision* collision,
    PlayerTrigger* trigger, PlayerContinuousJump* continuousJump, PlayerAnimator* animator,
    PlayerActionDiveInWater* actionDiveInWater, HackCap* hackCap, IJudge* judgeWallKeep, bool is2D)
    : al::ActorStateBase("", player), mConst(pConst), mModelChanger(modelChanger),
      mDimension(dimension), mJumpRequest(jumpRequest), mJudgeStartRise(judgeStartRise),
      mCounterForceRun(counterForceRun), mJointControlKeeper(jointControlKeeper),
      mCollision(collision), mAnimator(animator), mContinuousJump(continuousJump), mTrigger(trigger),
      mActionDiveInWater(actionDiveInWater), mAirMoveControl(nullptr), mHackCap(hackCap),
      mJudgeWallKeep(judgeWallKeep), mIs2D(is2D), _9c(0), _a0(0.0f), _a4(0.0f), _a8(0.0f),
      _ac(0), _b0(0), _b4(0), _b5(0), _b6(0), _b7(0), _b8(0), _b9(0), _ba(0), _bb(0),
      _bc(0.0f, 0.0f, 0.0f), _c8(nullptr), _d0(nullptr), mInput(input),
      mIsDownFallGroundCollision(false), mDownFallGroundPos(0.0f, 0.0f, 0.0f) {
    turnJumpAngle.set(0.0f, 0.0f, 0.0f);
    downFallConvergeCounter = 0;

    mAirMoveControl = new PlayerActionAirMoveControl(player, pConst, input, collision, false);
    mAirMoveControl->setUseGroundNormalForStartMove(true);
    mAirMoveControl->setStartMoveSpeedClamp(true, 0.0f, pConst->getJumpBaseSpeedMax());
    initNerve(&NrvPlayerStateJump.Jump, 0);
}

f32 PlayerStateJump::calcJumpPowerBorderSpeedMin() const {
    const IUseDimension* dimension = mDimension;
    bool use2DBorder = mModelChanger->is2DModel();
    if (use2DBorder)
        use2DBorder = rs::isIn2DArea(dimension);
    if (use2DBorder)
        return mConst->getJumpPowerMinBorder2D();
    return mConst->getNormalMinSpeed();
}

f32 PlayerStateJump::calcJumpPowerBorderSpeedMax() const {
    const IUseDimension* dimension = mDimension;
    bool use2DBorder = mModelChanger->is2DModel();
    if (use2DBorder)
        use2DBorder = rs::isIn2DArea(dimension);
    if (use2DBorder)
        return mConst->getJumpPowerMaxBorder2D();
    return mConst->getNormalMaxSpeed();
}

// NON_MATCHING: target is 0xb0 bytes and current is 0xac; behavior is recovered but switch
// lowering reaches the shared epilogue too early; next source-level hypothesis is target-order
// mode tests with the 3D/default getter kept in a distinct tail.
f32 PlayerStateJump::calcJumpPowerMin() const {
    const IUseDimension* dimension = mDimension;
    if (mModelChanger->is2DModel() && rs::isIn2DArea(dimension))
        return mConst->getJumpPowerMin2DArea();
    if (mCounterForceRun->isForceRun())
        return mConst->getJumpPowerForceRun();

    switch (_b0) {
    case 2:
    case 1:
        return mConst->getContinuousJumpPowerMin();
    case 0:
        return mConst->getJumpPowerMin();
    default:
        return mConst->getJumpPowerMax();
    }
}

// NON_MATCHING: target/current are 0xac bytes; behavior is recovered but branch-target layout
// places the shared epilogue too early; next source-level hypothesis is separate mode-1/mode-2
// return blocks that only join after selecting the jump-power getter.
f32 PlayerStateJump::calcJumpPowerMax() const {
    const IUseDimension* dimension = mDimension;
    if (mModelChanger->is2DModel() && rs::isIn2DArea(dimension))
        return mConst->getJumpPowerMax2DArea();
    if (mCounterForceRun->isForceRun())
        return mConst->getJumpPowerForceRun();

    switch (_b0) {
    case 2:
        return mConst->getJumpPowerMax3rd();
    case 1:
        return mConst->getJumpPowerMax2nd();
    default:
        return mConst->getJumpPowerMax();
    }
}

// NON_MATCHING: target/current are 0x64 bytes; current switch compares mode 1 then 2 while target compares 2 then 1; next source-level hypothesis is a control-flow form that preserves target comparison order without changing the early force-run path.
f32 PlayerStateJump::calcJumpGravity() const {
    if (mCounterForceRun->getCounter() >= 1)
        return mConst->getJumpGravityForceRun();

    switch (_b0) {
    case 2:
        return mConst->getJumpGravity3rd();
    case 1:
        return mConst->getJumpGravity2nd();
    default:
        return mConst->getJumpGravity();
    }
}

// NON_MATCHING: target is 0x140 bytes and current is 0x144; zero initialization and the D8
// lifetime now match, but current branches on the speed test before computing the direction dot.
// Next hypothesis is a source form that materializes the direction-valid bool before the speed branch.
void PlayerStateJump::tryCountUpContinuousJump(PlayerContinuousJump* continuousJump) {
    if (!_b4 || mModelChanger->is2DModel()) {
        continuousJump->clear();
        return;
    }

    sead::Vector3f jumpDir = {0.0f, 0.0f, 0.0f};
    al::verticalizeVec(&jumpDir, al::getGravity(mActor), al::getVelocity(mActor));
    al::tryNormalizeOrZero(&jumpDir);

    if (continuousJump->getCount() != 0) {
        const f32 jumpPower = _a0;
        const f32 speedLimit = mConst->getJumpPowerMax() * 0.99f;
        const f32 dot = jumpDir.dot(continuousJump->mLastJumpDir);
        const bool isDirectionValid = mIs2D ? dot >= 6.123e-17f : dot >= 0.70711f;
        if (jumpPower < speedLimit || !isDirectionValid) {
            continuousJump->clear();
            return;
        }
    }

    continuousJump->countUp(jumpDir);
}

bool PlayerStateJump::isJumpCapCatch() const {
    if (isDead() || !al::isNerve(this, &NrvPlayerStateJump.Jump))
        return false;
    return mAnimator->isAnim("JumpCapCatch");
}

bool PlayerStateJump::isJumpSpinFlower() const {
    if (isDead())
        return false;
    return al::isNerve(this, &NrvPlayerStateJump.JumpSpinFlower) ||
           al::isNerve(this, &NrvPlayerStateJump.JumpSpinFlowerDownFall);
}

bool PlayerStateJump::isJumpSpinGround() const {
    if (isDead())
        return false;
    return al::isNerve(this, &NrvPlayerStateJump.JumpSpinGround) ||
           al::isNerve(this, &NrvPlayerStateJump.JumpSpinGroundDownFall);
}

bool PlayerStateJump::isJumpSpinGroundClockwise() const {
    return al::isEqualString(_c8, "SpinJumpR");
}

bool PlayerStateJump::isJumpBack() const {
    return !isDead() && al::isNerve(this, &NrvPlayerStateJump.JumpBack);
}

bool PlayerStateJump::isHovering() const {
    if (isDead())
        return false;
    return al::isNerve(this, &NrvPlayerStateJump.Hovering2D) ||
           al::isNerve(this, &NrvPlayerStateJump.HoveringJump2D);
}

bool PlayerStateJump::isEndJumpDownFallLand() const {
    if (!al::isNerve(this, &NrvPlayerStateJump.JumpSpinFlowerDownFall) &&
        !al::isNerve(this, &NrvPlayerStateJump.JumpSpinGroundDownFall))
        return false;

    if (!mIsDownFallGroundCollision)
        return true;
    if (!rs::isCollidedGround(mCollision))
        return false;

    const sead::Vector3f groundPos = rs::getCollidedGroundPos(mCollision);
    const sead::Vector3f& gravity = al::getGravity(mActor);
    return (groundPos - mDownFallGroundPos).dot(gravity) > 10.0f;
}

bool PlayerStateJump::isHoldDownFall() const {
    if (al::isNerve(this, &NrvPlayerStateJump.JumpSpinFlowerDownFall) ||
        al::isNerve(this, &NrvPlayerStateJump.JumpSpinGroundDownFall))
        return mInput->isHoldHipDrop();
    return false;
}

// NON_MATCHING: target is 0x90 bytes and current is 0xa0; behavior matches, but current
// materializes the boolean result in W8 instead of sharing the target direct-W0 true/false blocks.
// Next source-level hypothesis is a control-flow form that preserves those shared return blocks.
bool PlayerStateJump::isEnableHipDropStart() const {
    if (isDead())
        return true;
    if (al::isNerve(this, &NrvPlayerStateJump.JumpSpinGround) ||
        al::isNerve(this, &NrvPlayerStateJump.JumpSpinGroundDownFall))
        return false;
    if (isDead())
        return true;
    return !al::isNerve(this, &NrvPlayerStateJump.JumpSpinFlower) &&
           !al::isNerve(this, &NrvPlayerStateJump.JumpSpinFlowerDownFall);
}

bool PlayerStateJump::isEnableTrampleByHipDropAttack() const {
    return !isDead() &&
           (al::isNerve(this, &NrvPlayerStateJump.JumpSpinFlowerDownFall) ||
            al::isNerve(this, &NrvPlayerStateJump.JumpSpinGroundDownFall)) &&
           downFallConvergeCounter == 0;
}

bool PlayerStateJump::isEnableReactionCapCatch() const {
    return !isDead() && al::isNerve(this, &NrvPlayerStateJump.Jump);
}

bool PlayerStateJump::isEnableCancelCarryThrow() const {
    if (isJumpSpinFlower())
        return true;
    if (isJumpSpinGround())
        return true;
    return false;
}

bool PlayerStateJump::isFormSquat2D() const {
    if (isDead() || !mModelChanger->is2DModel())
        return false;
    if (_d0)
        return al::isEqualString(_d0, "JumpSquat");
    return mAnimator->isAnim("JumpSquat");
}

bool PlayerStateJump::trySubAnimJumpReaction() {
    PlayerAnimator* animator = mAnimator;
    if (!animator->isSubAnimPlaying())
        return false;
    if (_b8) {
        animator->endSubAnim();
        return false;
    }
    al::startHitReaction(mActor, "アクションジャンプ");
    return true;
}

// NON_MATCHING: target is 0xcc bytes and current is 0xd0; strings/decisions are recovered but
// the mode branch and final conditional-select lowering differ; next source-level hypothesis is
// to select the 3D action pair before the mode test so only the target final pair joins.
const char* PlayerStateJump::calcJumpAnimName() const {
    if (mModelChanger->is2DModel())
        return _d0 ? _d0 : "Jump";

    if (_c8)
        return _c8;

    if (_b0 == 2)
        return "Jump3";
    if (_b0 == 1)
        return "Jump2";
    if (_b0 != 0)
        return "Jump";

    const bool isSubAnimPlaying = mAnimator->isSubAnimPlaying();
    const bool isFront = rs::isPlayerSideFaceToCameraZ(mActor);
    if (isSubAnimPlaying)
        return isFront ? "JumpInterp" : "JumpReverseInterp";
    return isFront ? "Jump" : "JumpReverse";
}

void PlayerStateJump::exeJumpSpinFlowerDownFall() {
    updateNerveDownFall("SpinJumpDownFall", mConst->getSpinFlowerJumpDownFallInitSpeed(),
                        mConst->getSpinFlowerJumpDownFallPower(),
                        mConst->getSpinFlowerJumpDownFallSpeedMax(),
                        &NrvPlayerStateJump.JumpSpinFlower);
}

void PlayerStateJump::exeJumpSpinGroundDownFall() {
    const char* animationName =
        isJumpSpinGroundClockwise() ? "SpinJumpDownFallR" : "SpinJumpDownFallL";
    updateNerveDownFall(animationName, mConst->getSpinJumpDownFallInitSpeed(),
                        mConst->getSpinJumpDownFallPower(), mConst->getSpinJumpDownFallSpeedMax(),
                        &NrvPlayerStateJump.JumpSpinGround);
}

// NON_MATCHING: target/current are 0x190 bytes; behavior and register lifetimes match, but the
// setup block loads mAirMoveControl one instruction before the target vtable-slot load. Next
// hypothesis is a natural expression/lifetime form that swaps those adjacent independent loads.
void PlayerStateJump::exeHovering2D() {
    if (!mModelChanger->is2DModel()) {
        kill();
        return;
    }

    al::LiveActor* actor = mActor;
    const IUsePlayerCollision* collision = mCollision;
    const PlayerConst* pConst = mConst;
    const f32 maxSpeed = mModelChanger->is2DModel() ? pConst->getNormalMaxSpeed()
                                                    : pConst->getNormalMaxSpeed2D();
    rs::scaleVelocityInertiaWallHit(actor, collision, 0.25f, 1.0f, maxSpeed);

    if (al::isFirstStep(this)) {
        mAnimator->startAnim("Hovering");
        sead::Vector3f* velocity = al::getVelocityPtr(actor);
        al::verticalizeVec(velocity, al::getGravity(actor), *velocity);
        const f32 moveSpeed = _a4;
        PlayerActionAirMoveControl* airMoveControl = mAirMoveControl;
        const f32 normalMaxSpeed2D = mConst->getNormalMaxSpeed2D();
        airMoveControl->setup(moveSpeed, normalMaxSpeed2D, 0, 0.0f, 0.0f, 0, 0.0f);
    }

    mAirMoveControl->update();
    if (rs::isOnGround(actor, mCollision) || !mInput->isHoldCapSeparateJump() ||
        al::isGreaterEqualStep(this, 60))
        kill();
}

// NON_MATCHING: defaulted deleting destructor is 0x34 vs target 0x24; target bypasses the
// derived/ActorStateBase destructor thunk and calls NerveExecutor::~NerveExecutor directly before
// delete. Next source-level hypothesis is the original trivial base-destructor declaration shape
// that lets Clang fold the intermediate destructor without a hand-written base call.
PlayerStateJump::~PlayerStateJump() = default;
