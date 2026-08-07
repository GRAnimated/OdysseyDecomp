#include "Player/PlayerActionGroundMoveControl.h"

#include "Library/Collision/Collider.h"
#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Se/SeFunction.h"

#include "Player/IUsePlayerHack.h"
#include "Player/PlayerActionFunction.h"
#include "Player/PlayerActionTurnControl.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerInput.h"
#include "Util/JudgeUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackInputFunction.h"

namespace {
bool adjustGroundVelocityForWall(sead::Vector3f*, const sead::Vector3f&,
                                 const IUsePlayerCollision*, const sead::Vector3f&);
}

PlayerActionGroundMoveControl::PlayerActionGroundMoveControl(
    al::LiveActor* parent, const PlayerConst* playerConst, const PlayerInput* playerInput,
    const IUsePlayerCollision* collision)
    : mParent(parent), mPlayerConst(playerConst), mPlayerInput(playerInput), mCollision(collision),
      mHack(nullptr), mIsSetup(false), mGroundNormal(sead::Vector3f::zero), mMaxSpeed(0.0f),
      mMinSpeed(0.0f), _40(0.0f), _44(0.0f), mRunFrame(0), mStickOnBrakeFrame(0), mBrakeFrame(0),
      mGravityMove(0.0f), mBrakeSpeed(0.0f), mCounterBorder(0), _60(0), _64(false),
      mJudge(nullptr), _70(0.0f), _74(0), _78(false), _7c(0.0f), mHasStopped(false),
      _84(sead::Vector3f::zero), _90(sead::Vector3f::zero), mIsForceRunCtrlActive(false), _a0(0.0f), mAlwaysFalse(false), _a8(0.0f), _ac(0.0f),
      mTurnCtrl(new PlayerActionTurnControl(parent)), _b8(false), _b9(false), _ba(false),
      _bb(false), _bc(false), _bd(false), _c0(0.0f), _c4(false), _d4(false), _d5(false) {}

void PlayerActionGroundMoveControl::initDash(IJudge* judge, f32 speed, s32 frame) {
    mJudge = judge;
    _70 = speed;
    _74 = frame;
}

void PlayerActionGroundMoveControl::setupDash(f32 speed, s32 frame) {
    _70 = speed;
    _74 = frame;
}

void PlayerActionGroundMoveControl::setup(f32 maxSpeed, f32 minSpeed, s32 runFrame,
                                          s32 stickOnBrakeFrame, s32 brakeFrame, f32 gravityMove,
                                          f32 brakeSpeed, s32 counterBorder) {
    mIsSetup = true;
    mMaxSpeed = maxSpeed;
    mMinSpeed = minSpeed;
    mRunFrame = runFrame;
    mStickOnBrakeFrame = stickOnBrakeFrame;
    mBrakeFrame = brakeFrame;
    mGravityMove = gravityMove;
    mHasStopped = false;
    mBrakeSpeed = brakeSpeed;
    mCounterBorder = counterBorder;
}

void PlayerActionGroundMoveControl::appear() {
    sead::Vector3f normal = {0.0f, 0.0f, 0.0f};
    rs::calcGroundNormalExceptJustLandOrGravityDir(&normal, mParent, mCollision);
    rs::calcGroundNormalOrGravityDir(&mGroundNormal, mParent, mCollision);
    _c8 = mGroundNormal;

    sead::Vector3f velocity = {0.0f, 0.0f, 0.0f};
    al::alongVectorNormalH(&velocity, al::getVelocity(mParent), normal, mGroundNormal);

    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    al::calcFrontDir(&front, mParent);
    al::verticalizeVec(&front, mGroundNormal, front);
    al::tryNormalizeOrZero(&front);

    al::setVelocity(mParent, velocity - mGravityMove * mGroundNormal);
    _c8 = mGroundNormal;
    _7c = 0.0f;
    calcInitBrakeOnCounter();
    mTurnCtrl->reset();
    _c0 = 0.0f;
}

// NON_MATCHING: exact 0x50 size; `set(const Vector3f&)` changes the target scalar load/store
// schedule into an unaligned pair copy before reloading for `_c8`. Next hypothesis is a sanctioned
// sead helper form that preserves the three loaded components across both destinations.
void PlayerActionGroundMoveControl::reset(const sead::Vector3f& groundNormal) {
    mGroundNormal.set(groundNormal);
    _c8 = mGroundNormal;
    _7c = 0.0f;
    calcInitBrakeOnCounter();
    mTurnCtrl->reset();
    _c0 = 0.0f;
}

void PlayerActionGroundMoveControl::calcInitBrakeOnCounter() {
    _60 = 0;
    sead::Vector3f velocity;
    updateNormalAndSnap(&velocity);
    const f32 speed = velocity.length();
    if (rs::isOnGroundSkateCode(mParent, mCollision) && !al::isNearZero(speed, 0.001f)) {
        _60 = mCounterBorder;
        return;
    }

    if (speed < mBrakeSpeed || !al::tryNormalizeOrZero(&velocity))
        return;

    sead::Vector3f front;
    al::calcFrontDir(&front, mParent);
    al::verticalizeVec(&front, mGroundNormal, front);
    if (al::tryNormalizeOrZero(&front) && !PlayerActionFunction::isOppositeDir(front, velocity))
        _60 = mCounterBorder;
}

f32 PlayerActionGroundMoveControl::update() {
    if (rs::isOnGroundSkateCode(mParent, mCollision))
        return updateSkateMove();
    return updateNormalMove();
}

// NON_MATCHING: faithful corpus implementation; exact register allocation and scheduling pending.
f32 PlayerActionGroundMoveControl::updateSkateMove() {
    if (mJudge)
        rs::updateJudge(mJudge);
    _64 = false;
    mHasStopped = false;
    _bc = false;

    mTurnCtrl->setup(mPlayerConst->getIceRoundMinDegree(),
                     mPlayerConst->getIceRoundFastDegree(),
                     mPlayerConst->getIceRoundLimitDegree(),
                     mPlayerConst->getIceRoundLimitDegree(),
                     mPlayerConst->getIceRoundAccelFrame(),
                     mPlayerConst->getIceRoundAccelFrameFast(),
                     mPlayerConst->getIceRoundBrakeFrame());
    mTurnCtrl->set_88(true);
    _c0 = 0.0f;

    sead::Vector3f velocity = sead::Vector3f::zero;
    updateNormalAndSnap(&velocity);
    sead::Vector3f velocityDir = sead::Vector3f::zero;
    al::tryNormalizeOrZero(&velocityDir, velocity);

    sead::Vector3f moveInput = sead::Vector3f::zero;
    calcMoveInput(&moveInput, mGroundNormal);
    _84 = moveInput;

    sead::Vector3f moveDir = sead::Vector3f::zero;
    const bool hasMoveInput = al::tryNormalizeOrZero(&moveDir, moveInput);
    mTurnCtrl->update(_b8 ? moveInput : sead::Vector3f::zero, mGroundNormal);

    sead::Vector3f front = hasMoveInput ? moveDir : mTurnCtrl->get_5c();
    if (mIsForceRunCtrlActive) {
        moveInput = front;
        moveDir = front;
    }

    const sead::Vector3f& wallNormal =
        rs::isCollidedWall(mCollision) ? rs::getCollidedWallNormal(mCollision)
                                       : sead::Vector3f::zero;
    updateHillAffect(velocityDir, front, hasMoveInput);

    sead::Vector3f slideDir = sead::Vector3f::zero;
    al::calcDirSlide(&slideDir, al::getGravity(mParent), mGroundNormal);
    f32 slideRate = 0.0f;
    const bool isSlideZero = al::isNearZero(slideDir, 0.001f);
    if (!isSlideZero) {
        slideRate = sead::Mathf::clamp(
            (90.0f - al::calcAngleDegree(al::getGravity(mParent), slideDir)) / 30.0f,
            0.0f, 1.0f);
    }

    _60 = sead::Mathi::min(_60 + 1, mCounterBorder);

    sead::Vector3f dirAcross = front;
    bool hasAcrossInput = hasMoveInput;
    if (!isSlideZero) {
        al::verticalizeVec(&dirAcross, slideDir, front);
        if (!al::tryNormalizeOrZero(&dirAcross)) {
            dirAcross.setCross(mGroundNormal, front);
            al::normalize(&dirAcross);
            hasAcrossInput = false;
        }
    }

    f32 speedAcross = dirAcross.dot(velocity);
    const bool squatBrake = isActiveSquatBrake();
    const bool reverseAcross = speedAcross < -mMinSpeed;
    const bool wallBrakeAcross = wallNormal.dot(speedAcross * dirAcross) < 0.0f && !squatBrake;
    f32 targetAcross = 0.0f;
    if (speedAcross >= -mMinSpeed && hasAcrossInput) {
        const f32 inputRate = sead::Mathf::clamp(dirAcross.dot(moveInput), 0.0f, 1.0f);
        targetAcross = inputRate * calcMaxSpeed(speedAcross) + (1.0f - inputRate) * mMinSpeed;
        const s32 accelFrame = mPlayerConst->getIceAccelFrame();
        speedAcross = PlayerActionFunction::accel(
            speedAcross, targetAcross, mMaxSpeed / (f32)accelFrame);
    }

    s32 brakeFrameAcross = 1;
    if (wallBrakeAcross)
        brakeFrameAcross = mPlayerConst->getIceBrakeFrameWall();
    else if (hasMoveInput && reverseAcross)
        brakeFrameAcross = mPlayerConst->getIceBrakeFrameHigh();
    else if (_60 >= mCounterBorder || hasMoveInput || squatBrake)
        brakeFrameAcross = mPlayerConst->getIceBrakeFrame();
    if (sead::Mathf::abs(speedAcross) > targetAcross)
        speedAcross = PlayerActionFunction::brake(speedAcross, brakeFrameAcross, mMaxSpeed);

    sead::Vector3f dirSlide = sead::Vector3f::zero;
    bool hasSlideInput = hasMoveInput;
    if (!isSlideZero) {
        al::parallelizeVec(&dirSlide, slideDir, front);
    }
    if (isSlideZero || !al::tryNormalizeOrZero(&dirSlide)) {
        dirSlide.setCross(mGroundNormal, front);
        al::normalize(&dirSlide);
        hasSlideInput = false;
    }

    f32 speedSlide = dirSlide.dot(velocity);
    const bool reverseSlide = hasMoveInput && speedSlide < -mMinSpeed;
    const bool wallBrakeSlide = wallNormal.dot(speedSlide * dirSlide) < 0.0f && !squatBrake;
    f32 targetSlide = 0.0f;
    if (!reverseSlide && hasSlideInput) {
        const f32 slideBias = dirSlide.dot(slideDir) < 0.0f ? -0.5f : 0.75f;
        const f32 inputRate = sead::Mathf::clamp(
            dirSlide.dot(moveInput) + slideRate * slideBias, 0.0f, 1.0f);
        targetSlide = inputRate * calcMaxSpeed(speedSlide);
        const s32 accelFrame = mPlayerConst->getIceAccelFrame();
        speedSlide = PlayerActionFunction::accel(
            speedSlide, targetSlide,
            (slideRate + 1.0f) * (mMaxSpeed / (f32)accelFrame));
    }

    s32 brakeFrameSlide = 1;
    if (wallBrakeSlide)
        brakeFrameSlide = mPlayerConst->getIceBrakeFrameWall();
    else if (reverseSlide)
        brakeFrameSlide = mPlayerConst->getIceBrakeFrameHigh();
    else if (_60 >= mCounterBorder || hasMoveInput || squatBrake)
        brakeFrameSlide = mPlayerConst->getIceBrakeFrame();
    if (sead::Mathf::abs(speedSlide) > targetSlide)
        speedSlide = PlayerActionFunction::brake(speedSlide, brakeFrameSlide, mMaxSpeed);

    mHasStopped = !hasMoveInput &&
                  al::isNearZero(sead::Mathf::abs(speedAcross) + sead::Mathf::abs(speedSlide),
                                 0.001f);

    sead::Vector3f nextVelocity = speedAcross * dirAcross + speedSlide * dirSlide;
    nextVelocity -= mGravityMove * mGroundNormal;
    al::setVelocity(mParent, nextVelocity);

    const sead::Vector2f speedComponents = {speedAcross, speedSlide};
    const f32 speed = speedComponents.length();
    if (speed > 3.0f && nextVelocity.dot(front) < 0.0f)
        al::holdSe(mParent, "SlipIceLv");

    const sead::Vector3f poseFront = _b8 ? mTurnCtrl->get_5c() : front;
    updatePoseUpFront(mGroundNormal, poseFront, speed);
    _90 = poseFront;
    return speed;
}

// NON_MATCHING: faithful corpus implementation; exact register allocation and scheduling pending.
f32 PlayerActionGroundMoveControl::updateNormalMove() {
    if (mJudge)
        rs::updateJudge(mJudge);
    _64 = false;
    mHasStopped = false;
    _bc = false;

    sead::Vector3f velocity = sead::Vector3f::zero;
    updateNormalAndSnap(&velocity);
    f32 speed = velocity.length();
    if (speed < 0.1f) {
        velocity = sead::Vector3f::zero;
        speed = 0.0f;
    }

    sead::Vector3f velocityDir = sead::Vector3f::zero;
    al::tryNormalizeOrZero(&velocityDir, velocity);

    const f32 roundRate = sead::Mathf::clamp(
        1.0f - al::calcRate01(speed,
                              mPlayerConst->getDashBorderSpeed() -
                                  mPlayerConst->getDashBlendRange(),
                              mPlayerConst->getDashBorderSpeed() +
                                  mPlayerConst->getDashBlendRange()),
        0.0f, 1.0f);
    f32 turnLimit = al::lerpValue(mPlayerConst->getRoundLimitDegreeMin(),
                                  mPlayerConst->getRoundLimitDegree(), roundRate);
    f32 turnLimitFast = turnLimit;
    f32 turnFast = mPlayerConst->getRoundFastDegree();
    s32 turnAccelFast = mPlayerConst->getRoundAccelFrameFast();
    s32 turnBrake = mPlayerConst->getRoundBrakeFrame();
    if (mIsForceRunCtrlActive) {
        turnLimit = mPlayerConst->getRoundLimitDegreeForce();
        turnLimitFast = mPlayerConst->getRoundLimitDegreeForceFast();
        turnFast = mPlayerConst->getRoundFastDegreeForce();
        turnAccelFast = mPlayerConst->getRoundAccelFrameForceFast();
        turnBrake = mPlayerConst->getRoundBrakeFrameForce();
    }
    if (_ba) {
        turnLimit = 0.0f;
        turnLimitFast = 0.0f;
    }
    mTurnCtrl->setup(mPlayerConst->getRoundMinDegree(), turnFast, turnLimit,
                     turnLimitFast, mPlayerConst->getRoundAccelFrame(), turnAccelFast,
                     turnBrake);
    mTurnCtrl->set_88(_b9 || mIsForceRunCtrlActive);

    sead::Vector3f moveInput = sead::Vector3f::zero;
    calcMoveInput(&moveInput, mGroundNormal);
    _84 = moveInput;

    sead::Vector3f moveDir = sead::Vector3f::zero;
    const bool hasMoveInput = al::tryNormalizeOrZero(&moveDir, moveInput);
    mTurnCtrl->update(moveInput, mGroundNormal);

    sead::Vector3f front;
    sead::Vector3f poseFront;
    if (_b8 || !hasMoveInput) {
        front = mTurnCtrl->get_5c();
        poseFront = front;
    } else {
        front = moveDir;
        poseFront = mTurnCtrl->get_50();
    }
    if (mIsForceRunCtrlActive) {
        moveInput = moveDir = front;
    }

    const sead::Vector3f& wallNormal =
        rs::isCollidedWall(mCollision) ? rs::getCollidedWallNormal(mCollision)
                                       : sead::Vector3f::zero;
    const bool poseFrontZero = al::isNearZero(poseFront, 0.001f);
    const f32 frontSpeed = poseFront.dot(velocityDir);
    const bool opposite = !poseFrontZero && hasMoveInput &&
                          PlayerActionFunction::isOppositeDir(moveDir, poseFront) &&
                          !mIsForceRunCtrlActive;
    const bool turnBlocked = mTurnCtrl->get_68();
    const bool squatBrake = isActiveSquatBrake();
    const bool wallBrake = wallNormal.dot(velocityDir) < -0.17365f && !squatBrake;

    updateHillAffect(velocityDir, front, hasMoveInput);

    f32 targetSpeed = 0.0f;
    const bool noForward = turnBlocked || !hasMoveInput;
    if (frontSpeed >= 0.0f && !noForward) {
        const f32 inputLength = moveInput.length();
        targetSpeed = inputLength * calcMaxSpeed(speed) + (1.0f - inputLength) * mMinSpeed;
        if (wallBrake && targetSpeed > mMaxSpeed)
            targetSpeed = mMaxSpeed;
        speed = PlayerActionFunction::accel(speed, targetSpeed, calcAccelRate(speed));
    }

    if (speed <= mBrakeSpeed) {
        if (_60 < mCounterBorder)
            _60 = 0;
    } else {
        _60 = sead::Mathi::min(_60 + 1, mCounterBorder);
    }

    s32 brakeFrame;
    if (wallBrake || (frontSpeed < 0.0f && opposite))
        brakeFrame = mBrakeFrame;
    else if (!hasMoveInput && frontSpeed < 0.0f)
        brakeFrame = 1;
    else if (opposite || !hasMoveInput)
        brakeFrame = (_60 >= mCounterBorder || squatBrake) ? mBrakeFrame : 1;
    else
        brakeFrame = mStickOnBrakeFrame;

    if (speed > targetSpeed)
        speed = sead::Mathf::max(PlayerActionFunction::brake(speed, brakeFrame, mMaxSpeed),
                                 targetSpeed);
    if (mIsForceRunCtrlActive && speed < _a0)
        speed = _a0;

    if (noForward && !poseFrontZero && !squatBrake && _60 >= mCounterBorder) {
        updatePoseUpFront(mGroundNormal, poseFront, speed);
        al::setVelocity(mParent, speed * poseFront - mGravityMove * mGroundNormal);
        _64 = true;
        _90 = poseFront;
        return speed;
    }

    if (_b8 && opposite && !poseFrontZero &&
        (frontSpeed < 0.0f || speed <= mMinSpeed)) {
        updatePoseUpFront(mGroundNormal, poseFront, speed);
        al::setVelocity(mParent, speed * velocityDir - mGravityMove * mGroundNormal);
        _bc = true;
        _90 = poseFront;
        return speed;
    }

    mHasStopped = !hasMoveInput && al::isNearZero(speed, 0.001f);

    sead::Vector3f nextVelocity;
    if (_bd && al::isNormalize(velocityDir, 0.001f)) {
        const f32 turnRate = sead::Mathf::abs(mTurnCtrl->calcTurnPowerRate(mGroundNormal));
        const f32 turnDelta = sead::Mathf::clamp(turnRate - _c0, 0.0f, 1.0f);
        _c0 = al::lerpValue(_c0, turnRate, 0.15f);

        sead::Vector3f verticalFront = sead::Vector3f::zero;
        al::verticalizeVec(&verticalFront, front, velocityDir);
        if (al::tryNormalizeOrZero(&verticalFront)) {
            const f32 blend = al::lerpValue(0.3f, 0.9f, turnDelta);
            const f32 forwardSpeed =
                sead::Mathf::max(speed * front.dot(velocityDir), mMinSpeed);
            const f32 verticalDot = velocityDir.dot(verticalFront);
            nextVelocity = forwardSpeed * front +
                           blend * speed * verticalDot * verticalFront;
            if (!al::isNearZero(nextVelocity, 0.001f))
                al::limitLength(&nextVelocity, nextVelocity, speed);
            adjustGroundVelocityForWall(&nextVelocity, -mGravityMove * mGroundNormal,
                                        mCollision, -al::getGravity(mParent));
            nextVelocity -= mGravityMove * mGroundNormal;
            al::setVelocity(mParent, nextVelocity);
        } else {
            nextVelocity = speed * front;
            adjustGroundVelocityForWall(&nextVelocity, -mGravityMove * mGroundNormal,
                                        mCollision, -al::getGravity(mParent));
            nextVelocity -= mGravityMove * mGroundNormal;
            al::setVelocity(mParent, nextVelocity);
        }
    } else {
        _c0 = 0.0f;
        nextVelocity = speed * front;
        adjustGroundVelocityForWall(&nextVelocity, -mGravityMove * mGroundNormal,
                                    mCollision, -al::getGravity(mParent));
        nextVelocity -= mGravityMove * mGroundNormal;
        al::setVelocity(mParent, nextVelocity);
    }

    sead::Vector3f poseUp = mGroundNormal;
    if (_bb)
        mTurnCtrl->calcTilt(&poseUp, mGroundNormal, mPlayerConst->getTiltPoseDegreeMax());
    updatePoseUpFront(poseUp, front, speed);
    _90 = front;
    return speed;
}

void PlayerActionGroundMoveControl::updateNormalAndSnap(sead::Vector3f* velocity) {
    const sead::Vector3f previousNormal = mGroundNormal;
    if (!_c4 || rs::isCollidedGround(mCollision)) {
        rs::calcGroundNormalOrGravityDir(&mGroundNormal, mParent, mCollision);
        _c8 = mGroundNormal;
    } else {
        mGroundNormal = -al::getGravity(mParent);

        sead::Vector3f hitPos = {0.0f, 0.0f, 0.0f};
        sead::Vector3f hitNormal = {0.0f, 0.0f, 0.0f};
        if (alCollisionUtil::getHitPosAndNormalOnArrow(
                mParent, &hitPos, &hitNormal, al::getTrans(mParent),
                al::getGravity(mParent) * 100.0f, nullptr, nullptr) &&
            al::isFloorPolygon(hitNormal, al::getGravity(mParent))) {
            if (!(_c8.dot(hitNormal) > 0.087156f)) {
                sead::Vector3f* velocityPtr = al::getVelocityPtr(mParent);
                al::limitLength(velocityPtr, *velocityPtr, al::calcSpeedH(mParent));
            } else {
                al::turnVecToVecRate(&mGroundNormal, previousNormal, hitNormal, 0.5f);
                _c8 = mGroundNormal;
            }
        }
    }

    al::alongVectorNormalH(velocity, al::getVelocity(mParent), previousNormal, mGroundNormal);
}

f32 PlayerActionGroundMoveControl::calcTurnTiltRate() const {
    if (rs::isOnGroundSkateCode(mParent, mCollision))
        return 0.0f;
    return mTurnCtrl->calcTurnPowerRate(mGroundNormal);
}

void PlayerActionGroundMoveControl::calcMoveInput(sead::Vector3f* moveInput,
                                                   const sead::Vector3f& up) {
    if (mHack)
        rs::calcHackerMoveVec(moveInput, *mHack, up);
    else
        mPlayerInput->calcMoveInput(moveInput, up);
}

bool PlayerActionGroundMoveControl::isActiveSquatBrake() const {
    return _d5 && !mHack && mPlayerInput->isHoldSquat();
}

// NON_MATCHING: exact 0x29c size and exact prefix through the hill-rate clamp; first mismatch
// is the no-input branch target because the target advances its `_7c` base pointer before the
// shared tail, while current code retains `this` and uses offset loads.
void PlayerActionGroundMoveControl::updateHillAffect(const sead::Vector3f& groundNormal,
                                                     const sead::Vector3f& moveInput,
                                                     bool hasMoveInput) {
    if (!_78)
        return;

    f32* hillAffect = &_7c;

    const f32 hillAngle = sead::Mathf::acos(sead::Mathf::clamp(
        mGroundNormal.dot(-al::getGravity(mParent)), -1.0f, 1.0f));

    sead::Vector3f hillDir = {0.0f, 0.0f, 0.0f};
    al::verticalizeVec(&hillDir, mGroundNormal, al::getGravity(mParent));

    f32 target = 0.0f;
    f32 hillSpeed = 0.0f;
    bool isDownhill = false;
    if (al::tryNormalizeOrZero(&hillDir)) {
        const f32 hillDegree = sead::Mathf::rad2deg(hillAngle);
        const f32 hillRate =
            sead::Mathf::clamp(hillDegree / mPlayerConst->getStandAngleMin(), 0.0f, 1.0f);
        f32 moveDot = 0.0f;
        if (hasMoveInput && (moveDot = hillDir.dot(moveInput)) > 0.0f)
            target += moveDot * al::easeOut(hillRate);
        else
            target -= al::easeIn(hillRate);

        hillSpeed = sead::Mathf::clampMin(hillDegree * moveDot, 0.0f);
        isDownhill = target > 0.0f;
        if (isDownhill && *hillAffect < 0.0f)
            *hillAffect = 0.0f;
    }

    s32 accelFrame = mPlayerConst->getHillAccelAddFrame();
    if (*hillAffect > target) {
        const f32 frameRate = sead::Mathf::clamp(target + 1.0f, 0.0f, 1.0f);
        accelFrame = sead::Mathf::ceil(frameRate * (mPlayerConst->getHillAccelSubFrame() - 1)) + 1;
        if (isDownhill) {
            const f32 rate = al::calcRate01(hillSpeed,
                                            mPlayerConst->getHillAccelSubAngleMin(),
                                            mPlayerConst->getHillAccelSubAngleMax());
            target = al::lerpValue(target, *hillAffect, al::easeOut(al::easeOut(rate)));
        }
    }

    *hillAffect = al::converge(*hillAffect, target, 1.0f / (f32)accelFrame);
}

f32 PlayerActionGroundMoveControl::calcMaxSpeed(f32 speed) const {
    f32 hillSpeed = 0.0f;
    if (_78) {
        if (_7c > 0.0f)
            hillSpeed = _7c * mPlayerConst->getHillAddSpeed();
        else
            hillSpeed = -_7c * mPlayerConst->getHillSubSpeed();
    }

    if (mJudge) {
        const bool isDashSpeed = al::isNearZeroOrLess(mMaxSpeed * 0.99f - speed, 0.001f);
        const bool isDash = rs::isJudge(mJudge);
        if (isDashSpeed && isDash)
            return hillSpeed + _70;
    }

    return PlayerActionFunction::accel(hillSpeed + mMaxSpeed, _44, _40);
}

f32 PlayerActionGroundMoveControl::calcAccelRate(f32 speed) const {
    if (mJudge) {
        const f32 speedDiff = mMaxSpeed - speed;
        bool isBelowMax = false;
        if (!(speedDiff <= 0.0f))
            isBelowMax = !al::isNearZero(speedDiff, 0.001f);
        const bool isDash = rs::isJudge(mJudge);
        if (!isBelowMax && isDash)
            return (_70 - mMaxSpeed) / (f32)_74;
    }

    if (_d4 && mMaxSpeed > speed) {
        const s32 frame = mRunFrame;
        const f32 frameSquared = (f32)(frame * frame);
        const f32 baseAccel = mMaxSpeed / (f32)frame;
        const f32 scale = mPlayerConst->getRunAccelAverageScale();
        const f32 accel = baseAccel * scale;
        const f32 accelRate =
            (mMaxSpeed - accel * (f32)mRunFrame) * 2.0f / frameSquared;
        return accel +
               accelRate *
                   (1.0f +
                    (sead::Mathf::sqrt(accel * accel + accelRate * 2.0f * speed) - accel) /
                        accelRate);
    }

    return mMaxSpeed / (f32)mRunFrame;
}

void PlayerActionGroundMoveControl::updatePoseUpFront(const sead::Vector3f& up,
                                                       const sead::Vector3f& front, f32 speed) {
    sead::Vector3f poseFront = front;
    if (al::isParallelDirection(up, poseFront, 0.01f)) {
        al::calcFrontDir(&poseFront, mParent);
        if (al::isParallelDirection(up, poseFront, 0.01f))
            al::calcUpDir(&poseFront, mParent);
    }

    const f32 rate = mAlwaysFalse ? al::calcRate01(speed, _a8, _ac) : 1.0f;
    rs::slerpUpFront(mParent, up, poseFront, mPlayerConst->getSlerpQuatRate(),
                     rate * mPlayerConst->getHillPoseDegreeMax());
}
namespace {
// NON_MATCHING: exact 0x130 size and instruction order; final scalar FMULs use the opposite
// commutative operand encoding from the target. Next hypothesis is a vector/scalar expression form
// that preserves component-first multiplication without folding the stores.
bool adjustGroundVelocityForWall(sead::Vector3f* velocity, const sead::Vector3f& gravityMove,
                                 const IUsePlayerCollision* collision,
                                 const sead::Vector3f& gravityUp) {
    if (!rs::isCollidedWallFace(collision))
        return false;

    const sead::Vector3f& wallNormal = rs::getCollidedWallNormal(collision);
    sead::Vector3f wallVelocity = {0.0f, 0.0f, 0.0f};
    al::verticalizeVec(&wallVelocity, wallNormal, *velocity);
    const f32 wallVelocityUp = gravityUp.dot(wallVelocity);
    sead::Vector3f wallGravity = {0.0f, 0.0f, 0.0f};
    al::verticalizeVec(&wallGravity, wallNormal, gravityMove);
    const f32 push = wallVelocityUp + gravityUp.dot(gravityMove) * 0.9f;
    if (!al::isNearZeroOrGreater(push, 0.001f))
        return false;

    velocity->x -= push * gravityUp.x;
    velocity->y -= push * gravityUp.y;
    velocity->z -= push * gravityUp.z;
    return true;
}
}  // namespace

