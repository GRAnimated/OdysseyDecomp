#include "Player/PlayerStateWallCatch.h"

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/IJudge.h"
#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerActionCollisionSnap.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerAreaChecker.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerJointParamGrab.h"
#include "Player/PlayerJudgePreInputJump.h"
#include "Player/PlayerJudgePreInputPoleClimbSwing.h"
#include "Player/PlayerTrigger.h"
#include "Util/JudgeUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(PlayerStateWallCatch, Start)
NERVE_IMPL(PlayerStateWallCatch, EndFall)
NERVE_IMPL(PlayerStateWallCatch, Jump)
NERVE_IMPL(PlayerStateWallCatch, Climb)
NERVE_IMPL(PlayerStateWallCatch, ClimbFast)
NERVE_IMPL(PlayerStateWallCatch, Wait)
NERVE_IMPL(PlayerStateWallCatch, MoveLeft)
NERVE_IMPL(PlayerStateWallCatch, MoveRight)
NERVES_MAKE_NOSTRUCT(PlayerStateWallCatch, EndFall)
NERVES_MAKE_STRUCT(PlayerStateWallCatch, Start, Jump, Climb, ClimbFast, Wait, MoveLeft, MoveRight)

// NON_MATCHING: behavior is recovered; current is 0x188 vs target 0x18c. Target stores the raw
// stick result with two scalar STRs while current combines them into STP, and target uses ordered
// FP branch encodings. Next source-level hypothesis is the original result/Vector2 assignment form
// that preserves scalar stores and result materialization without forced codegen.
s64 calcWallCatchMoveInputType(sead::Vector2f* inputDir, sead::Vector2f* previousStick,
                                const PlayerInput* input, const sead::Vector3f& up,
                                const sead::Vector3f& front, const sead::Vector3f& side,
                                f32 climbDegree, f32 moveDegree, f32 repeatAngle);
}  // namespace

// NON_MATCHING: behavior and target size (0x188) are recovered; remaining differences are constructor
// register-live-range/store scheduling, notably collision preservation and initial zero-store ordering.
// Next source-level hypothesis is a natural lifetime split for collision/player inputs around the two
// helper allocations that delays collision preservation until the AirMove constructor call.
PlayerStateWallCatch::PlayerStateWallCatch(
    al::LiveActor* player, const PlayerConst* pConst, const PlayerInput* input,
    IUsePlayerCollision* collision, const IUsePlayerCeilingCheck* ceilingCheck,
    const PlayerModelHolder* modelHolder, const PlayerAreaChecker* areaChecker,
    PlayerAnimator* animator, PlayerTrigger* trigger,
    PlayerJudgePreInputJump* judgePreInputJump, PlayerJointParamGrab* grabJoint)
    : al::ActorStateBase("壁つかまり", player), mConst(pConst), mInput(input),
      mCollision(collision), mCeilingCheck(ceilingCheck), mModelHolder(modelHolder),
      mAreaChecker(areaChecker), mAnimator(animator), mTrigger(trigger), mAirMoveControl(nullptr),
      mCollisionSnap(nullptr), mJudgePreInputJump(judgePreInputJump), mGrabJoint(grabJoint),
      mJudgePreInputPoleClimbSwing(nullptr) {
    _80.set(0.0f, 0.0f, 0.0f);
    _8c = 0;
    mWallCatchInputDir.set(0.0f, 0.0f);
    mWallCatchStick.set(0.0f, 0.0f);
    _a0.set(0.0f, 0.0f, 0.0f);
    _ac = false;
    _ad = false;

    mAirMoveControl = new PlayerActionAirMoveControl(player, pConst, input, collision, true);
    mAirMoveControl->setupCollideWallScaleVelocity(pConst->getFallSpeedMax(), 0.0f,
                                                   pConst->getJumpPowerMaxBorder2D());
    mAirMoveControl->setIsInertiaWall(true);

    mJudgePreInputPoleClimbSwing = new PlayerJudgePreInputPoleClimbSwing(pConst, input);
    mCollisionSnap = new PlayerActionCollisionSnap(player, collision);
    mCollisionSnap->setVerticalizeSnapFront(true);
    initNerve(&NrvPlayerStateWallCatch.Start, 0);
}

void PlayerStateWallCatch::appear() {
    al::NerveStateBase::appear();
    mCollisionSnap->startWallCatch();
    if (mAnimator->isSubAnimPlaying())
        mAnimator->endSubAnim();
    mAnimator->startAnim("WallCatchStart");

    mWallCatchInputDir.set(0.0f, 0.0f);
    mWallCatchStick.set(0.0f, 0.0f);
    _a0.set(al::getTrans(mActor));
    _8c = 0;
    mGrabJoint->dynamicsRate = 1.0f;
    mGrabJoint->dynamicsFollowRate = 1.0f;
    mGrabJoint->poseRate = 1.0f;
    mGrabJoint->interpolateRate = 0.0f;
    mCollisionSnap->calcFollowDir(&mGrabJoint->direction, _80);
    rs::resetJudge(reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing));
    _ac = false;
    _ad = initIgnoreFallInput();
    rs::resetCollisionExpandCheck(mCollision);
    al::setNerve(this, &NrvPlayerStateWallCatch.Start);
}

// NON_MATCHING: behavior, getter mapping, and the two Vector2f state fields are recovered; current
// is 0x134 vs target 0x13c because target spills/reloads the move-input result before comparing it
// to 2, while current keeps W0 live. Next source-level hypothesis is the original result enum/local
// representation that naturally materializes the stack local without volatile or forced codegen.
bool PlayerStateWallCatch::initIgnoreFallInput() {
    al::LiveActor* actor = mActor;
    const sead::Vector3f gravity = al::getGravity(actor);
    sead::Vector3f front;
    front.set(0.0f, 0.0f, 0.0f);
    const sead::Vector3f up = -gravity;
    al::calcFrontDir(&front, actor);
    sead::Vector3f side = front.cross(gravity);
    al::tryNormalizeOrZero(&side);

    const s32 inputType = calcWallCatchMoveInputType(
        &mWallCatchInputDir, &mWallCatchStick, mInput, up, front, side,
        mConst->getWallClimbDegree(), mConst->getWallCatchMoveDegree(),
        mConst->getWallCatchInputRepeatAngle());
    if (inputType == 2)
        return true;

    mWallCatchInputDir.set(0.0f, 0.0f);
    mWallCatchStick.set(0.0f, 0.0f);
    return false;
}

void PlayerStateWallCatch::kill() {
    mGrabJoint->dynamicsRate = 0.0f;
    mGrabJoint->dynamicsFollowRate = 1.0f;
    mGrabJoint->poseRate = 0.0f;
    mGrabJoint->direction = sead::Vector3f::ey;
    mGrabJoint->interpolateRate = 0.0f;

    if (!isDead()) {
        if (!al::isNerve(this, &NrvPlayerStateWallCatch.Jump) || !al::isGreaterStep(this, 0)) {
            if (al::isNerve(this, &NrvPlayerStateWallCatch.Climb)) {
                if (!al::isLessEqualStep(this, 25)) {
                    al::NerveStateBase::kill();
                    return;
                }
            } else if (al::isNerve(this, &NrvPlayerStateWallCatch.ClimbFast)) {
                if (!al::isLessEqualStep(this, 15)) {
                    al::NerveStateBase::kill();
                    return;
                }
            }

            if (!al::isNerve(this, &NrvPlayerStateWallCatch.Start))
                mTrigger->set(PlayerTrigger::EActionTrigger_val3);
        }
    }

    al::NerveStateBase::kill();
}

bool PlayerStateWallCatch::isWallCatchForm() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvPlayerStateWallCatch.Jump) && al::isGreaterStep(this, 0))
        return false;
    if (al::isNerve(this, &NrvPlayerStateWallCatch.Climb))
        return al::isLessEqualStep(this, 25);
    if (al::isNerve(this, &NrvPlayerStateWallCatch.ClimbFast))
        return al::isLessEqualStep(this, 15);
    return true;
}

bool PlayerStateWallCatch::update() {
    rs::updateJudge(reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing));
    return al::NerveStateBase::update();
}


void PlayerStateWallCatch::control() {
    _a0 = al::getTrans(mActor);
    mCollisionSnap->calcFollowDir(&mGrabJoint->direction, _80);
}

void PlayerStateWallCatch::setup(const al::CollisionParts* collisionParts,
                                 const sead::Vector3f& position,
                                 const sead::Vector3f& front,
                                 const sead::Vector3f& up) {
    mCollisionSnap->setup(collisionParts, position, front, up);
    _80 = up;
}


// NON_MATCHING: target is 0x190 bytes and validator-clean current is 0x188; behavior is recovered,
// but sead vector helpers fold the target zero-minus scale into FNEG and shorten the body. Next
// source-level hypothesis is a validator-clean vector form that preserves the target zero scalar.
void PlayerStateWallCatch::endFallFromWall() {
    PlayerAnimator* animator = mAnimator;
    al::LiveActor* actor = mActor;
    const PlayerConst* pConst = mConst;
    PlayerActionCollisionSnap* collisionSnap = mCollisionSnap;

    f32 fallSpeed = 0.0f;
    if (!rs::isCollidedGround(mCollision))
        fallSpeed = pConst->getWallFallJumpSpeed();

    sead::Vector3f up = {0.0f, 0.0f, 0.0f};
    al::calcUpDir(&up, actor);
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    al::calcFrontDir(&front, actor);

    const f32 radius = pConst->getCollisionRadius() + 10.0f;
    sead::Vector3f move;
    move.setScale(front, -radius);
    sead::Vector3f upOffset;
    upOffset.setScale(up, -pConst->getTall());
    move += upOffset;
    collisionSnap->endFall(fallSpeed, move, pConst->getCollisionRadius());
    animator->startAnim("Fall");
    animator->clearInterpolation();
    al::setNerve(this, &NrvPlayerStateWallCatch.Start);
    kill();
}

bool PlayerStateWallCatch::isClimbJump() const {
    return !isDead() && al::isNerve(this, &NrvPlayerStateWallCatch.Jump) &&
           al::isGreaterStep(this, 0);
}

bool PlayerStateWallCatch::isClimbJumpFall() const {
    if (isDead() || !al::isNerve(this, &NrvPlayerStateWallCatch.Jump) ||
        !al::isGreaterStep(this, 0))
        return false;
    return mAnimator->isAnim("Fall");
}

bool PlayerStateWallCatch::isFallEnd() const {
    return isDead() && al::isNerve(this, &EndFall);
}

bool PlayerStateWallCatch::isEnableIK() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvPlayerStateWallCatch.Climb))
        return al::isGreaterEqualStep(this, 40);
    if (al::isNerve(this, &NrvPlayerStateWallCatch.ClimbFast))
        return al::isGreaterEqualStep(this, 16);
    return false;
}

bool PlayerStateWallCatch::isEnableTrample() const {
    return isDead() || !al::isNerve(this, &NrvPlayerStateWallCatch.Start) ||
           al::isGreaterEqualStep(this, 10);
}

bool PlayerStateWallCatch::isEnableDamage() const {
    return isDead() || !al::isNerve(this, &NrvPlayerStateWallCatch.Start) ||
           al::isGreaterEqualStep(this, 11);
}

sead::Vector3f PlayerStateWallCatch::getWallCatchFront() const {
    sead::Vector3f front(0.0f, 0.0f, 0.0f);
    mCollisionSnap->calcFollowDir(&front, mCollisionSnap->getSnapFront());
    return front;
}

const sead::Vector3f& PlayerStateWallCatch::getCeilingCheckPos() const {
    return _a0;
}


void PlayerStateWallCatch::exeStart() {
    bool isWallLost = false;
    if (!followCollision(&isWallLost, true)) {
        if (isWallLost)
            al::setNerve(this, &NrvPlayerStateWallCatch.ClimbFast);
        else
            endFallFromWall();
        return;
    }

    if (rs::isJudge(mJudgePreInputJump) && enableClimb() &&
        al::isGreaterEqualStep(this, mConst->getWallClimbJumpStartFrame())) {
        rs::resetJudge(mJudgePreInputJump);
        if (_8c <= mConst->getWallClimbJumpEndFrame()) {
            al::setNerve(this, &NrvPlayerStateWallCatch.Jump);
            return;
        }
        al::setNerve(this, &NrvPlayerStateWallCatch.ClimbFast);
        return;
    }

    if (!al::isGreaterEqualStep(this, mConst->getWallClimbStartFrame()) ||
        !tryStartClimbFallMove()) {
        _8c++;
        if (mAnimator->isAnimEnd())
            al::setNerve(this, &NrvPlayerStateWallCatch.Wait);
    }
}

// NON_MATCHING: target is 0x1a0 bytes and validator-clean current is 0x18c; behavior/call signature
// match, but sead vector helpers shorten and reallocate the target-position reconstruction. Next
// source-level hypothesis is a validator-clean expression retaining delta/vertical in D12-D14.
bool PlayerStateWallCatch::enableClimb() {
    if (mAreaChecker->isInWallClimbBan(al::getTrans(mActor)))
        return false;

    sead::Vector3f front(0.0f, 0.0f, 0.0f);
    al::calcFrontDir(&front, mActor);
    sead::Vector3f targetPos(0.0f, 0.0f, 0.0f);

    al::LiveActor* actor = mActor;
    const sead::Vector3f& snapPos = mCollisionSnap->getCurrentSnapPos();
    const sead::Vector3f& trans = al::getTrans(actor);
    const sead::Vector3f delta = trans - snapPos;
    const sead::Vector3f& gravity = al::getGravity(actor);
    const f32 vertical = delta.dot(gravity);

    sead::Vector3f currentFront(0.0f, 0.0f, 0.0f);
    al::calcFrontDir(&currentFront, actor);
    const f32 horizontal = delta.dot(currentFront);
    targetPos.setScale(gravity, vertical);
    sead::Vector3f horizontalOffset;
    horizontalOffset.setScale(currentFront, horizontal);
    targetPos += horizontalOffset;
    targetPos += snapPos;

    return rs::judgeWallCatchClimb(actor, mCollision, mCeilingCheck, front, targetPos,
                                   mConst->getCollisionRadiusStand(), false, sead::Vector3f::zero);
}

// NON_MATCHING: behavior and target size (0x408) are recovered; keeping the successful collision
// parts pointer live reproduces the target 0x100 frame/X28-X23 save shape, and an s32 isNerve
// temporary reproduces the target TBNZ. Remaining differences are instruction/register scheduling.
// Next source-level hypothesis is the target switch/source ordering and named snap-front lifetime.
bool PlayerStateWallCatch::tryStartClimbFallMove() {
    al::LiveActor* actor = mActor;
    const sead::Vector3f gravity = al::getGravity(actor);
    sead::Vector3f front;
    front.set(0.0f, 0.0f, 0.0f);
    const sead::Vector3f up = -gravity;
    al::calcFrontDir(&front, actor);
    sead::Vector3f side = front.cross(gravity);
    al::tryNormalizeOrZero(&side);

    const s32 inputType = calcWallCatchMoveInputType(
        &mWallCatchInputDir, &mWallCatchStick, mInput, up, front, side,
        mConst->getWallClimbDegree(), mConst->getWallCatchMoveDegree(),
        mConst->getWallCatchInputRepeatAngle());

    if (_ad) {
        if (inputType == 2)
            return false;
        if (inputType == 0) {
            _ad = mInput->isMoveDeepDown();
            return false;
        }
        _ad = false;
    }

    if (inputType == 0)
        return false;
    if (inputType == 2) {
        endFallFromWall();
        return true;
    }
    if (inputType == 1) {
        if (!enableClimb())
            return false;
        al::setNerve(this, &NrvPlayerStateWallCatch.Climb);
        return true;
    }

    const bool isMoving = _ac;
    if (al::isNerve(this, &NrvPlayerStateWallCatch.MoveLeft)) {
        if (isMoving)
            return false;
    } else {
        const s32 isMoveRight = al::isNerve(this, &NrvPlayerStateWallCatch.MoveRight);
        if (isMoving) {
            if (isMoveRight)
                return false;
        }
    }

    const f32 moveHeightRange = mConst->getWallCatchMoveHeightRange();
    const sead::Vector3f searchPosition = mCollisionSnap->getCurrentSnapPos() - front * 100.0f;
    sead::Vector3f move = side * mConst->getWallCatchMoveSpeed();
    if (inputType == 4)
        move = -move;

    sead::Vector3f position;
    position.set(0.0f, 0.0f, 0.0f);
    sead::Vector3f moveFront;
    moveFront.set(0.0f, 0.0f, 0.0f);
    sead::Vector3f moveUp;
    moveUp.set(0.0f, 0.0f, 0.0f);
    const al::CollisionParts* collisionParts = nullptr;
    if (!rs::findWallCatchMovePos(
            &collisionParts, &position, &moveFront, &moveUp, mActor, front, searchPosition, move,
            200.0f, mConst->getWallKeepDegree(), mConst->getWallCatchDegree(), moveHeightRange,
            mConst->getWallCatchHeightBottom(), mConst->getCollisionRadius(),
            mConst->getCollisionRadiusStand()))
        return false;

    const al::CollisionParts* moveCollisionParts = collisionParts;
    s32 moveFrame = mInput->isHoldWallCatchMoveFast() ? mConst->getWallCatchMoveFrameFast()
                                                      : mConst->getWallCatchMoveFrame();
    if (rs::isJudge(reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing)))
        moveFrame = mConst->getWallCatchMoveFrameSwing();

    mCollisionSnap->moveSnapPos(moveCollisionParts, position, -moveFront, moveUp, moveFrame);
    _80 = moveUp;
    if (inputType == 3)
        al::setNerve(this, &NrvPlayerStateWallCatch.MoveLeft);
    else
        al::setNerve(this, &NrvPlayerStateWallCatch.MoveRight);
    return true;
}

void PlayerStateWallCatch::exeWait() {
    if (al::isFirstStep(this))
        mAnimator->startAnim("WallCatch");
    updateWallCatchKeep(true);
}

bool PlayerStateWallCatch::updateWallCatchKeep(bool isKeep) {
    bool isWallLost = false;
    if (!followCollision(&isWallLost, isKeep)) {
        if (!isWallLost) {
            endFallFromWall();
            return true;
        }
        al::setNerve(this, &NrvPlayerStateWallCatch.ClimbFast);
        return true;
    }

    if (rs::judgeAndResetReturnTrue(mJudgePreInputJump) && enableClimb()) {
        al::setNerve(this, &NrvPlayerStateWallCatch.ClimbFast);
        return true;
    }

    _8c++;
    return tryStartClimbFallMove();
}

void PlayerStateWallCatch::exeMoveLeft() {
    if (al::isFirstStep(this)) {
        if (!mAnimator->isAnim("WallCatchMoveL"))
            mAnimator->startAnim("WallCatchMoveL");
        initMoveFrameLeftRight();
    }

    _ac = !mCollisionSnap->isMoveEnd();
    mCollisionSnap->updateMove();
    PlayerJointParamGrab* grabJoint = mGrabJoint;
    grabJoint->dynamicsRate = al::converge(grabJoint->dynamicsRate, 0.2f, 0.2f);
    if (!updateWallCatchKeep(false) && !_ac) {
        mGrabJoint->dynamicsRate = 1.0f;
        al::setNerve(this, &NrvPlayerStateWallCatch.Wait);
    }
}

// NON_MATCHING: target/current are 0x4c bytes; target reloads mAnimator before FP epilogue/divide scheduling while current reload occurs later; next source-level hypothesis is a source lifetime that retains the animator member address through getAnimFrameMax.
void PlayerStateWallCatch::initMoveFrameLeftRight() {
    const s32 moveFrame = 2 * mCollisionSnap->getMoveFrame() - 1;
    const f32 animFrameMax = mAnimator->getAnimFrameMax();
    mAnimator->setAnimRate(animFrameMax / static_cast<f32>(moveFrame) * 0.999f);
}

void PlayerStateWallCatch::exeMoveRight() {
    if (al::isFirstStep(this)) {
        if (!mAnimator->isAnim("WallCatchMoveR"))
            mAnimator->startAnim("WallCatchMoveR");
        initMoveFrameLeftRight();
    }

    _ac = !mCollisionSnap->isMoveEnd();
    mCollisionSnap->updateMove();
    PlayerJointParamGrab* grabJoint = mGrabJoint;
    grabJoint->dynamicsRate = al::converge(grabJoint->dynamicsRate, 0.2f, 0.2f);
    if (!updateWallCatchKeep(false) && !_ac) {
        mGrabJoint->dynamicsRate = 1.0f;
        al::setNerve(this, &NrvPlayerStateWallCatch.Wait);
    }
}

// NON_MATCHING: behavior is recovered; current is 0x1c0 vs target 0x1bc. The target retains
// projection scalars with a 0xa0-byte frame, while current vector temporaries use a 0x90-byte
// frame and different spill order. Next source-level hypothesis is the original scalar/component
// construction of targetPos that keeps the projection values live.
void PlayerStateWallCatch::endClimb() {
    sead::Vector3f front;
    front.set(0.0f, 0.0f, 0.0f);
    al::calcFrontDir(&front, mActor);

    bool isWallFacing = false;
    if (rs::isCollidedWall(mCollision))
        isWallFacing = front.dot(rs::getCollidedWallNormal(mCollision)) > 0.70711f;

    al::LiveActor* actor = mActor;
    const sead::Vector3f& snapPos = mCollisionSnap->getCurrentSnapPos();
    const sead::Vector3f delta = al::getTrans(actor) - snapPos;
    const sead::Vector3f& gravity = al::getGravity(actor);
    const f32 vertical = delta.dot(gravity);

    sead::Vector3f currentFront;
    currentFront.set(0.0f, 0.0f, 0.0f);
    al::calcFrontDir(&currentFront, actor);
    f32 horizontal = delta.dot(currentFront);
    if (isWallFacing && horizontal < 0.0f)
        horizontal = 0.0f;

    sead::Vector3f targetPos;
    targetPos.set(0.0f, 0.0f, 0.0f);
    targetPos += currentFront * horizontal;
    targetPos += gravity * vertical;
    targetPos += snapPos;
    al::setTrans(actor, targetPos);
    kill();
}

void PlayerStateWallCatch::exeJump() {
    if (al::isFirstStep(this)) {
        mTrigger->set(PlayerTrigger::EActionTrigger_val3);
        mAnimator->startAnim("WallCatchEndJump");

        sead::Vector3f front;
        al::calcFrontDir(&front, mActor);
        al::setVelocityToDirection(mActor, front, mConst->getWallClimbJumpSpeedH());

        mAirMoveControl->setup(mConst->getJumpMoveSpeedMax(), mConst->getWallClimbJumpSpeedH(), 0,
                               mConst->getWallClimbJumpSpeedV(),
                               mConst->getWallClimbJumpGravity(),
                               mConst->getWallClimbJumpInvalidFrame(), 1.0f);
        mGrabJoint->poseRate = 0.0f;
    }

    mAirMoveControl->update();
    if (mAnimator->isAnim("WallCatchEndJump") && mAnimator->isAnimEnd())
        mAnimator->startAnim("Fall");

    if (rs::isLandGroundRunAngle(mActor, mCollision, mConst))
        kill();
}

void PlayerStateWallCatch::exeEndFall() {}

namespace {
s64 calcWallCatchMoveInputType(sead::Vector2f* inputDir, sead::Vector2f* previousStick,
                                const PlayerInput* input, const sead::Vector3f& up,
                                const sead::Vector3f& front, const sead::Vector3f& side,
                                f32 climbDegree, f32 moveDegree, f32 repeatAngle) {
    sead::Vector3f moveInput;
    moveInput.set(0.0f, 0.0f, 0.0f);
    input->calcMoveInput(&moveInput, up);

    if (!input->isMoveDeepDown()) {
        inputDir->set(0.0f, 0.0f);
        previousStick->set(0.0f, 0.0f);
        return 0;
    }

    f32 sideDot;
    f32 frontDot;
    if (al::isNearZero(*previousStick, 0.001f)) {
        sead::Vector3f normalizedInput;
        normalizedInput.set(0.0f, 0.0f, 0.0f);
        al::normalize(&normalizedInput, moveInput);
        frontDot = front.dot(normalizedInput);
        inputDir->y = frontDot;
        sideDot = normalizedInput.dot(side);
        inputDir->x = sideDot;
        const sead::Vector2f rawStick = input->getStickMoveRaw();
        previousStick->x = rawStick.x;
        previousStick->y = rawStick.y;
    } else {
        if (!input->isSameStickMove(*previousStick, repeatAngle)) {
            inputDir->set(0.0f, 0.0f);
            previousStick->set(0.0f, 0.0f);
            return 0;
        }
        sideDot = inputDir->x;
        frontDot = inputDir->y;
    }

    const f32 climbCos = sead::Mathf::cos(sead::Mathf::deg2rad(climbDegree));
    s64 result = 1;
    if (frontDot <= climbCos) {
        result = 2;
        if (frontDot >= -climbCos) {
            const f32 moveCos = sead::Mathf::cos(sead::Mathf::deg2rad(moveDegree));
            result = 3;
            if (sideDot <= moveCos) {
                result = 4;
                if (sideDot >= -moveCos)
                    return 0;
            }
        }
    }
    return result;
}
}  // namespace

void PlayerStateWallCatch::moveCatchPos(const al::CollisionParts* collisionParts,
                                        const sead::Vector3f& position,
                                        const sead::Vector3f& front,
                                        const sead::Vector3f& up) {
    s32 moveFrame = mInput->isHoldWallCatchMoveFast() ? mConst->getWallCatchMoveFrameFast()
                                                      : mConst->getWallCatchMoveFrame();
    if (rs::isJudge(reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing)))
        moveFrame = mConst->getWallCatchMoveFrameSwing();

    mCollisionSnap->moveSnapPos(collisionParts, position, -front, up, moveFrame);
    _80 = up;
}

PlayerStateWallCatch::~PlayerStateWallCatch() = default;
