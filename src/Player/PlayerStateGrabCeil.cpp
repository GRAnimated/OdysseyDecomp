#include "Player/PlayerStateGrabCeil.h"

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Se/SeFunction.h"

#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerActionCollisionSnap.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerJointParamGrab.h"
#include "Player/PlayerJudgePreInputJump.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerTrigger.h"
#include "Util/InputInterruptTutorialUtil.h"
#include "Util/JudgeUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(PlayerStateGrabCeil, Start)
NERVE_IMPL(PlayerStateGrabCeil, Jump)
NERVE_IMPL_(PlayerStateGrabCeil, Fall, Jump)
NERVE_IMPL(PlayerStateGrabCeil, Swing)
NERVE_IMPL(PlayerStateGrabCeil, Wait)
NERVES_MAKE_NOSTRUCT(PlayerStateGrabCeil, Start)
NERVES_MAKE_STRUCT(PlayerStateGrabCeil, Jump, Fall, Swing, Wait)


bool updateGrabCeilSwing(PlayerActionCollisionSnap* collisionSnap, f32* angle,
                         f32* angleVelocity, f32* sideAngle, f32* sideAngleVelocity,
                         s32* reverseCounter, f32* inputPower, s32* inputCounter,
                         const PlayerInput* input, const PlayerConst* playerConst,
                         const sead::Vector3f& front, const sead::Vector3f& side,
                         const sead::Vector3f& up, bool isSwing, bool isWaitSwing,
                         bool isInputReverse);
}  // namespace

PlayerStateGrabCeil::PlayerStateGrabCeil(
    al::LiveActor* actor, const PlayerConst* playerConst, const PlayerInput* input,
    const PlayerModelHolder* modelHolder, PlayerTrigger* trigger, IUsePlayerCollision* collision,
    PlayerAnimator* animator, PlayerJudgePreInputJump* judgePreInputJump, al::HitSensor* sensor,
    PlayerJointParamGrab* jointParamGrab)
    : al::ActorStateBase("天井ぶら下がり", actor), mConst(playerConst), mInput(input),
      mModelHolder(modelHolder), mTrigger(trigger), mCollision(collision), mAnimator(animator),
      mJudgePreInputJump(judgePreInputJump), mSensor(sensor), mJointParamGrab(jointParamGrab) {
    mAirMoveControl = new PlayerActionAirMoveControl(actor, playerConst, input, collision, false);
    mAirMoveControl->setupCollideWallScaleVelocity(mConst->getFallWallScaleVelocity(), 0.0f,
                                                   mConst->getNormalMaxSpeed());
    mCollisionSnap = new PlayerActionCollisionSnap(actor, collision);
    initNerve(&Start, 0);
}

// NON_MATCHING: target 1444 bytes, current 1428 bytes; inertial spring/local setup is close but not instruction-identical. next source-level hypothesis: recover the original max/abs expression and temporary ordering around the two spring-energy calculations.
void PlayerStateGrabCeil::appear() {
    _c0 = 1.0f;
    al::NerveStateBase::appear();
    _84 = 0.0f;
    _88 = 0.0f;
    _ac = 0;
    _b0 = 0.0f;
    _b4 = 0;

    al::LiveActor* actor = mActor;
    _94.set(mCollisionSnap->getSnapFront());
    _a0.setCross(_94, al::getGravity(actor));
    al::normalize(&_a0);

    sead::Vector3f inertia{0.0f, 0.0f, 0.0f};
    const sead::Vector3f& gravity = al::getGravity(actor);
    const sead::Vector3f& velocity = al::getVelocity(actor);
    al::limitVectorOppositeDir(&inertia, gravity, velocity, velocity.length());
    inertia *= 0.75f;
    al::limitLength(&inertia, inertia, 20.0f);

    const f32 frontSpeed = inertia.dot(_94);
    f32 frontEnergy = al::sign(-frontSpeed) *
                      sead::Mathf::max(sead::Mathf::abs(frontSpeed),
                                       sead::Mathf::max(inertia.dot(al::getGravity(actor)), 0.0f));
    f32 sideEnergy = inertia.dot(_a0);

    const sead::Vector3f position = al::getTrans(actor);
    mCollisionSnap->start();
    sead::Vector3f snapDelta = position - al::getTrans(actor);
    f32 frontSign = al::sign(frontEnergy);
    f32 sideSign = al::sign(sideEnergy);
    if (al::tryNormalizeOrZero(&snapDelta)) {
        if (al::isNearZero(frontSign, 0.001f))
            frontSign = -1.0f;
        f32 angle = sead::Mathf::abs(
            al::calcAngleOnPlaneDegreeOrZero(al::getGravity(actor), snapDelta, _a0));
        _84 = -frontSign * sead::Mathf::min(angle, 90.0f);

        if (al::isNearZero(sideSign, 0.001f))
            sideSign = -1.0f;
        angle = sead::Mathf::abs(
            al::calcAngleOnPlaneDegreeOrZero(al::getGravity(actor), snapDelta, _94));
        _88 = -sideSign * sead::Mathf::min(angle, 20.0f);
    }

    if (al::isNearZero(frontSign, 0.001f)) {
        _8c = -7.5f;
    } else {
        const f32 springSpeed = al::convertSpringEnergyToSpeed(_84, frontEnergy, 0.01f);
        const f32 remaining =
            sead::Mathf::sqrt(sead::Mathf::max(frontEnergy * frontEnergy - _84 * _84 * 0.01f, 0.0f));
        _8c = frontSign * (remaining + sead::Mathf::max(7.5f - springSpeed, 0.0f));
    }

    if (al::isNearZero(sideSign, 0.001f)) {
        _90 = -0.0f;
    } else {
        const f32 springSpeed = al::convertSpringEnergyToSpeed(_88, sideEnergy, 0.01f);
        const f32 remaining =
            sead::Mathf::sqrt(sead::Mathf::max(sideEnergy * sideEnergy - _88 * _88 * 0.01f, 0.0f));
        _90 = sideSign * (remaining + sead::Mathf::max(-springSpeed, 0.0f));
    }

    f32 angle = _84;
    f32 angleVelocity = 0.0f;
    f32 sideAngle = _88;
    f32 sideAngleVelocity = 0.0f;
    s32 reverseCounter = 0;
    f32 inputPower = 0.0f;
    s32 inputCounter = 0;
    const sead::Vector3f up = -al::getGravity(actor);
    updateGrabCeilSwing(mCollisionSnap, &angle, &angleVelocity, &sideAngle, &sideAngleVelocity,
                   &reverseCounter, &inputPower, &inputCounter, mInput, mConst, _94, _a0, up,
                   false, false, false);
    followCollision();

    if (mAnimator->isSubAnimPlaying())
        mAnimator->endSubAnim();
    if (mAnimator->isUpperBodyAnimAttached())
        mAnimator->clearUpperBodyAnim();
    mAnimator->startAnim("GrabCeilStart");
    sendMsgStartGrab();
    rs::tryAppearPlayerGrabPoleTutorial(actor);
    al::setNerve(this, &Start);
}

namespace {
// NON_MATCHING: target 1500 bytes, current 1416 bytes; input-power/counter damping control flow is still too compact. next source-level hypothesis: mirror the corpus branch ordering around inputCounter, reverse power, and spring damping without algebraic consolidation.
bool updateGrabCeilSwing(PlayerActionCollisionSnap* collisionSnap, f32* angle,
                    f32* angleVelocity, f32* sideAngle, f32* sideAngleVelocity,
                    s32* reverseCounter, f32* inputPower, s32* inputCounter,
                    const PlayerInput* input, const PlayerConst* playerConst,
                    const sead::Vector3f& front, const sead::Vector3f& side,
                    const sead::Vector3f& up, bool isSwing, bool isWaitSwing,
                    bool isInputReverse) {
    *angle += *angleVelocity;
    *sideAngle += *sideAngleVelocity;

    const f32 springSpeed = al::convertSpringEnergyToSpeed(*angle, *angleVelocity, 0.01f);
    const bool isMovingAway = al::isNearZeroOrGreater(*angle * *angleVelocity, 0.001f);
    if (isMovingAway)
        *reverseCounter = 45;
    else
        *reverseCounter = al::converge(*reverseCounter, 0, 1);

    sead::Vector3f snapUp = front;
    sead::Vector3f snapFront = front.cross(side);
    al::rotateVectorDegree(&snapFront, snapFront, front, *sideAngle);
    al::rotateVectorDegree(&snapFront, snapFront, side, *angle);
    al::normalize(&snapFront);
    al::rotateVectorDegree(&snapUp, snapUp, side, *angle);
    al::normalize(&snapUp);
    collisionSnap->setSnapPose(snapUp, snapFront);

    sead::Vector3f followDir{0.0f, 0.0f, 0.0f};
    collisionSnap->calcFollowDir(&followDir, front);
    sead::Vector3f moveInput{0.0f, 0.0f, 0.0f};
    input->calcMoveInput(&moveInput, up);

    const f32 inputDot = moveInput.dot(followDir);
    f32 swingInput = 0.0f;
    if ((inputDot > 0.0f ? inputDot : -inputDot) >= 0.17365f) {
        f32 inputLength = moveInput.length();
        if (!isSwing && input->isMove())
            inputLength = 1.0f;
        swingInput = -inputLength * al::sign(inputDot);
        if (isWaitSwing && input->isMove() && !al::isSameSign(*angleVelocity, swingInput))
            *angleVelocity = -*angleVelocity;
    }

    const f32 previousInputPower = *inputPower;
    if (!al::isNearZero(swingInput, 0.001f)) {
        if (al::isNearZero(*inputPower, 0.001f)) {
            *inputPower = swingInput;
        } else if (swingInput * *inputPower < 0.0f) {
            *inputPower = swingInput;
            *inputCounter = 45;
        } else {
            *inputCounter = al::converge(*inputCounter, 0, 1);
        }
    } else {
        *inputCounter = al::converge(*inputCounter, 0, 1);
    }

    if (*inputCounter >= 1) {
        const f32 absSwingInput = swingInput > 0.0f ? swingInput : -swingInput;
        const f32 absInputPower = *inputPower > 0.0f ? *inputPower : -*inputPower;
        if (absSwingInput > absInputPower) {
            *inputPower = absSwingInput * al::sign(*inputPower);
        } else if (al::isNearZero(absSwingInput, 0.001f)) {
            swingInput = *inputPower;
        } else {
            swingInput = absInputPower * al::sign(swingInput);
        }
    } else if (*inputCounter == 0 && !al::isNearZero(previousInputPower, 0.001f)) {
        *inputPower = swingInput;
    }

    f32 damping = 0.005f;
    const f32 inputVelocity = swingInput * *angleVelocity;
    bool isReversePower = false;
    if (inputVelocity < 0.0f && *inputCounter == 0 &&
        (springSpeed >= playerConst->getGrabCeilReverseInputBorder() || isSwing)) {
        damping = (isMovingAway || *reverseCounter == 0) ? 0.06f : 0.005f;
        isReversePower = true;
    }

    const f32 absInput = swingInput > 0.0f ? swingInput : -swingInput;
    const f32 inputRate = al::clamp(absInput, 0.0f, 1.0f);
    const f32 springDamping = al::lerpValue(0.06f, damping, inputRate);
    const f32 angleDamping = isInputReverse ? 0.0f : springDamping;
    const f32 springForce =
        al::calcSpringDumperForce(*angle, *angleVelocity, 0.01f, angleDamping);

    f32 inputForce = 0.0f;
    const f32 absAngle = *angle > 0.0f ? *angle : -*angle;
    if (!isReversePower && absAngle < 90.0f) {
        f32 rate = 1.0f;
        if (!isSwing) {
            rate = al::clamp(springSpeed / playerConst->getGrabCeilInputPowerBorder(), 0.0f,
                             1.0f);
            rate = al::easeIn(rate);
        }
        if (isWaitSwing)
            inputForce = swingInput * al::lerpValue(3.5f, 1.2f, rate);
        else
            inputForce = swingInput * al::lerpValue(0.5f, 0.08f, rate);
    }

    const f32 addForce = inputVelocity < 0.0f ? -inputForce : inputForce;
    *angleVelocity += springForce + addForce;
    *sideAngleVelocity +=
        al::calcSpringDumperForce(*sideAngle, *sideAngleVelocity, 0.01f, 0.05f);

    const f32 absAddForce = addForce > 0.0f ? addForce : -addForce;
    if (absAddForce <= 0.0f)
        return false;
    if (inputVelocity < 0.0f)
        return *inputCounter != 0;
    return true;
}
}  // namespace

bool PlayerStateGrabCeil::followCollision() {
    mCollisionSnap->followCollision();
    PlayerJointParamGrab* jointParam = mJointParamGrab;
    const f32 angleSide = _84;
    const f32 angleFront = _88;
    jointParam->poseRate = _c0;

    sead::Vector3f direction = _94.cross(_a0);
    al::rotateVectorDegree(&direction, direction, _94, -angleFront);
    al::rotateVectorDegree(&direction, direction, _a0, angleSide);
    al::normalize(&jointParam->direction, direction);
    return mCollisionSnap->isSnapPartsValid();
}


void PlayerStateGrabCeil::sendMsgStartGrab() {
    al::HitSensor* connectedSensor = mCollisionSnap->tryGetConnectedSensor();
    if (connectedSensor)
        rs::sendMsgPlayerStartGrabCeil(connectedSensor, mSensor);
}

void PlayerStateGrabCeil::kill() {
    sendMsgEndGrab();

    PlayerJointParamGrab* jointParam = mJointParamGrab;
    jointParam->poseRate = 0.0f;
    jointParam->direction = sead::Vector3f::ey;
    jointParam->interpolateRate = 0.0f;
    rs::tryClosePlayerGrabPoleTutorial(mActor);

    if (mTrigger->isOn(PlayerTrigger::EPreMovementTrigger_val3)) {
        al::LiveActor* actor = mActor;
        const sead::Vector3f up = -al::getGravity(actor);
        sead::Quatf quat = sead::Quatf::unit;
        al::makeQuatUpFront(&quat, up, _94);
        al::updatePoseQuat(actor, quat);
        al::resetPosition(actor, al::getTrans(actor) + up * 0.0f);
        mTrigger->set(PlayerTrigger::EActionTrigger_val13);
    }

    al::NerveStateBase::kill();
}

void PlayerStateGrabCeil::sendMsgEndGrab() {
    al::HitSensor* connectedSensor = mCollisionSnap->tryGetConnectedSensor();
    if (connectedSensor)
        rs::sendMsgPlayerEndGrabCeil(connectedSensor, mSensor);
}

void PlayerStateGrabCeil::setup(const al::CollisionParts* parts, const sead::Vector3f& position,
                                const sead::Vector3f& front, const sead::Vector3f& up) {
    mCollisionSnap->setup(parts, position, front, up);
}

// NON_MATCHING: target/current 112 bytes; boolean complement lowers as EOR plus epilogue normalization instead of target MVN/AND. next source-level hypothesis: preserve the raw isGreaterStep result in an integer temporary before complementing bit 0.
bool PlayerStateGrabCeil::isFormGrabCeil() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvPlayerStateGrabCeil.Jump) ||
        al::isNerve(this, &NrvPlayerStateGrabCeil.Fall))
        return !al::isGreaterStep(this, 0);
    return true;
}

bool PlayerStateGrabCeil::isJump() const {
    if (isDead())
        return false;
    if (!al::isNerve(this, &NrvPlayerStateGrabCeil.Jump) &&
        !al::isNerve(this, &NrvPlayerStateGrabCeil.Fall))
        return false;
    return al::isGreaterStep(this, 0);
}

bool PlayerStateGrabCeil::isEnableNextGrabCeil() const {
    if (!isJump())
        return false;
    return al::isGreaterStep(this, mConst->getGrabCeilEnableNextFrame());
}

bool PlayerStateGrabCeil::isEnableSnapForce() const {
    if (!isJump())
        return false;
    const s32 frame = al::isNerve(this, &NrvPlayerStateGrabCeil.Fall)
                          ? mConst->getGrabCeilEnableFallSnapFrame()
                          : mConst->getGrabCeilEnableNextFrame();
    return al::isGreaterStep(this, frame);
}

bool PlayerStateGrabCeil::isEnableTrample() const {
    return true;
}

const sead::Vector3f& PlayerStateGrabCeil::getGrabCeilFront() const {
    return mCollisionSnap->getSnapFront();
}

// NON_MATCHING: 980 bytes vs target 1060; target keeps different stack/register lifetimes through the swing update and terminal nerve branches; next source-level hypothesis: mirror the target local pointer lifetimes and branch nesting instead of the current consolidated control flow.
void PlayerStateGrabCeil::exeStart() {
    if (al::isFirstStep(this)) {
        _78 = false;
        _79 = true;
        const f32 energy = al::convertSpringEnergyToSpeed(_84, _8c, 0.01f);
        _80 = mConst->getGrabCeilEnableJumpEnergy() > energy;
        _7c = 0;
    }

    if (_79)
        _79 = al::isNearZeroOrLess(al::sign(_84 * _8c), 0.001f);

    const bool canLeave = al::isGreaterEqualStep(this, 10);
    const bool isJumpInput = rs::judgeAndResetReturnTrue(mJudgePreInputJump);
    if (canLeave) {
        if (isJumpInput || _78) {
            leaveGrabCeil(true);
            return;
        }
    } else if (isJumpInput) {
        _78 = true;
    }

    const f32 prevAngle = _84;
    const sead::Vector3f up = -al::getGravity(mActor);
    const bool isSwingInput =
        updateGrabCeilSwing(mCollisionSnap, &_84, &_8c, &_88, &_90, &_ac, &_b0, &_b4,
                       mInput, mConst, _94, _a0, up, false, _80, _79);
    const f32 energy = al::convertSpringEnergyToSpeed(_84, _8c, 0.01f);
    updateWaitSwingFlag(isSwingInput, energy);

    const f32 border = _84 >= prevAngle ? 30.0f : -30.0f;
    if ((_84 + border) * (prevAngle + border) < -0.1f)
        al::tryStartSe(mActor, prevAngle >= 0.0f ? "GrabCeilSwingForward" : "GrabCeilSwingBack");

    _c0 = al::converge(_c0, 0.0f, 0.022222f);
    if (!followCollision()) {
        kill();
        return;
    }

    if (mInput->isMove()) {
        if (!al::isGreaterEqualStep(this, 10))
            return;
    } else {
        if (!mAnimator->isAnimEnd())
            return;
        if (!mInput->isMove()) {
            al::setNerve(this, &NrvPlayerStateGrabCeil.Wait);
            return;
        }
    }

    if (mConst->getGrabCeilEnableJumpEnergy() <= energy)
        al::setNerve(this, &NrvPlayerStateGrabCeil.Swing);
    else
        al::setNerve(this, &NrvPlayerStateGrabCeil.Wait);
}


// NON_MATCHING: 1264 bytes vs target 1284; target explicitly calls al::getGravity(actor) before the zero-initialized calcUpDir path and keeps different local lifetimes; next source-level hypothesis: recover the source expression that forces that gravity call and target stack/register schedule.
void PlayerStateGrabCeil::leaveGrabCeil(bool isJumpInput) {
    followCollision();

    al::LiveActor* actor = mActor;
    const f32 leaveSpeedMin = mConst->getGrabCeilLeaveSpeedMin();
    sead::Vector3f up{0.0f, 0.0f, 0.0f};
    al::calcUpDir(&up, actor);
    const f32 energy = al::convertSpringEnergyToSpeed(_84, _8c, 0.01f);
    const bool doJump = isJumpInput && mConst->getGrabCeilEnableJumpEnergy() <= energy;
    _bc = 0;

    sead::Vector3f followDir{0.0f, 0.0f, 0.0f};
    mCollisionSnap->calcFollowDir(&followDir, _94);

    if (!doJump) {
        mAnimator->startAnim("Fall");
        const f32 collisionRadius = mConst->getCollisionRadius();
        const f32 popPower = mConst->getGrabCeilLeavePopPower();
        sead::Vector3f offset{0.0f, 0.0f, 0.0f};
        rs::calcOffsetAllRoot(&offset, mModelHolder);
        mCollisionSnap->endFall(popPower, offset, collisionRadius);
        mAnimator->clearInterpolation();
        al::addVelocityToDirection(actor, followDir, -leaveSpeedMin);
        _b8 = mConst->getGrabCeilLeavePopGravity();
        al::setNerve(this, &NrvPlayerStateGrabCeil.Fall);
        return;
    }

    f32 directionSign = 1.0f;
    if (!_79) {
        f32 signValue = -_84;
        const f32 absAngle = sead::Mathf::abs(_84);
        if (absAngle <= 10.0f)
            signValue = -_8c;
        directionSign = al::sign(signValue);
    }

    mAnimator->startAnim("GrabCeilJump");
    mAnimator->clearInterpolation();
    const f32 forceAngle = -directionSign * mConst->getGrabCeilJumpForceAngle();
    sead::Vector3f snapFront = _94.cross(_a0);
    al::rotateVectorDegree(&snapFront, snapFront, _94, 0.0f);
    al::rotateVectorDegree(&snapFront, snapFront, _a0, forceAngle);
    al::normalize(&snapFront);
    sead::Vector3f snapUp = _94;
    al::rotateVectorDegree(&snapUp, snapUp, _a0, forceAngle);
    al::normalize(&snapUp);
    mCollisionSnap->setSnapPose(snapUp, snapFront);
    mCollisionSnap->followCollision();
    rs::resetCollision(mCollision);

    sead::Vector3f localOffset{0.0f, 0.0f, 0.0f};
    rs::calcLocalOffsetAllRoot(&localOffset, mModelHolder);
    sead::Matrix34f pose = sead::Matrix34f::ident;
    al::makeMtxSRT(&pose, actor);
    sead::Vector3f offset{0.0f, 0.0f, 0.0f};
    offset.setMul(pose, localOffset);
    mCollisionSnap->endFall(mConst->getGrabCeilJumpPower(), offset, mConst->getCollisionRadius());

    const f32 rate = al::calcRate01(energy, mConst->getGrabCeilEnableJumpEnergy(),
                                    mConst->getGrabCeilEnableJumpEnergyMax());
    const f32 moveSpeed =
        al::lerpValue(mConst->getGrabCeilJumpMoveMin(), mConst->getGrabCeilJumpMoveMax(), rate);
    al::addVelocityToDirection(actor, followDir, directionSign * moveSpeed);
    _b8 = mConst->getGrabCeilJumpGravity();
    _bc = mConst->getGrabCeilJumpInvalidFrame();
    al::setNerve(this, &NrvPlayerStateGrabCeil.Jump);
}

void PlayerStateGrabCeil::updateWaitSwingFlag(bool isSwingInput, f32 energy) {
    if (_80) {
        if (_7c > 0 || isSwingInput) {
            _7c = al::converge(_7c, 3, 1);
            if (_7c >= 3)
                _80 = false;
        }
    } else {
        if (mInput->isMove() || mConst->getGrabCeilSwingWaitEnergy() <= energy) {
            _80 = false;
            return;
        }
        _80 = _b4 < 1;
        if (_b4 <= 0)
            _7c = 0;
    }
}

// NON_MATCHING: target 916 bytes, current 848 bytes; wait behavior is recovered but helper-result/bookkeeping branches are over-consolidated. next source-level hypothesis: mirror the corpus updateWaitSwingFlag and swing-SE branch ordering while retaining same-TU calls where Clang inlines them.
void PlayerStateGrabCeil::exeWait() {
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("GrabCeilWait");
        rs::resetJudge(mJudgePreInputJump);
    }

    if (_79)
        _79 = al::isNearZeroOrLess(al::sign(_84 * _8c), 0.001f);
    if (rs::judgeAndResetReturnTrue(mJudgePreInputJump)) {
        leaveGrabCeil(false);
        return;
    }

    const f32 prevAngle = _84;
    const sead::Vector3f up = -al::getGravity(mActor);
    const bool isSwingInput =
        updateGrabCeilSwing(mCollisionSnap, &_84, &_8c, &_88, &_90, &_ac, &_b0, &_b4,
                       mInput, mConst, _94, _a0, up, false, _80, _79);
    const f32 energy = al::convertSpringEnergyToSpeed(_84, _8c, 0.01f);
    updateWaitSwingFlag(isSwingInput, energy);

    const f32 border = _84 >= prevAngle ? 30.0f : -30.0f;
    if ((_84 + border) * (prevAngle + border) < -0.1f)
        al::tryStartSe(mActor, prevAngle >= 0.0f ? "GrabCeilSwingForward" : "GrabCeilSwingBack");

    _c0 = al::converge(_c0, 0.0f, 0.022222f);
    if (!followCollision()) {
        kill();
        return;
    }

    if (isSwingInput &&
        mConst->getGrabCeilEnableJumpEnergy() <= energy - mConst->getGrabCeilSwingStartOffset())
        al::setNerve(this, &NrvPlayerStateGrabCeil.Swing);
}

// NON_MATCHING: target 1068 bytes, current 952 bytes; swing behavior is recovered but energy/frame calculation and bookkeeping are too compact. next source-level hypothesis: mirror the target energy-limit and animator-frame temporary order before the wait transition.
void PlayerStateGrabCeil::exeSwing() {
    if (al::isFirstStep(this))
        mAnimator->startAnim("GrabCeilSwing");

    if (_79)
        _79 = al::isNearZeroOrLess(al::sign(_84 * _8c), 0.001f);
    if (rs::judgeAndResetReturnTrue(mJudgePreInputJump)) {
        leaveGrabCeil(true);
        return;
    }

    const f32 prevAngle = _84;
    const sead::Vector3f up = -al::getGravity(mActor);
    const bool isSwingInput =
        updateGrabCeilSwing(mCollisionSnap, &_84, &_8c, &_88, &_90, &_ac, &_b0, &_b4,
                       mInput, mConst, _94, _a0, up, true, _80, _79);
    const f32 energy = al::convertSpringEnergyToSpeed(_84, _8c, 0.01f);
    updateWaitSwingFlag(isSwingInput, energy);

    const sead::Vector2f swingEnergy{_84, _8c * 10.0f};
    const f32 energyLimit = swingEnergy.length();
    const f32 angleRate = al::calcRate01(_84, -energyLimit, energyLimit);
    const f32 direction = _84 - prevAngle > 0.0f ? 1.0f : 0.0f;
    const f32 frameRate = al::lerpValue(0.5f, direction, angleRate);
    mAnimator->setAnimFrame(frameRate * mAnimator->getAnimFrameMax());

    const f32 border = _84 >= prevAngle ? 30.0f : -30.0f;
    if ((_84 + border) * (prevAngle + border) < -0.1f)
        al::tryStartSe(mActor, prevAngle >= 0.0f ? "GrabCeilSwingForward" : "GrabCeilSwingBack");

    _c0 = al::converge(_c0, 0.0f, 0.022222f);
    if (!followCollision()) {
        kill();
        return;
    }

    if (mConst->getGrabCeilEnableJumpEnergy() > energy)
        al::setNerve(this, &NrvPlayerStateGrabCeil.Wait);
}

void PlayerStateGrabCeil::exeJump() {
    if (al::isFirstStep(this)) {
        mAirMoveControl->setup(mConst->getJumpMoveSpeedMax(), mConst->getJumpMoveSpeedMin(), 0,
                               al::calcSpeedV(mActor), _b8, _bc, 0.0f);
        PlayerJointParamGrab* jointParam = mJointParamGrab;
        jointParam->poseRate = 0.0f;
        jointParam->direction = sead::Vector3f::ey;
        jointParam->interpolateRate = 0.0f;
    }

    mAirMoveControl->update();
    if (rs::isOnGround(mActor, mCollision))
        kill();
}

PlayerStateGrabCeil::~PlayerStateGrabCeil() = default;

