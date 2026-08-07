#include "Player/HackerActionAirMoveControl.h"

#include <cstring>

#include "Library/Collision/Collider.h"
#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"

#include "Player/IUsePlayerCollision.h"
#include "Player/IUsePlayerHack.h"
#include "Player/PlayerActionTurnControl.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackInputFunction.h"

HackerActionAirMoveControl::HackerActionAirMoveControl(al::LiveActor* actor, bool isSlerpGravity,
                                                       bool isUseAnyButton)
    : mActor(actor), mPlayerHack(nullptr), mTurnControl(nullptr), mIsSlerpGravity(isSlerpGravity),
      mIsUseAnyButton(isUseAnyButton), mIsHoldJumpExtend(false), mIsForceJumpExtend(false),
      mExtendFrame(0), mFrame(0), mNoInputFrame(0), mIsGroundUp(false), mIsInertiaWall(false),
      mIsClampSpeed(false), mClampSpeedMin(0.0f), mClampSpeedMax(0.0f),
      mStartMoveDir(sead::Vector3f::zero), mSideDir(sead::Vector3f::zero), mSpeedH(0.0f),
      mGravityAccel(0.0f), mFallSpeedMax(0.0f), mIsScaleWallVelocity(false),
      mScaleWallFront(1.0f), mScaleWallSide(0.0f), mScaleWallUp(0.0f), mSlerpQuatRate(0.0f),
      mPrevInput{0.0f, 0.0f, 0.0f}, mAccelFront(0.0f), mAccelBack(0.0f), mAccelTurn(0.0f),
      mCollision(nullptr) {
    mTurnControl = new PlayerActionTurnControl(actor);
    mTurnControl->set_88(true);
    mTurnControl->set_89(true);
    mTurnControl->setup(1.0f, 135.0f, 6.0f, 25.0f, 20, 1, 10);
}

// NON_MATCHING: exact 912-byte size; first mismatch at 0x40E0F0 is initial member-store/cross-product scheduling, with a later extend-frame compare difference.
void HackerActionAirMoveControl::setup(f32 speedMax, f32 inertiaAdd, s32 extendFrame,
                                       f32 velocityV, f32 gravityAccel, s32 noInputFrame,
                                       f32 inertiaRate, f32 fallSpeedMax, f32 normalMinSpeed,
                                       f32 slerpQuatRate, f32 accelFront, f32 accelBack,
                                       f32 accelTurn) {
    mNoInputFrame = noInputFrame;
    mGravityAccel = gravityAccel;
    mFallSpeedMax = fallSpeedMax;
    mAccelFront = accelFront;
    mAccelBack = accelBack;
    mAccelTurn = accelTurn;

    sead::Vector3f velocityH = {0.0f, 0.0f, 0.0f};
    sead::Vector3f groundNormal = {0.0f, 0.0f, 0.0f};
    if (mCollision) {
        if (mIsGroundUp)
            rs::calcGroundNormalOrUpDir(&groundNormal, mActor, mCollision);
        else
            rs::calcGroundNormalOrGravityDir(&groundNormal, mActor, mCollision);
    } else if (mIsGroundUp) {
        al::calcGroundNormalOrUpDir(&groundNormal, mActor);
    } else {
        al::calcGroundNormalOrGravityDir(&groundNormal, mActor);
    }

    al::verticalizeVec(&velocityH, groundNormal, al::getVelocity(mActor));
    al::verticalizeVec(&velocityH, al::getGravity(mActor), velocityH);
    if (!al::tryNormalizeOrZero(&mStartMoveDir, velocityH)) {
        al::calcFrontDir(&mStartMoveDir, mActor);
        al::verticalizeVec(&mStartMoveDir, al::getGravity(mActor), mStartMoveDir);
        if (!al::tryNormalizeOrZero(&mStartMoveDir))
            al::calcUpDir(&mStartMoveDir, mActor);
    }

    mSideDir.setCross(mStartMoveDir, al::getGravity(mActor));
    al::normalize(&mSideDir);

    f32 speed = velocityH.length();
    if (mIsClampSpeed) {
        if (speed < mClampSpeedMin)
            speed = mClampSpeedMin;
        else if (speed > mClampSpeedMax)
            speed = mClampSpeedMax;
    }

    sead::Vector3f inertia = {0.0f, 0.0f, 0.0f};
    sead::Vector3f moveVelocity = {0.0f, 0.0f, 0.0f};
    if (speed >= normalMinSpeed)
        sead::Vector3CalcCommon<f32>::multScalar(moveVelocity, mStartMoveDir, speed);

    if (mCollision) {
        if (mIsInertiaWall)
            rs::calcJumpInertiaWall(&inertia, mActor, mCollision, inertiaRate);
        else
            rs::calcJumpInertia(&inertia, mActor, mCollision, moveVelocity, inertiaRate);
    } else if (mIsInertiaWall) {
        al::calcJumpInertiaWall(&inertia, mActor, inertiaRate);
    } else {
        al::calcJumpInertia(&inertia, mActor, moveVelocity, inertiaRate);
    }

    f32 moveSpeed = inertia.length() + inertiaAdd;
    if (moveSpeed > speedMax)
        moveSpeed = speedMax;
    const f32 startSpeed = speed > speedMax ? speedMax : speed;
    if (startSpeed < moveSpeed) {
    } else {
        moveSpeed = startSpeed;
    }
    mSpeedH = moveSpeed;

    al::LiveActor* actor = mActor;
    const sead::Vector3f velocity =
        startSpeed * mStartMoveDir - velocityV * al::getGravity(actor) + inertia;
    al::setVelocity(actor, velocity);

    mIsForceJumpExtend = false;
    mIsHoldJumpExtend = extendFrame > 0;
    mExtendFrame = extendFrame < 1 ? 0 : extendFrame;
    mFrame = 0;
    mSlerpQuatRate = slerpQuatRate;
    mPrevInput.set(0.0f, 0.0f, 0.0f);
    mTurnControl->reset();
}

void HackerActionAirMoveControl::setupTurn(f32 angleStart, f32 angleFast, f32 angleLimit,
                                           f32 angleFastLimit, s32 accelFrame,
                                           s32 accelFrameFast, s32 brakeFrame) {
    mTurnControl->setup(angleStart, angleFast, angleLimit, angleFastLimit, accelFrame,
                        accelFrameFast, brakeFrame);
    mTurnControl->reset();
}

void HackerActionAirMoveControl::setExtendFrame(s32 frame) {
    mExtendFrame = frame;
}

void HackerActionAirMoveControl::setupCollideWallScaleVelocity(f32 scaleFront, f32 scaleSide,
                                                               f32 scaleUp) {
    mIsScaleWallVelocity = true;
    mScaleWallFront = scaleFront;
    mScaleWallSide = scaleSide;
    mScaleWallUp = scaleUp;
}

void HackerActionAirMoveControl::verticalizeStartMoveDir(const sead::Vector3f& vertical) {
    sead::Vector3f moveDir = {0.0f, 0.0f, 0.0f};
    sead::Vector3f* startMoveDir = &mStartMoveDir;
    const sead::Vector3f* verticalDir = &vertical;
    al::verticalizeVec(&moveDir, *verticalDir, *startMoveDir);
    if (!al::tryNormalizeOrZero(&moveDir)) {
        std::memcpy(&moveDir, &mSideDir, sizeof(moveDir));
        al::verticalizeVec(&moveDir, *verticalDir, mSideDir);
        if (!al::tryNormalizeOrZero(&moveDir))
            return;
    }

    const sead::Vector3f up = -al::getGravity(mActor);
    if (al::isParallelDirection(up, moveDir, 0.01f))
        return;

    std::memcpy(startMoveDir, &moveDir, sizeof(moveDir));
    mSideDir.setCross(up, *startMoveDir);
    al::normalize(&mSideDir);
}

// NON_MATCHING: 1096 bytes versus the 1228-byte target; next recover the missing turn/collision branch structure and its retained velocity temporaries.
void HackerActionAirMoveControl::update() {
    if (mIsScaleWallVelocity) {
        if (mCollision) {
            if (!rs::isCollisionCodeGrabCeilWall(mCollision))
                rs::scaleVelocityInertiaWallHit(mActor, mCollision, mScaleWallFront,
                                                mScaleWallSide, mScaleWallUp);
        } else if (al::isCollidedWall(mActor)) {
            if (!al::isCollidedWall(mActor) ||
                !al::isFloorCode(al::getActorCollider(mActor)->getWallHit().triangle, "GrabCeil"))
                al::scaleVelocityInertiaWallHit(mActor, mScaleWallFront, mScaleWallSide,
                                                mScaleWallUp);
        }
    }

    const s32 nextFrame = mFrame + 1;
    mFrame = nextFrame <= mExtendFrame + 1 ? nextFrame : mExtendFrame + 1;
    if (mIsHoldJumpExtend && mFrame <= mExtendFrame) {
        if (mIsForceJumpExtend) {
            mIsHoldJumpExtend = true;
        } else if (mIsUseAnyButton) {
            mIsHoldJumpExtend = rs::isHoldHackAnyButton(*mPlayerHack);
        } else {
            mIsHoldJumpExtend = rs::isHoldHackJump(*mPlayerHack);
        }
    } else {
        al::addVelocityToGravity(mActor, mGravityAccel);
    }

    const sead::Vector3f gravity = al::getGravity(mActor);
    sead::Vector3f moveInput = sead::Vector3f::zero;
    const sead::Vector3f* moveDir = &mStartMoveDir;
    if (mNoInputFrame == 0) {
        const sead::Vector3f up = -gravity;
        rs::calcHackerMoveVec(&moveInput, *mPlayerHack, up);

        al::verticalizeVec(&mStartMoveDir, gravity, mStartMoveDir);
        if (!al::tryNormalizeOrZero(&mStartMoveDir)) {
            al::calcFrontDir(&mStartMoveDir, mActor);
            al::verticalizeVec(&mStartMoveDir, gravity, mStartMoveDir);
            if (!al::tryNormalizeOrZero(&mStartMoveDir))
                al::calcUpDir(&mStartMoveDir, mActor);
            mSideDir.setCross(mStartMoveDir, gravity);
            al::normalize(&mSideDir);
        }

        f32 velocityFront = al::getVelocity(mActor).dot(mStartMoveDir);
        if (velocityFront < 0.0f)
            mStartMoveDir = -mStartMoveDir;

        const f32 inputFront = mStartMoveDir.dot(moveInput);
        const f32 accel = inputFront > 0.0f || al::isNearZero(velocityFront, 0.001f) ?
                              mAccelFront :
                              mAccelBack;
        al::addVelocity(mActor, accel * inputFront * mStartMoveDir);
        const sead::Vector3f inputSide = moveInput - inputFront * mStartMoveDir;
        al::addVelocity(mActor, mAccelTurn * inputSide);
        moveDir = &mStartMoveDir;
    }

    al::limitVelocityDir(mActor, *moveDir, mSpeedH);
    al::limitVelocityDir(mActor, mSideDir, mSpeedH);
    if (al::getVelocity(mActor).dot(gravity) > 0.0f)
        al::limitVelocityDir(mActor, gravity, mFallSpeedMax);

    if (mIsSlerpGravity) {
        rs::slerpGravity(mActor, mSlerpQuatRate);
    } else {
        const sead::Vector3f up = -gravity;
        const sead::Vector3f& turnInput = al::isNearZero(moveInput, 0.001f) ? mPrevInput : moveInput;
        mTurnControl->update(turnInput, up);
        if (mTurnControl->get_6a())
            rs::slerpSkyPoseAir(mActor, up, mTurnControl->get_5c(), mSlerpQuatRate);
        else
            rs::slerpUpFront(mActor, up, mTurnControl->get_5c(), mSlerpQuatRate, 0.0f);
    }

    mNoInputFrame--;
    if (mNoInputFrame < 0)
        mNoInputFrame = 0;
}

bool HackerActionAirMoveControl::isHoldJumpExtend() const {
    if (mIsForceJumpExtend)
        return true;
    if (mIsUseAnyButton)
        return rs::isHoldHackAnyButton(*mPlayerHack);
    return rs::isHoldHackJump(*mPlayerHack);
}

void HackerActionAirMoveControl::calcMoveInput(sead::Vector3f* moveInput,
                                               const sead::Vector3f& up) const {
    rs::calcHackerMoveVec(moveInput, *mPlayerHack, up);
}
