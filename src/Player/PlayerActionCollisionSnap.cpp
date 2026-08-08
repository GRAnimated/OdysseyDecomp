#include "Player/PlayerActionCollisionSnap.h"

#include <math/seadMathCalcCommon.h>

#include "Library/Collision/CollisionParts.h"
#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/PartsConnectorUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseKeeper.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/MatrixUtil.h"

#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

PlayerActionCollisionSnap::PlayerActionCollisionSnap(al::LiveActor* actor,
                                                     IUsePlayerCollision* collision)
    : mActor(actor), mCollision(collision), mState(&mSnapMtx),
      mMoveFrame(0), mMoveStep(0), mPreviousSnapParts(nullptr),
      mMoveStartMtx(sead::Matrix34f::ident), mMoveEndMtx(sead::Matrix34f::ident), _108(false) {
    mState.mtxConnector = al::createMtxConnector(actor);
}

void PlayerActionCollisionSnap::setup(const al::CollisionParts* parts,
                                      const sead::Vector3f& position, const sead::Vector3f& front,
                                      const sead::Vector3f& up) {
    mState.snapPos = position;
    mState.snapFront = front;
    mState.snapUp = up;
    mState.snapParts = parts;
    makeSnapPose(&mSnapMtx);
    al::attachMtxConnectorToCollisionParts(mState.mtxConnector, mState.snapParts);
    mMoveFrame = 0;
    mPreviousSnapParts = nullptr;
    mState.forceMovePower = {0.0f, 0.0f, 0.0f};
    mState._3c = {0.0f, 0.0f, 0.0f};
    mState._48 = {0.0f, 0.0f, 0.0f};
    mMoveStartMtx.makeIdentity();
    mMoveEndMtx.makeIdentity();
}

void PlayerActionCollisionSnap::makeSnapPose(sead::Matrix34f* outMtx) {
    if (!_108) {
        al::makeMtxUpFrontPos(outMtx, mState.snapUp, mState.snapFront, mState.snapPos);
        return;
    }

    al::LiveActor* actor = mActor;
    const sead::Vector3f& snapUp = mState.snapUp;
    const sead::Vector3f gravity = al::getGravity(actor);
    sead::Vector3f vertical(0.0f, 0.0f, 0.0f);
    al::verticalizeVec(&vertical, gravity, mState.snapFront);
    if (al::tryNormalizeOrZero(&vertical)) {
        al::makeMtxFrontUpPos(outMtx, mState.snapFront, snapUp, mState.snapPos);
        return;
    }

    const f32 upGravity = sead::Mathf::abs(gravity.dot(snapUp));
    const f32 frontGravity = sead::Mathf::abs(gravity.dot(vertical));
    if (al::isNearZeroOrGreater(upGravity - frontGravity, 0.001f)) {
        al::makeMtxUpFrontPos(outMtx, mState.snapUp, mState.snapFront, mState.snapPos);
        return;
    }
    al::makeMtxFrontUpPos(outMtx, mState.snapFront, snapUp, mState.snapPos);
}

void PlayerActionCollisionSnap::start() {
    al::resetPosition(mActor, mState.snapPos);
    rs::resetCollision(mCollision);
    rs::slerpUpFront(mActor, mState.snapUp, mState.snapFront, 1.0f, 90.0f);
    startCommon();
}

void PlayerActionCollisionSnap::startCommon() {
    al::setVelocityZero(mActor);
    mState.forceMovePower = {0.0f, 0.0f, 0.0f};
    if (mState.snapParts)
        al::calcForceMovePower(&mState.forceMovePower, mState.snapParts, al::getTrans(mActor));

    mState._48 = mState.snapPos + mState.forceMovePower;
    mState._3c = mState.snapPos;
}

void PlayerActionCollisionSnap::startWallCatch() {
    al::resetPosition(mActor, mState.snapPos);
    rs::resetCollision(mCollision);
    const sead::Vector3f up = -al::getGravity(mActor);
    const f32 upAngle = sead::Mathf::abs(al::calcAngleDegree(up, mState.snapUp));
    const f32 frontAngle =
        sead::Mathf::abs(sead::Mathf::abs(al::calcAngleDegree(up, mState.snapFront)) - 90.0f);
    rs::slerpUpFront(mActor, mState.snapUp, mState.snapFront, 1.0f,
                     upAngle < frontAngle ? upAngle : frontAngle);
    startCommon();
}

void PlayerActionCollisionSnap::moveSnapPos(const al::CollisionParts* parts,
                                            const sead::Vector3f& position,
                                            const sead::Vector3f& front, const sead::Vector3f& up,
                                            s32 frame) {
    mPreviousSnapParts = mState.snapParts;
    mState.snapPos = position;
    mState.snapFront = front;
    mState.snapUp = up;
    mState.snapParts = parts;
    al::attachMtxConnectorToCollisionParts(mState.mtxConnector, parts);
    mMoveFrame = frame;
    mMoveStep = 0;
    al::makeMtxRT(&mMoveStartMtx, mActor);
    mMoveStartMtx.m[0][3] = mState._3c.x;
    mMoveStartMtx.m[1][3] = mState._3c.y;
    mMoveStartMtx.m[2][3] = mState._3c.z;
    makeSnapPose(&mMoveEndMtx);
    mSnapMtx = mMoveStartMtx;
}

void PlayerActionCollisionSnap::resetSnapPos(const sead::Vector3f& position) {
    mMoveFrame = 0;
    mMoveStep = 0;
    mState.snapPos = position;
    al::calcFrontDir(&mState.snapFront, mActor);
    al::calcUpDir(&mState.snapUp, mActor);
    makeSnapPose(&mSnapMtx);
    al::attachMtxConnectorToCollisionParts(mState.mtxConnector, mState.snapParts);
    mMoveStartMtx.makeIdentity();
    mMoveEndMtx.makeIdentity();
}

void PlayerActionCollisionSnap::setSnapPose(const sead::Vector3f& front, const sead::Vector3f& up) {
    mState.snapFront = front;
    mState.snapUp = up;
    updateSnapPose();
}

void PlayerActionCollisionSnap::updateSnapPose() {
    if (mMoveFrame == 0) {
        makeSnapPose(&mSnapMtx);
        return;
    }
    makeSnapPose(&mMoveEndMtx);
    if (mMoveStep >= mMoveFrame) {
        mMoveStep = mMoveFrame - 1;
        mMoveStep = al::converge(mMoveStep, mMoveFrame, 1);
        f32 rate = static_cast<f32>(mMoveStep) / static_cast<f32>(mMoveFrame);
        if (!(rate < 0.0f)) {
            if (rate > 1.0f)
                rate = 1.0f;
        } else {
            rate = 0.0f;
        }
        const f32 easedRate = al::easeOut(rate);
        al::blendMtx(&mSnapMtx, mMoveStartMtx, mMoveEndMtx, easedRate);
    }
}

void PlayerActionCollisionSnap::turnSnapFrontAxisUp(f32 degree) {
    al::rotateVectorDegree(&mState.snapFront, mState.snapFront, mState.snapUp, degree);
    al::normalize(&mState.snapFront);
    updateSnapPose();
}

void PlayerActionCollisionSnap::rotateSnapPoseAxisFront(f32 degree) {
    al::rotateVectorDegree(&mState.snapUp, mState.snapUp, mState.snapFront, degree);
    al::normalize(&mState.snapUp);
    updateSnapPose();
}

void PlayerActionCollisionSnap::rotateSnapPoseAxisSide(f32 degree) {
    sead::Vector3f side = mState.snapUp.cross(mState.snapFront);
    al::normalize(&side);
    al::rotateVectorDegree(&mState.snapFront, mState.snapFront, side, degree);
    al::normalize(&mState.snapFront);
    al::rotateVectorDegree(&mState.snapUp, mState.snapUp, side, degree);
    al::normalize(&mState.snapUp);
    updateSnapPose();
}

void PlayerActionCollisionSnap::rotateSnapPoseWithAxis(const sead::Vector3f& axis, f32 degree) {
    al::rotateVectorDegree(&mState.snapFront, mState.snapFront, axis, degree);
    al::normalize(&mState.snapFront);
    al::rotateVectorDegree(&mState.snapUp, mState.snapUp, axis, degree);
    al::normalize(&mState.snapUp);
    if (mMoveFrame == 0) {
        makeSnapPose(&mSnapMtx);
        return;
    }
    makeSnapPose(&mMoveEndMtx);
    if (mMoveStep >= mMoveFrame) {
        mMoveStep = mMoveFrame - 1;
        mMoveStep = al::converge(mMoveStep, mMoveFrame, 1);
        f32 rate = static_cast<f32>(mMoveStep) / static_cast<f32>(mMoveFrame);
        if (!(rate < 0.0f)) {
            if (rate > 1.0f)
                rate = 1.0f;
        } else {
            rate = 0.0f;
        }
        const f32 easedRate = al::easeOut(rate);
        al::blendMtx(&mSnapMtx, mMoveStartMtx, mMoveEndMtx, easedRate);
    }
}

void PlayerActionCollisionSnap::updateMove() {
    if (mMoveStep < mMoveFrame) {
        mMoveStep = al::converge(mMoveStep, mMoveFrame, 1);
        f32 rate = static_cast<f32>(mMoveStep) / static_cast<f32>(mMoveFrame);
        if (!(rate < 0.0f)) {
            if (rate > 1.0f)
                rate = 1.0f;
        } else {
            rate = 0.0f;
        }
        const f32 easedRate = al::easeOut(rate);
        al::blendMtx(&mSnapMtx, mMoveStartMtx, mMoveEndMtx, easedRate);
    }
}

void PlayerActionCollisionSnap::restartMoveCurrentMtx(s32 frame) {
    mMoveFrame = frame;
    mMoveStep = 0;
    mMoveStartMtx = mSnapMtx;
}

void PlayerActionCollisionSnap::forceMoveEndNearestLeaveDir(const sead::Vector3f& leaveDir) {
    if (mMoveFrame == 0) {
        followCollision();
        return;
    }

    sead::Matrix34f startConnected = sead::Matrix34f::ident;
    al::calcConnectMtx(&startConnected, mState.mtxConnector, mMoveStartMtx);
    sead::Vector3f startFront = startConnected.getBase(2);
    al::normalize(&startFront);
    const f32 startDot = sead::Mathf::abs(startFront.dot(leaveDir));

    sead::Vector3f endFront;
    sead::Matrix34f endConnected = sead::Matrix34f::ident;
    al::calcConnectMtx(&endConnected, mState.mtxConnector, mMoveEndMtx);
    endFront = endConnected.getBase(2);
    al::normalize(&endFront);
    const f32 endDot = sead::Mathf::abs(endFront.dot(leaveDir));

    if (al::isNearZeroOrGreater(endDot - startDot, 0.001f))
        skipMove();
    else
        cancelMove();

    al::resetPosition(mActor);
    rs::resetCollision(mCollision);
}

void PlayerActionCollisionSnap::followCollision() {
    mState._3c = al::getTrans(mActor);
    if (mState.snapParts)
        al::calcForceMovePower(&mState.forceMovePower, mState.snapParts, al::getTrans(mActor));
    al::connectPoseMtx(mActor, mState.mtxConnector, mSnapMtx);
    const sead::Vector3f& actorTrans = al::getTrans(mActor);
    std::memcpy(&mState._48, &actorTrans, sizeof(mState._48));

    sead::Vector3f reaction = rs::getCollidedFixReaction(mCollision);
    if (al::tryNormalizeOrZero(&reaction)) {
        sead::Vector3f delta = mState._48 - mState._3c;
        const f32 reactionPower = delta.dot(reaction);
        if (reactionPower < 0.0f) {
            delta -= reaction * reactionPower;
            mState._48.setAdd(mState._3c, delta);
        }
    }
}

void PlayerActionCollisionSnap::skipMove() {
    if (mMoveFrame != 0) {
        mMoveStep = mMoveFrame;
        al::blendMtx(&mSnapMtx, mMoveStartMtx, mMoveEndMtx, 1.0f);
    }
    al::connectPoseMtx(mActor, mState.mtxConnector, mSnapMtx);
    followCollision();
    const sead::Vector3f& trans = al::getTrans(mActor);
    std::memcpy(&mState._48, &trans, sizeof(mState._48));
}

void PlayerActionCollisionSnap::cancelMove() {
    if (mMoveFrame != 0) {
        mMoveStep = mMoveFrame;
        al::blendMtx(&mSnapMtx, mMoveStartMtx, mMoveEndMtx, 0.0f);
    }
    al::connectPoseMtx(mActor, mState.mtxConnector, mSnapMtx);
    followCollision();
    const sead::Vector3f& trans = al::getTrans(mActor);
    std::memcpy(&mState._48, &trans, sizeof(mState._48));
}

void PlayerActionCollisionSnap::updateInertia() {
    if (mState.snapParts)
        al::calcForceMovePower(&mState.forceMovePower, mState.snapParts, al::getTrans(mActor));
}

namespace {
void adjustEndFallMove(sead::Vector3f* move, const sead::Vector3f& snapPos,
                       const sead::Vector3f& collidedPos, const sead::Vector3f& normal,
                       f32 collisionOffset) {
    const f32 projection = move->dot(normal);
    if (!al::isNearZeroOrGreater(projection, 0.001f)) {
        const f32 distance = (collidedPos - snapPos).dot(normal);
        if (projection - collisionOffset > distance)
            return;
        const f32 push = sead::Mathf::clampMin(distance * 0.5f - projection, 0.0f);
        move->x += normal.x * push;
        move->y += normal.y * push;
        move->z += normal.z * push;
    }
}
}  // namespace

void PlayerActionCollisionSnap::endFall(f32 velocity, const sead::Vector3f& move,
                                        f32 collisionOffset) {
    const sead::Vector3f snapPos = mState._48;
    sead::Vector3f correctedMove = move;
    if (rs::isCollidedGround(mCollision))
        adjustEndFallMove(&correctedMove, snapPos, rs::getCollidedGroundPos(mCollision),
                          rs::getCollidedGroundNormal(mCollision), collisionOffset);
    if (rs::isCollidedWall(mCollision))
        adjustEndFallMove(&correctedMove, snapPos, rs::getCollidedWallPos(mCollision),
                          rs::getCollidedWallNormal(mCollision), collisionOffset);
    if (rs::isCollidedCeiling(mCollision))
        adjustEndFallMove(&correctedMove, snapPos, rs::getCollidedCeilingPos(mCollision),
                          rs::getCollidedCeilingNormal(mCollision), collisionOffset);

    al::resetPosition(mActor, snapPos + correctedMove);
    rs::resetCollisionExpandCheck(mCollision);
    al::setVelocityToGravity(mActor, -velocity);
    sead::Vector3f front(0.0f, 0.0f, 0.0f);
    al::calcFrontDir(&front, mActor);
    const f32 frontPower = sead::Mathf::clampMax(mState.forceMovePower.dot(front), 0.0f);
    const sead::Vector3f velocityAdd = front * frontPower;
    al::addVelocity(mActor, velocityAdd);
    al::setTrans(mActor, al::getTrans(mActor) + velocityAdd);
}

bool PlayerActionCollisionSnap::isSnapPartsMoving() const {
    if (mState.snapParts && al::isCollisionMoving(mState.snapParts))
        return true;
    if (mPreviousSnapParts && al::isCollisionMoving(mPreviousSnapParts))
        return true;
    return false;
}

bool PlayerActionCollisionSnap::isSnapPartsValid() const {
    if (mState.snapParts && al::isCollisionValid(mState.snapParts))
        return true;
    if (mPreviousSnapParts && al::isCollisionValid(mPreviousSnapParts))
        return true;
    return false;
}

bool PlayerActionCollisionSnap::isSnapParts(const al::CollisionParts* parts) const {
    return mState.snapParts == parts || mPreviousSnapParts == parts;
}

al::HitSensor* PlayerActionCollisionSnap::tryGetConnectedSensor() const {
    if (mState.snapParts && al::isCollisionValid(mState.snapParts))
        return mState.snapParts->getConnectedSensor();
    return nullptr;
}

void PlayerActionCollisionSnap::calcFollowDir(sead::Vector3f* outDir,
                                              const sead::Vector3f& dir) const {
    al::calcConnectDir(outDir, mState.mtxConnector, dir);
}
