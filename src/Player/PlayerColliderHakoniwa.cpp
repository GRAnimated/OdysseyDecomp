#include "Player/PlayerColliderHakoniwa.h"

#include <cmath>
#include <cstring>
#include <math/seadQuat.h>

#include "Library/Area/AreaObjUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"

#include "Player/CollisionShapeKeeper.h"
#include "Player/PlayerCeilingCheck.h"
#include "Player/PlayerCollider.h"
#include "Player/PlayerConst.h"
#include "Util/ActorDimensionUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
void createCollisionShapeArrows(CollisionShapeKeeper* collisionShapeKeeper,
                                const sead::Vector3f& pos) {
    const char* shapeNames[] = {"LegFront", "LegLeft", "LegRight"};
    sead::Vector3f up = sead::Vector3f::ey;
    sead::Vector3f upper = -40.0f * up;
    sead::Vector3f lower = 20.0f * up;
    sead::Vector3f radial = 30.0f * sead::Vector3f::ez;

    for (s32 i = 0; i < 3; i++) {
        sead::Quatf quat;
        sead::QuatCalcCommon<f32>::setAxisAngle(quat, sead::Vector3f::ey, i * 120.0f);
        sead::Vector3f start;
        start.setRotated(quat, radial);
        start += pos;
        start -= upper;
        collisionShapeKeeper->createShapeArrow(shapeNames[i], start, upper - lower, 20.0f, i);
    }
}

void createCollisionShapeArrows2D(CollisionShapeKeeper* collisionShapeKeeper) {
    sead::Vector3f upper = -40.0f * sead::Vector3f::ey;
    sead::Vector3f lower = 20.0f * sead::Vector3f::ey;
    sead::Vector3f end = upper - lower;
    sead::Vector3f center = sead::Vector3f::zero - upper;
    sead::Vector3f side = 40.0f * sead::Vector3f::ez;
    collisionShapeKeeper->createShapeArrow("LegLeft", center + side, end, 20.0f, 0);
    collisionShapeKeeper->createShapeArrow("LegCenter", center, end, 20.0f, 1);
    collisionShapeKeeper->createShapeArrow("LegRight", center - side, end, 20.0f, 2);
}
}  // namespace

PlayerColliderHakoniwa::PlayerColliderHakoniwa(al::LiveActor* player,
                                               const PlayerConst* playerConst)
    : mPlayer(player), mConst(playerConst), mCollider(nullptr), mCeilingCheck(nullptr),
      mIsAboveGround(false), mGroundHeight(0.0f), mShadowDropHeight(0.0f),
      mGroundNormal(0.0f, 0.0f, 0.0f), mFallStartPos(0.0f, 0.0f, 0.0f), mFallDistance(0.0f),
      mNoGroundFrames(0), mGroundSensor(nullptr), mCollisionNormal(nullptr),
      mCollisionMini(nullptr), mCollisionSwim(nullptr), mCollisionGrabCeil(nullptr),
      mCollisionWallGrab(nullptr), mCollisionPoleClimb(nullptr), mCollision2DNormal(nullptr),
      mCollision2DMini(nullptr), mCollisionFilter2D(rs::createCollisionPartsFilter2DOnly()) {}

// NON_MATCHING: exact 1912-byte body; named wall-grab vector lifetimes align the target through
// 0x4361F0, with one remaining head-offset multiply scheduling delta at 0x4361F4.
void PlayerColliderHakoniwa::init() {
    mCollisionNormal = new CollisionShapeKeeper(5, 64, 32);
    mCollisionNormal->set54(25.0f);
    mCollisionNormal->set58(25.0f);
    createCollisionShapeArrows(mCollisionNormal, sead::Vector3f::zero);
    mCollisionNormal->createShapeSphereSupportGround("Body", 55.0f, sead::Vector3f::ey * 65.0f,
                                                     sead::Vector3f::ey, 14.5f);
    mCollisionNormal->createShapeSphere("Head", 55.0f, sead::Vector3f::ey * 105.0f);
    mCollisionNormal->updateShape();

    mCollisionMini = new CollisionShapeKeeper(5, 64, 32);
    createCollisionShapeArrows(mCollisionMini, sead::Vector3f::zero);
    mCollisionMini->createShapeSphereSupportGround("Body", 55.0f, sead::Vector3f::ey * 55.0f,
                                                   sead::Vector3f::ey, 14.5f);
    mCollisionMini->set58(25.0f);
    mCollisionMini->updateShape();

    mCollisionSwim = new CollisionShapeKeeper(5, 64, 32);
    mCollisionSwim->set54(25.0f);
    createCollisionShapeArrows(mCollisionSwim, sead::Vector3f::zero);
    mCollisionSwim->createShapeSphereIgnoreGround("Body", 55.0f, sead::Vector3f::ey * 55.0f);
    mCollisionSwim->createShapeSphere("Head", 55.0f, sead::Vector3f::ey * 105.0f);
    mCollisionSwim->updateShape();

    mCollisionGrabCeil = new CollisionShapeKeeper(5, 64, 32);
    createCollisionShapeArrows(mCollisionGrabCeil, sead::Vector3f::ey * -140.0f);
    mCollisionGrabCeil->createShapeSphereSupportGround("Body", 55.0f, sead::Vector3f::ey * -75.0f,
                                                       sead::Vector3f::ey, 30.0f);
    mCollisionGrabCeil->createShapeSphereIgnoreGround("Head", 55.0f, sead::Vector3f::ey * -35.0f);
    mCollisionGrabCeil->updateShape();

    mCollisionWallGrab = new CollisionShapeKeeper(5, 64, 32);
    const sead::Vector3f wallGrabVertical = sead::Vector3f::ey * 140.0f;
    const sead::Vector3f wallGrabDepth = sead::Vector3f::ez * -50.0f;
    createCollisionShapeArrows(mCollisionWallGrab, wallGrabDepth - wallGrabVertical);
    const sead::Vector3f wallGrabBodyVertical = sead::Vector3f::ey * 75.0f;
    const sead::Vector3f wallGrabBodyDepth = sead::Vector3f::ez * -50.0f;
    mCollisionWallGrab->createShapeSphereIgnoreGround("Body", 55.0f,
                                                      wallGrabBodyDepth - wallGrabBodyVertical);
    const sead::Vector3f wallGrabHeadVertical = sead::Vector3f::ey * 35.0f;
    const sead::Vector3f wallGrabHeadDepth = sead::Vector3f::ez * -50.0f;
    mCollisionWallGrab->createShapeSphereIgnoreGround("Head", 55.0f,
                                                      wallGrabHeadDepth - wallGrabHeadVertical);
    mCollisionWallGrab->updateShape();

    mCollisionPoleClimb = new CollisionShapeKeeper(5, 64, 32);
    const sead::Vector3f poleClimbVertical = sead::Vector3f::ey * 35.0f;
    const sead::Vector3f poleClimbDepth = sead::Vector3f::ez * -50.0f;
    createCollisionShapeArrows(mCollisionPoleClimb, poleClimbDepth - poleClimbVertical);
    mCollisionPoleClimb->createShapeSphereIgnoreGround(
        "Body", 55.0f, sead::Vector3f::ey * 30.0f - sead::Vector3f::ez * 50.0f);
    mCollisionPoleClimb->createShapeSphereIgnoreGround(
        "Head", 55.0f, sead::Vector3f::ey * 60.0f - sead::Vector3f::ez * 50.0f);
    mCollisionPoleClimb->updateShape();

    mCollision2DNormal = new CollisionShapeKeeper(5, 64, 32);
    createCollisionShapeArrows2D(mCollision2DNormal);
    mCollision2DNormal->createShapeDiskSupportGround("Body", 55.0f, sead::Vector3f::ey * 55.0f,
                                                     sead::Vector3f::ex, 5.0f, sead::Vector3f::ey,
                                                     14.5f);
    mCollision2DNormal->createShapeDisk("Head", 55.0f, sead::Vector3f::ey * 130.0f,
                                        sead::Vector3f::ex, 5.0f);
    mCollision2DNormal->updateShape();

    mCollision2DMini = new CollisionShapeKeeper(5, 64, 32);
    createCollisionShapeArrows2D(mCollision2DMini);
    mCollision2DMini->createShapeDiskSupportGround("Body", 55.0f, sead::Vector3f::ey * 55.0f,
                                                   sead::Vector3f::ex, 5.0f, sead::Vector3f::ey,
                                                   14.5f);
    mCollision2DMini->createShapeDisk("Head", 55.0f, sead::Vector3f::ey * 55.0f, sead::Vector3f::ex,
                                      5.0f);
    mCollision2DMini->updateShape();

    mCollider = new PlayerCollider(mPlayer->getCollisionDirector(), mPlayer->getBaseMtx(),
                                   al::getTransPtr(mPlayer), al::getGravityPtr(mPlayer), false);
    mCollider->validateCorrectMovePartsCheck();
    mCeilingCheck = new PlayerCeilingCheck(mPlayer->getCollisionDirector());
}

sead::Vector3f PlayerColliderHakoniwa::updateCollider(const sead::Vector3f& velocity) {
    mCollider->setInFastMoveCollisionArea(
        al::isInAreaObj(static_cast<const al::IUseAreaObj*>(mPlayer), "FastMoveCollisionArea",
                        al::getTrans(mPlayer)));
    sead::Vector3f result = mCollider->collide(velocity);

    if (rs::isCollidedGround(this)) {
        mGroundSensor = rs::tryGetCollidedGroundSensor(this);
        mNoGroundFrames = 0;
    } else {
        mNoGroundFrames = al::converge(mNoGroundFrames, 10000, 1);
    }
    return result;
}

void PlayerColliderHakoniwa::updateHeightCheck(const sead::Vector3f& trans,
                                               const sead::Vector3f& up, bool isGrounded) {
    if (rs::isCollidedGround(this) && isGrounded) {
        mGroundHeight = 0.0f;
        mIsAboveGround = true;
        mShadowDropHeight = 1700.0f;
        rs::calcGroundHeight(&mShadowDropHeight, &mGroundNormal, mCollider, trans, up, 30.0f,
                             1700.0f);
        const sead::Vector3f& groundNormal = rs::getCollidedGroundNormal(this);
        mGroundNormal = groundNormal;
    } else {
        mGroundHeight = 1700.0f;
        mIsAboveGround = rs::calcGroundHeight(&mGroundHeight, &mGroundNormal, mCollider, trans, up,
                                              30.0f, 1700.0f);
        mShadowDropHeight = mGroundHeight;
    }
}

void PlayerColliderHakoniwa::updateCeilingCheck(const sead::Vector3f& trans,
                                                const sead::Vector3f& up, f32 headClearance,
                                                f32 holdHeight) {
    mCeilingCheck->update(trans, up, mConst->getTall(), mConst->getCollisionRadiusSquat(),
                          headClearance, holdHeight);
}

void PlayerColliderHakoniwa::updateFallDistanceCheck(const sead::Vector3f& trans,
                                                     const sead::Vector3f& velocity,
                                                     const sead::Vector3f& gravity, f32 threshold) {
    if (rs::isOnGround(mPlayer, this) && !rs::isJustLand(this)) {
        mFallStartPos.set(0.0f, 0.0f, 0.0f);
        mFallDistance = 0.0f;
    } else {
        f32 velocityGravity = velocity.dot(gravity);
        if (velocityGravity < threshold) {
            mFallStartPos.set(0.0f, 0.0f, 0.0f);
            mFallDistance = 0.0f;
        } else {
            if (al::isNearZero(mFallStartPos, 0.001f))
                std::memcpy(&mFallStartPos, &trans, sizeof(mFallStartPos));

            sead::Vector3f fallVec = trans - mFallStartPos;
            f32 fallDistance = fallVec.dot(gravity);
            mFallDistance = fallDistance < 0.0f ? 0.0f : fallDistance;
        }
    }
}

void PlayerColliderHakoniwa::changeCollisionNormal() {
    mCollider->setCollisionShapeKeeper(mCollisionNormal);
    mCollider->setCollisionPartsFilter(nullptr);
    mCollider->setDuringRecovery(false);
    mCollider->offCutCollideAffectDir();
    mCollider->setWallBorderCheckTypeNoFace();
    mCeilingCheck->setupCeilingCheckNormal();
    mCeilingCheck->setCollisionPartsFilter(nullptr);
}

void PlayerColliderHakoniwa::changeCollisionMini() {
    mCollider->setCollisionShapeKeeper(mCollisionMini);
    mCollider->setCollisionPartsFilter(nullptr);
    mCollider->setDuringRecovery(false);
    mCollider->offCutCollideAffectDir();
    mCollider->setWallBorderCheckTypeNoFace();
    mCeilingCheck->setupCeilingCheckNormal();
    mCeilingCheck->setCollisionPartsFilter(nullptr);
}

void PlayerColliderHakoniwa::changeCollisionSwim() {
    mCollider->setCollisionShapeKeeper(mCollisionSwim);
    mCollider->setCollisionPartsFilter(nullptr);
    mCollider->setDuringRecovery(false);
    mCollider->offCutCollideAffectDir();
    mCollider->setWallBorderCheckTypeNoFace();
    mCeilingCheck->setupCeilingCheckNormal();
    mCeilingCheck->setCollisionPartsFilter(nullptr);
}

void PlayerColliderHakoniwa::changeCollisionWallGrab(const sead::Vector3f& cutDir) {
    mCollider->setCollisionShapeKeeper(mCollisionWallGrab);
    mCollider->setCollisionPartsFilter(nullptr);
    mCollider->setDuringRecovery(false);
    mCollider->onCutCollideAffectDir(cutDir);
    mCollider->setWallBorderCheckTypeAll();
    mCeilingCheck->setupCeilingCheckGrab();
    mCeilingCheck->setCollisionPartsFilter(nullptr);
}

void PlayerColliderHakoniwa::changeCollisionGrabCeil(const sead::Vector3f& cutDir) {
    mCollider->setCollisionShapeKeeper(mCollisionGrabCeil);
    mCollider->setCollisionPartsFilter(nullptr);
    mCollider->setDuringRecovery(true);
    mCollider->onCutCollideAffectDir(cutDir);
    mCollider->setWallBorderCheckTypeAll();
    mCeilingCheck->setupCeilingCheckGrab();
    mCeilingCheck->setCollisionPartsFilter(nullptr);
}

void PlayerColliderHakoniwa::changeCollisionPoleClimb(const sead::Vector3f& cutDir) {
    mCollider->setCollisionShapeKeeper(mCollisionPoleClimb);
    mCollider->setCollisionPartsFilter(nullptr);
    mCollider->setDuringRecovery(false);
    mCollider->onCutCollideAffectDir(cutDir);
    mCollider->setWallBorderCheckTypeAll();
    mCeilingCheck->setupCeilingCheckNormal();
    mCeilingCheck->setCollisionPartsFilter(nullptr);
}

void PlayerColliderHakoniwa::changeCollision2DNormal() {
    mCollider->setCollisionShapeKeeper(mCollision2DNormal);
    mCollider->setCollisionPartsFilter(mCollisionFilter2D);
    mCollider->setDuringRecovery(false);
    mCollider->offCutCollideAffectDir();
    mCollider->setWallBorderCheckTypeNoFace();
    mCeilingCheck->setupCeilingCheckNormal();
    mCeilingCheck->setCollisionPartsFilter(mCollisionFilter2D);
}

void PlayerColliderHakoniwa::changeCollision2DMini() {
    mCollider->setCollisionShapeKeeper(mCollision2DMini);
    mCollider->setCollisionPartsFilter(mCollisionFilter2D);
    mCollider->setDuringRecovery(false);
    mCollider->offCutCollideAffectDir();
    mCollider->setWallBorderCheckTypeNoFace();
    mCeilingCheck->setupCeilingCheckNormal();
    mCeilingCheck->setCollisionPartsFilter(mCollisionFilter2D);
}

void PlayerColliderHakoniwa::changeCollisionHack(const al::CollisionPartsFilterBase* filter) {
    mCollider->setCollisionShapeKeeper(mCollisionNormal);
    mCollider->setCollisionPartsFilter(nullptr);
    mCollider->setDuringRecovery(false);
    mCollider->offCutCollideAffectDir();
    mCollider->setWallBorderCheckTypeNoFace();
    mCeilingCheck->setupCeilingCheckGrab();
    mCeilingCheck->setCollisionPartsFilter(filter);
}

void PlayerColliderHakoniwa::changeCollisionRecovery(bool is2D) {
    if (is2D) {
        mCollider->setCollisionShapeKeeper(mCollision2DNormal);
        mCollider->setCollisionPartsFilter(mCollisionFilter2D);
        mCollider->setDuringRecovery(false);
        mCollider->offCutCollideAffectDir();
        mCollider->setWallBorderCheckTypeNoFace();
        mCeilingCheck->setupCeilingCheckNormal();
        mCeilingCheck->setCollisionPartsFilter(mCollisionFilter2D);
    } else {
        mCollider->setCollisionShapeKeeper(mCollisionNormal);
        mCollider->setCollisionPartsFilter(nullptr);
        mCollider->setDuringRecovery(false);
        mCollider->offCutCollideAffectDir();
        mCollider->setWallBorderCheckTypeNoFace();
        mCeilingCheck->setupCeilingCheckNormal();
        mCeilingCheck->setCollisionPartsFilter(nullptr);
    }
    mCollider->setDuringRecovery(true);
}

void PlayerColliderHakoniwa::changeCollisionAbyss(const sead::Vector3f& cutDir) {
    mCollider->setCollisionShapeKeeper(mCollisionNormal);
    mCollider->setCollisionPartsFilter(nullptr);
    mCollider->setDuringRecovery(false);
    mCollider->onCutCollideAffectDir(cutDir);
    mCollider->setWallBorderCheckTypeNoFace();
    mCeilingCheck->setupCeilingCheckNormal();
    mCeilingCheck->setCollisionPartsFilter(nullptr);
}

void PlayerColliderHakoniwa::prepareCollisionEndHack() {
    mCollider->setCollisionPartsFilter(nullptr);
    mCeilingCheck->setCollisionPartsFilter(nullptr);
    rs::resetCollisionExpandCheck(this);
}

void PlayerColliderHakoniwa::prepareCollisionEndRecovery() {
    mCollider->setDuringRecovery(false);
    rs::resetCollisionExpandCheck(this);
}

f32 PlayerColliderHakoniwa::getColliderRadius() const {
    f32 radius = 0.0f;
    mCollider->calcBoundingRadius(&radius);
    return radius;
}

f32 PlayerColliderHakoniwa::getColliderDiskHalfHeight() const {
    return 5.0f;
}

void PlayerColliderHakoniwa::calcBoundingCenter(sead::Vector3f* center) const {
    mCollider->calcBoundingCenter(center);
}

void PlayerColliderHakoniwa::calcSeparateCapLocalOffset(sead::Vector3f* offset) const {
    CollisionShapeKeeper* shape = mCollider->getCollisionShapeKeeper();
    if (shape == mCollisionPoleClimb) {
        *offset = sead::Vector3f::ez * -50.0f;
    } else if (shape == mCollisionWallGrab) {
        const sead::Vector3f vertical = sead::Vector3f::ey * 140.0f;
        const sead::Vector3f depth = sead::Vector3f::ez * -50.0f;
        offset->setSub(depth, vertical);
    } else if (shape == mCollisionGrabCeil) {
        *offset = sead::Vector3f::ey * -115.0f;
    }
}

void PlayerColliderHakoniwa::validateGroundSupport() {
    mCollider->setValidGroundSupport(true);
}

void PlayerColliderHakoniwa::invalidateGroundSupport() {
    mCollider->setValidGroundSupport(false);
}

PlayerCollider* PlayerColliderHakoniwa::getPlayerCollider() const {
    return mCollider;
}

bool PlayerColliderHakoniwa::isEnableStandUp() const {
    return mCeilingCheck->isEnableStandUp();
}

bool PlayerColliderHakoniwa::isEnableHoldUp() const {
    return mCeilingCheck->isEnableHoldUp();
}

bool PlayerColliderHakoniwa::isPressedCeil() const {
    return mCeilingCheck->isPressedCeil();
}

f32 PlayerColliderHakoniwa::getSafetyCeilSpace() const {
    return mCeilingCheck->getSafetyCeilSpace();
}

f32 PlayerColliderHakoniwa::getCeilCheckHeight() const {
    return mCeilingCheck->getCeilCheckHeight();
}

bool PlayerColliderHakoniwa::isAboveGround() const {
    return mIsAboveGround;
}

f32 PlayerColliderHakoniwa::getGroundHeight() const {
    return mGroundHeight;
}

f32 PlayerColliderHakoniwa::getShadowDropHeight() const {
    return mShadowDropHeight;
}

f32 PlayerColliderHakoniwa::getFallDistance() const {
    return mFallDistance;
}
