#include "Npc/YukimaruStateMove.h"

#include <cstring>
#include <math/seadQuat.h>

#include "Library/Effect/EffectSystemInfo.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Matrix/MatrixUtil.h"
#include "Library/Movement/AnimScaleController.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Se/SeFunction.h"

#include "Npc/YukimaruInput.h"
#include "Util/ObjUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(YukimaruStateMove, JumpEnd);

al::AnimScaleParam sScaleParam(0.1f, 0.85f, 0.2f, 1.8f, 0.06f, 0.12f, 0.91f, 20, 0.25f, 0.9f, 5.2f,
                               0.05f);
al::AnimScaleParam sBoundScaleParam(0.2f, 0.85f, 0.6f, 1.3f, 0.06f, 0.12f, 0.91f, 20, 0.25f, 0.9f,
                                    5.2f, 0.05f);

YukimaruInput sNullInput;

NERVE_IMPL(YukimaruStateMove, RunStart);
NERVE_IMPL(YukimaruStateMove, Run);
NERVE_IMPL(YukimaruStateMove, Jump);
NERVES_MAKE_NOSTRUCT(YukimaruStateMove, JumpEnd, RunStart, Run, Jump);
}  // namespace

YukimaruStateMove::YukimaruStateMove(al::LiveActor* actor, YukimaruInput* input, sead::Quatf* quat)
    : ActorStateBase("ユキマル移動状態", actor), mInput(input), mQuat(quat) {
    std::memcpy(&mEffectMtx, &sead::Matrix34f::ident, sizeof(sead::Matrix34f));
    mVelocity.set(sead::Vector3f::zero);
    mCollidedNormal.set(sead::Vector3f::ey);
    mRotation.set(sead::Vector3f::zero);
    mIsJumping = false;
    mIsHack = false;
    mSpeed = 0.0f;
    mJumpType = 0;
    mSandSinkTimer = 0;
    mGravityTimer = 0;
    mBounceCount = 0;
    mSlideTimer = 0;
    mInputTimer = 0;
    mCollisionBounceTimer = 0;
    mScaleController = new al::AnimScaleController(&sScaleParam);
    mBoundScaleController = new al::AnimScaleController(&sBoundScaleParam);
    al::setEffectNamedMtxPtr(mActor, "Land", &mEffectMtx);
    initNerve(&RunStart, 0);
}

void YukimaruStateMove::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    YukimaruMovement::attackSensor(mActor, self, other);
}

// clang-format off
bool YukimaruMovement::attackSensor(al::LiveActor* actor, al::HitSensor* self, al::HitSensor* other) {
    // clang-format on
    if (al::getVelocity(actor).y < 0.0f &&
        rs::trySendMsgPlayerReflectOrTrample(actor, self, other)) {
        al::setVelocityOnlyV(actor, 24.0f);
        return true;
    }

    if (rs::sendMsgYukimaruPush(other, self)) {
        const sead::Vector3f& velSelf = al::getActorVelocity(self);
        const sead::Vector3f& velOther = al::getActorVelocity(other);
        f32 dx = velSelf.x - velOther.x;
        f32 dy = velSelf.y - velOther.y;
        f32 dz = velSelf.z - velOther.z;

        sead::Vector3f dir;
        al::calcDirBetweenSensors(&dir, self, other);

        f32 dot = dx * dir.x + dy * dir.y + dz * dir.z;
        if (dot > 0.0f) {
            if (dot > 5.0f) {
                al::startHitReactionHitEffect(actor, "接触", self, other);
                al::LiveActor* otherHost = al::getSensorHost(other);
                al::startHitReaction(otherHost, "被接触");
            }
            al::LiveActor* selfHost = al::getSensorHost(self);
            al::addVelocityToDirection(selfHost, dir, dot * -1.3f);
            al::LiveActor* otherHost2 = al::getSensorHost(other);
            al::addVelocityToDirection(otherHost2, dir, dot * 1.3f);
        }
        return true;
    }

    return al::sendMsgPushAndKillVelocityToTarget(actor, self, other);
}

bool YukimaruStateMove::receiveMsg(const al::SensorMsg* msg, al::HitSensor* self,
                                   al::HitSensor* other) {
    if (al::isMsgPushAll(msg))
        return true;

    if (rs::isMsgYukimaruPush(msg)) {
        const sead::Vector3f& velOther = al::getActorVelocity(other);
        const sead::Vector3f& velSelf = al::getActorVelocity(self);
        f32 dx = velOther.x - velSelf.x;
        f32 dy = velOther.y - velSelf.y;
        f32 dz = velOther.z - velSelf.z;

        sead::Vector3f dir;
        al::calcDirBetweenSensors(&dir, other, self);

        f32 dot = dx * dir.x + dy * dir.y + dz * dir.z;
        if (dot > 0.0f) {
            if (dot > 5.0f)
                al::startHitReactionHitEffect(mActor, "接触", other, self);
            al::LiveActor* otherHost = al::getSensorHost(other);
            al::addVelocityToDirection(otherHost, dir, dot * -1.3f);
        }
        return true;
    }

    if (rs::isMsgPushToPlayer(msg)) {
        al::pushAndAddVelocityH(mActor, self, other, 1.3f);
        return true;
    }

    return false;
}

void YukimaruStateMove::startRun(bool isRunning) {
    mVelocity.set(sead::Vector3f::zero);
    mCollidedNormal.set(sead::Vector3f::ey);
    mRotation.set(sead::Vector3f::zero);
    mInputTimer = 0;
    mCollisionBounceTimer = 0;
    mSlideTimer = 0;
    mSandSinkTimer = 0;
    mGravityTimer = 0;
    mIsJumping = false;

    if (isRunning)
        al::setNerve(this, &Run);
    else
        al::setNerve(this, &RunStart);
}

void YukimaruStateMove::resetInput() {
    mVelocity.set(sead::Vector3f::zero);
    mCollidedNormal.set(sead::Vector3f::ey);
    mRotation.set(sead::Vector3f::zero);
    mInputTimer = 0;
    mCollisionBounceTimer = 0;
    mSlideTimer = 0;
    mSandSinkTimer = 0;
    mGravityTimer = 0;
    mIsJumping = false;
}

void YukimaruStateMove::exeRunStart() {
    if (al::isFirstStep(this))
        al::startAction(mActor, mIsHack ? "RunStartHack" : "RunStartEnemy");

    updateMove(mInput);

    if (mIsJumping)
        al::setNerve(this, &Jump);
    else if (al::isActionEnd(mActor))
        al::setNerve(this, &Run);
}

void YukimaruStateMove::updateMove() {
    updateMove(mInput);
}

void YukimaruStateMove::exeRun() {
    if (al::isFirstStep(this))
        al::startAction(mActor, mIsHack ? "RunHack" : "RunEnemy");

    updateMove(mInput);

    if (mIsJumping)
        al::setNerve(this, &Jump);
}

// NON_MATCHING: comparison order (compiler checks case 1 before case 2), instruction scheduling
void YukimaruStateMove::exeJump() {
    if (al::isFirstStep(this)) {
        sead::Vector3f rollMoment = sead::Vector3f::zero;

        const sead::Vector3f& vel = al::getVelocity(mActor);
        const sead::Vector3f& grav = al::getGravity(mActor);
        sead::Vector3f up = {-grav.x, -grav.y, -grav.z};

        al::calcMomentRollBall(&rollMoment, vel, up, 5.0f);
        al::tryNormalizeOrZero(&rollMoment);

        f32 scale;
        const char* actionName;

        if (mJumpType == 2) {
            scale = 0.8f;
            actionName = mIsHack ? "JumpHackHigh" : "JumpEnemyHigh";
        } else if (mJumpType == 1) {
            scale = 0.3f;
            actionName = mIsHack ? "JumpHackLow" : "JumpEnemyLow";
        } else {
            scale = 0.5f;
            actionName = mIsHack ? "JumpHack" : "JumpEnemy";
        }

        rollMoment *= scale;
        mRotation += rollMoment;

        al::startAction(mActor, actionName);
    }

    updateMove(mInput);

    if (mCollisionBounceTimer > 0 || al::isActionEnd(mActor))
        al::setNerve(this, &JumpEnd);
}

// NON_MATCHING: compiler merges redundant mIsHack test instructions, ADRP scheduling
void YukimaruStateMove::exeJumpEnd() {
    if (al::isFirstStep(this)) {
        s32 jumpType = mJumpType;
        const char* actionHigh = mIsHack ? "JumpEndHackHigh" : "JumpEndEnemyHigh";
        const char* actionNormal = mIsHack ? "JumpEndHack" : "JumpEndEnemy";
        const char* actionLow = mIsHack ? "JumpEndHackLow" : "JumpEndEnemyLow";

        const char* action = jumpType != 1 ? actionNormal : actionLow;
        action = jumpType == 2 ? actionHigh : action;
        al::startAction(mActor, action);
    }

    updateMove(mInput);

    if (mIsJumping)
        al::setNerve(this, &Jump);
    else if (al::isActionEnd(mActor))
        al::setNerve(this, &Run);
}

void YukimaruStateMove::updateMoveNoInput() {
    updateMove(&sNullInput);
}

// NON_MATCHING: regswap, instruction scheduling, sead operator differences
void YukimaruStateMove::updateMove(YukimaruInput* input) {
    mIsJumping = false;

    if (input->isTriggerJump())
        mInputTimer = 10;
    else if (mInputTimer > 0)
        mInputTimer--;

    if (mSandSinkTimer > 0)
        mSandSinkTimer--;

    if (mGravityTimer > 0)
        mGravityTimer--;

    if (al::isCollidedFloorCode(mActor, "SandSink"))
        mSandSinkTimer = 10;

    if (al::isCollidedVelocity(mActor)) {
        mGravityTimer = 0;
        al::calcCollidedNormalSum(mActor, &mCollidedNormal);
        al::tryNormalizeOrZero(&mCollidedNormal);
        mCollisionBounceTimer = 12;
        mSlideTimer = 0;
        mBounceCount = mBounceCount + 1;
    } else if (mCollisionBounceTimer > 0) {
        mCollisionBounceTimer--;
        if (mCollisionBounceTimer == 0)
            mBounceCount = 0;
        else
            mBounceCount = mBounceCount + 1;
    } else {
        mBounceCount = 0;
    }

    if (mSlideTimer > 0)
        mSlideTimer--;

    if (!input->isHoldJump())
        mSlideTimer = 0;

    sead::Vector3f inputDir;
    input->calcInputVec(&inputDir);

    bool isOnGround = al::isOnGroundNoVelocity(mActor, 10);

    f32 moveSpeed = al::normalize(mBounceCount, 5, 60);
    moveSpeed = al::lerpValue(2.0f, 1.6f, moveSpeed);
    if (!isOnGround)
        moveSpeed = 0.8f;

    mVelocity = (inputDir * moveSpeed + mVelocity) * 0.5f;

    sead::Vector3f horizVel = mVelocity;
    horizVel.y = 0.0f;
    if (!al::isNearZero(horizVel, 0.001f)) {
        sead::Quatf* quatPtr = al::getQuatPtr(mActor);
        al::makeQuatUpFront(quatPtr, sead::Vector3f::ey, horizVel);
    }

    sead::Vector3f collNormal;
    collNormal.set(sead::Vector3f::ey);
    al::calcCollidedNormalSum(mActor, &collNormal);
    if (!al::tryNormalizeOrZero(&collNormal))
        collNormal.set(sead::Vector3f::ey);

    sead::Vector3f collidedPos;
    collidedPos.set(sead::Vector3f::zero);
    if (al::tryGetCollidedPos(&collidedPos, mActor))
        al::makeMtxUpNoSupportPos(&mEffectMtx, collNormal, collidedPos);

    f32 speedV = al::calcSpeedV(mActor);
    bool bounced = al::reboundVelocityFromCollision(mActor, 0.7f, 10.0f, 1.0f);

    if (bounced) {
        f32 absSpeedV = speedV > 0.0f ? speedV : -speedV;
        f32 normalY = collNormal.y;
        f32 scaleY = al::lerpValue(absSpeedV, 5.0f, 30.0f, 0.87f, 0.75f);
        mBoundScaleController->startAndSetScaleY(scaleY);

        const char* reactionName;
        const char* reactionBound = "バウンド[敵]";
        const char* reactionWall = "壁バウンド[敵]";
        if (normalY >= 0.17365f) {
            reactionBound = "バウンド";
            reactionWall = "壁バウンド";
        }
        if (mIsHack)
            reactionName = reactionBound;
        else
            reactionName = reactionWall;
        al::startHitReaction(mActor, reactionName);
    }

    al::addVelocity(mActor, mVelocity);

    f32 gravity = mSlideTimer > 0 ? 1.5f : 2.0f;
    al::addVelocityToGravity(mActor, gravity);

    f32 frictionH = isOnGround ? 0.98f : 0.992f;
    al::scaleVelocityHV(mActor, frictionH, 0.997f);

    if (mSandSinkTimer >= 1)
        al::scaleVelocityHV(mActor, 0.96f, 0.8f);

    f32 speedH = al::calcSpeedH(mActor);
    f32 slipAmount = 0.0f;
    f32 normalizedSpeed = al::normalize(speedH, 0.0f, 90.0f);

    if (mCollisionBounceTimer >= 1) {
        sead::Vector3f rollBig;
        const sead::Vector3f& vel = al::getVelocity(mActor);
        al::calcMomentRollBall(&rollBig, vel, mCollidedNormal, 120.0f);

        const sead::Vector3f& grav = al::getGravity(mActor);
        sead::Vector3f up = {-grav.x, -grav.y, -grav.z};
        sead::Vector3f rollSmall;
        al::calcMomentRollBall(&rollSmall, mRotation, up, 5.0f);

        const sead::Vector3f& vel2 = al::getVelocity(mActor);
        f32 speed = sead::Mathf::max(vel2.length(), 60.0f);

        const sead::Vector3f& vel3 = al::getVelocity(mActor);
        sead::Vector3f diff = vel3 * (1.0f / speed) - inputDir;

        f32 diffLen = diff.length();
        f32 inputLen = inputDir.length();

        slipAmount = al::normalize(diffLen * inputLen, 0.6f, 1.2f);
        f32 rotLen = mVelocity.length();
        f32 blendT = al::lerpValue(rotLen, 0.0f, 1.5f, 0.0f, 1.0f);
        al::lerpVec(&mRotation, rollBig, rollSmall, blendT);

        al::tryHoldSeWithParam(mActor, "GroundLv", normalizedSpeed, "接地速度");
    } else {
        sead::Vector3f rollMoment = sead::Vector3f::zero;
        const sead::Vector3f& grav = al::getGravity(mActor);
        sead::Vector3f up = {-grav.x, -grav.y, -grav.z};
        al::calcMomentRollBall(&rollMoment, mRotation, up, 5.0f);

        f32 gravityFactor = al::lerpValue((f32)mGravityTimer, 0.0f, 30.0f, 1.0f, 0.0f);
        gravityFactor = al::easeInOut(gravityFactor);
        f32 rollScale = gravityFactor * 0.1f;

        rollMoment *= rollScale;
        mRotation += rollMoment;

        f32 damping = input->isHoldJump() ? 0.97f : 0.95f;
        mRotation *= damping;
    }

    if (normalizedSpeed <= 0.1f || mCollisionBounceTimer < 1)
        al::tryDeleteEffect(mActor, "Ground");
    else
        al::tryEmitEffect(mActor, "Ground", nullptr);

    f32 rollingSpeed = al::normalize(mRotation.length(), 0.0f, 0.4f);
    al::tryHoldSeWithParam(mActor, "RollingLv", rollingSpeed, "回転速度");

    if (slipAmount <= 0.0f) {
        al::tryDeleteEffect(mActor, "Slip");
    } else {
        al::tryHoldSeWithParam(mActor, "SlipLv", slipAmount, "スリップ度合");
        al::tryEmitEffect(mActor, "Slip", nullptr);
    }

    al::rotateQuatMoment(mQuat, *mQuat, mRotation);

    if (mInputTimer >= 1 && mCollisionBounceTimer >= 1) {
        sead::Vector3f normal = mCollidedNormal;

        if (normal.dot(mVelocity) < 0.0f) {
            sead::Vector3f vertVec;
            if (al::tryNormalizeOrZero(&vertVec, mRotation))
                al::verticalizeVec(&normal, vertVec, normal);
        }

        sead::Vector3f jumpVel = normal * 24.0f;
        al::addVelocity(mActor, jumpVel);

        const sead::Vector3f& curVel = al::getVelocity(mActor);
        f32 dotVel = normal.dot(curVel);
        mSpeed = dotVel;

        if (dotVel < 40.0f) {
            f32 boost = 40.0f - dotVel;
            sead::Vector3f boostVel = normal * boost;
            al::addVelocity(mActor, boostVel);
            const sead::Vector3f& newVel = al::getVelocity(mActor);
            dotVel = normal.dot(newVel);
            mSpeed = dotVel;
        }

        mJumpType = 0;
        if (dotVel < 50.0f || mSandSinkTimer >= 1)
            mJumpType = 1;
        if (dotVel > 70.0f)
            mJumpType = 2;

        mInputTimer = 0;
        mCollisionBounceTimer = 0;
        mRotation *= 0.5f;
        mSlideTimer = 30;
        mGravityTimer = 45;
        mIsJumping = true;

        mScaleController->startAndSetScaleAnsVelocityY(1.0f, 0.15f);
    }

    mScaleController->update();
    mBoundScaleController->update();

    f32 sy = mScaleController->getScale().y;
    const sead::Vector3f& boundScale = mBoundScaleController->getScale();
    sead::Vector3f finalScale = {sy * boundScale.x, sy * boundScale.y, sy * boundScale.z};
    al::setScale(mActor, finalScale);
}

void YukimaruStateMove::reactionBound(f32 speed, bool isUpward) {
    f32 absSpeed = speed > 0.0f ? speed : -speed;
    f32 scaleY = al::lerpValue(absSpeed, 5.0f, 30.0f, 0.87f, 0.75f);
    mBoundScaleController->startAndSetScaleY(scaleY);

    const char* reactionName;
    if (mIsHack)
        if (isUpward)
            reactionName = "バウンド";
        else
            reactionName = "壁バウンド";
    else if (isUpward)
        reactionName = "バウンド[敵]";
    else
        reactionName = "壁バウンド[敵]";
    al::startHitReaction(mActor, reactionName);
}

void YukimaruStateMove::updateScale() {
    mScaleController->update();
    mBoundScaleController->update();
    f32 sy = mScaleController->getScale().y;
    const sead::Vector3f& boundScale = mBoundScaleController->getScale();
    al::LiveActor* actor = mActor;
    sead::Vector3f finalScale = {sy * boundScale.x, sy * boundScale.y, sy * boundScale.z};
    al::setScale(actor, finalScale);
}

void YukimaruStateMove::startScaleBoundReaction(f32 speed) {
    f32 absSpeed = speed > 0.0f ? speed : -speed;
    f32 scaleY = al::lerpValue(absSpeed, 5.0f, 30.0f, 0.87f, 0.75f);
    mBoundScaleController->startAndSetScaleY(scaleY);
}

void YukimaruStateMove::resetEffect() {
    al::tryDeleteEffect(mActor, "Ground");
    al::tryDeleteEffect(mActor, "Slip");
}

void YukimaruMovement::updateVelocity(al::LiveActor* actor) {
    bool isOnGround = al::isOnGroundNoVelocity(actor, 10);
    al::reboundVelocityFromCollision(actor, 0.0f, 0.0f, 1.0f);
    al::addVelocityToGravity(actor, 2.0f);
    f32 frictionH = isOnGround ? 0.98f : 0.992f;
    al::scaleVelocityHV(actor, frictionH, 0.997f);
}
