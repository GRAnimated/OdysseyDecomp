#include "Player/PlayerCollider.h"

#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/CollisionPartsTriangle.h"
#include "Library/Collision/KCollisionServer.h"
#include "Library/Math/MathUtil.h"

#include "Player/CollisionMultiShape.h"
#include "Player/CollisionShapeKeeper.h"
#include "Player/CollidedShapeResult.h"
#include "Player/CollisionShapeInfo.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
constexpr f32 cInvalidHitDistance = -99999.0f;

void updateDirectionFlags(sead::BitFlag32* flags, const sead::Vector3f& fix,
                          const sead::Vector3f& fallback) {
    sead::Vector3f direction = fix;
    if (al::isNearZero(direction, 0.001f)) {
        direction = fallback;
        if (al::isNearZero(direction.x, 0.05f))
            direction.x = 0.0f;
        if (al::isNearZero(direction.y, 0.05f))
            direction.y = 0.0f;
        if (al::isNearZero(direction.z, 0.05f))
            direction.z = 0.0f;
        al::normalize(&direction);
    } else {
        const f32 length = direction.length();
        if (al::isNearZero(direction.x / length, 0.05f))
            direction.x = 0.0f;
        if (al::isNearZero(direction.y / length, 0.05f))
            direction.y = 0.0f;
        if (al::isNearZero(direction.z / length, 0.05f))
            direction.z = 0.0f;
    }

    if (!al::isNearZero(direction.x, 0.001f)) {
        if (direction.x > 0.0f)
            flags->set(0x80);
        if (direction.x < 0.0f)
            flags->set(0x100);
    }
    if (!al::isNearZero(direction.y, 0.001f)) {
        if (direction.y > 0.0f)
            flags->set(0x200);
        if (direction.y < 0.0f)
            flags->set(0x400);
    }
    if (!al::isNearZero(direction.z, 0.001f)) {
        if (direction.z > 0.0f)
            flags->set(0x800);
        if (direction.z < 0.0f)
            flags->set(0x1000);
    }
}


void includeVectorBounds(sead::Vector3f* minVec, sead::Vector3f* maxVec,
                         const sead::Vector3f& value) {
    minVec->set(sead::Mathf::min(minVec->x, value.x),
                sead::Mathf::min(minVec->y, value.y),
                sead::Mathf::min(minVec->z, value.z));
    maxVec->set(sead::Mathf::max(maxVec->x, value.x),
                sead::Mathf::max(maxVec->y, value.y),
                sead::Mathf::max(maxVec->z, value.z));
}

bool isGroundNormal(const sead::Vector3f& normal, const sead::Vector3f& gravity, f32 angle) {
    if (al::isNearZero(normal, 0.001f))
        return false;
    const f32 dot = normal.dot(gravity);
    return dot < 0.0f && sead::Mathf::abs(dot) >= sead::Mathf::cos(sead::Mathf::deg2rad(angle));
}

s32 classifyHitNormal(const sead::Vector3f& normal, const sead::Vector3f& gravity, f32 angle) {
    if (isGroundNormal(normal, gravity, angle))
        return 0;
    if (!al::isNearZero(normal, 0.001f) &&
        sead::Mathf::abs(normal.dot(gravity)) < sead::Mathf::cos(sead::Mathf::deg2rad(angle)))
        return 1;
    return 2;
}

void accumulateHitFix(sead::BitFlag32* flags, sead::Vector3f* staticMin,
                      sead::Vector3f* staticMax, sead::Vector3f* movingMin,
                      sead::Vector3f* movingMax, sead::Vector3f* fix,
                      const sead::Vector3f& fallback, const al::HitInfo& hitInfo) {
    if (!alCollisionUtil::isCollisionMoving(&hitInfo)) {
        includeVectorBounds(staticMin, staticMax, *fix);
        updateDirectionFlags(flags, *fix, fallback);
        return;
    }

    f32 length = 0.0f;
    sead::Vector3f direction(0.0f, 0.0f, 0.0f);
    if (al::separateScalarAndDirection(&length, &direction, *fix) ||
        direction.dot(hitInfo.collisionMovingReaction) >= 0.0f) {
        includeVectorBounds(movingMin, movingMax, *fix);
        includeVectorBounds(movingMin, movingMax, hitInfo.collisionMovingReaction);
        return;
    }

    const f32 adjustedLength = sead::Mathf::max(direction.dot(hitInfo.collisionMovingReaction) +
                                                    length,
                                                0.0f);
    *fix = direction * adjustedLength;
    includeVectorBounds(movingMin, movingMax, *fix);
}

void collectGroundArrowHit(sead::PtrArray<al::HitInfo>* hitInfos,
                           sead::Buffer<f32>* hitDistances, sead::Buffer<f32>* hitValues,
                           const CollidedShapeResult* result, const sead::Vector3f& gravity,
                           f32 groundAngle) {
    if (!result->isArrow())
        return;

    const al::HitInfo& hitInfo = **result->getArrowHitInfo();
    const sead::Vector3f& normal = hitInfo.triangle.getFaceNormal();
    if (al::isNearZero(normal, 0.001f))
        return;

    const f32 dot = normal.dot(gravity);
    if (dot >= 0.0f || sead::Mathf::abs(dot) < sead::Mathf::cos(sead::Mathf::deg2rad(groundAngle)))
        return;

    const CollisionShapeInfoArrow* shapeInfo = result->getShapeInfoArrow();
    const s32 index = shapeInfo->getIndex();
    if ((*hitDistances)[index] >= hitInfo._70)
        return;

    *(*hitInfos)[index] = hitInfo;
    (*hitDistances)[index] = hitInfo._70;
    (*hitValues)[index] = shapeInfo->getRadius();
}

f32 stabilizeFixAxis(f32 current, f32 previous, u32 positiveBit, u32 negativeBit,
                     u32 previousFlags, u32 currentFlags) {
    if ((previousFlags & negativeBit) && current > 0.0f && current > previous &&
        al::isNearZeroOrGreater(previous, 0.001f)) {
        if ((currentFlags & positiveBit) != 0)
            return (current + previous) * 0.5f;
        return previous;
    }
    if ((previousFlags & positiveBit) && current < 0.0f && current < previous &&
        al::isNearZeroOrLess(previous, 0.001f)) {
        if ((currentFlags & negativeBit) != 0)
            return (current + previous) * 0.5f;
        return previous;
    }
    return current;
}
}  // namespace

// NON_MATCHING: implementation is 76 bytes smaller; recover original container initialization and inlining order.
PlayerCollider::PlayerCollider(al::CollisionDirector* collisionDirector,
                               const sead::Matrix34f* mtx, const sead::Vector3f* trans,
                               const sead::Vector3f* gravity, bool isLargeCollisionBuffer)
    : mCollisionDirector(collisionDirector), mMtxPtr(mtx), mTransPtr(trans),
      mGravityPtr(gravity), mTrans(*trans), mSize(0.0f), mMtx(sead::Matrix34f::ident),
      _68(new al::HitInfo), _70(0.0f), _78(new al::HitInfo), _7c(0.0f),
      _88(new al::HitInfo), _8c(0.0f), mCollidedFixReaction(0.0f, 0.0f, 0.0f), _a0(false),
      _a1(false), mCollisionHitNormal(0.0f, 0.0f, 0.0f),
      mCollisionHitPos(0.0f, 0.0f, 0.0f), mTimeInAir(0),
      mCollidePosMtx(sead::Matrix34f::ident), mCollisionShapeKeeper(nullptr),
      mCollisionShapeScale(1.0f), mCollisionMultiShape(nullptr), _108(0),
      mIsInFastMoveCollisionArea(false), mIsValidGroundSupport(true), mIsDuringRecovery(false),
      mCutCollideAffectDir(0.0f, 0.0f, 0.0f), mWallBorderCheckType(1),
      mCollisionPartsFilter(nullptr), _128(), _158(nullptr), _160(0), _164(0), _168(),
      mHitDistanceArray(), mHitValueArray(), _198(0.0f, 0.0f, 0.0f),
      mCollidedGroundNormal(0.0f, 1.0f, 0.0f), _1b0(70.0f) {
    mCollisionMultiShape = new CollisionMultiShape(this, isLargeCollisionBuffer ? 256 : 128);
    _158 = new al::HitInfo[64];

    for (s32 i = 0; i < 3; i++)
        _128[i].allocBuffer(64, nullptr);

    _168.allocBuffer(3, nullptr);
    mHitDistanceArray.allocBuffer(3, nullptr, 8);
    mHitValueArray.allocBuffer(3, nullptr, 8);

    for (s32 i = 0; i < 3; i++) {
        _168.pushBack(new al::HitInfo);
        mHitDistanceArray[i] = cInvalidHitDistance;
        mHitValueArray[i] = 0.0f;
    }

    clear();
    onInvalidate();
}

void PlayerCollider::onInvalidate() {
    _8c = cInvalidHitDistance;
    mCollidedFixReaction.set(0.0f, 0.0f, 0.0f);
    _70 = cInvalidHitDistance;
    _7c = cInvalidHitDistance;
    mHitDistanceArray[0] = cInvalidHitDistance;
    mHitDistanceArray[1] = cInvalidHitDistance;
    mHitDistanceArray[2] = cInvalidHitDistance;
    mTrans = *mTransPtr;
    mSize = mCollisionShapeKeeper
                ? mCollisionShapeKeeper->getBoundingRadius() * mCollisionShapeScale
                : 0.0f;
    mMtx = *mMtxPtr;
    mTimeInAir = 2;
}

void PlayerCollider::setCollisionShapeKeeper(CollisionShapeKeeper* keeper) {
    if (!mCollisionShapeKeeper) {
        mSize = keeper->getBoundingRadius();
        mCollisionShapeKeeper = keeper;
        return;
    }
    if (mCollisionShapeKeeper != keeper)
        mSize = mCollisionShapeKeeper->getBoundingRadius() * mCollisionShapeScale;
    mCollisionShapeKeeper = keeper;
}

void PlayerCollider::calcBoundingRadius(f32* radius) const {
    *radius = mCollisionShapeKeeper->getBoundingRadius() * mCollisionShapeScale;
}

void PlayerCollider::setCollisionShapeScale(f32 scale) {
    if (mCollisionShapeKeeper) {
        const f32 radius = mCollisionShapeKeeper->getBoundingRadius() * mCollisionShapeScale;
        mSize = sead::Mathf::min(mSize, radius);
    }
    mCollisionShapeScale = scale;
}

void PlayerCollider::onCutCollideAffectDir(const sead::Vector3f& direction) {
    mCutCollideAffectDir = direction;
}

void PlayerCollider::offCutCollideAffectDir() {
    mCutCollideAffectDir.set(0.0f, 0.0f, 0.0f);
}

void PlayerCollider::clear() {
    _8c = cInvalidHitDistance;
    mCollidedFixReaction.set(0.0f, 0.0f, 0.0f);
    _70 = cInvalidHitDistance;
    _7c = cInvalidHitDistance;
    mHitDistanceArray[0] = cInvalidHitDistance;
    mHitDistanceArray[1] = cInvalidHitDistance;
    mHitDistanceArray[2] = cInvalidHitDistance;
}

void PlayerCollider::calcCheckPos(sead::Vector3f* checkPos) const {
    *checkPos = *mTransPtr;
}

void PlayerCollider::resetPose(const sead::Quatf& quat) {
    mMtx.fromQuat(quat);
}

// NON_MATCHING: implementation is 80 bytes smaller; recover final press-state branches and exact temporary lifetime order.
sead::Vector3f PlayerCollider::collide(const sead::Vector3f& move) {
    for (s32 i = 0; i < 3; i++)
        _128[i].clear();
    _160 = 0;
    _108 = 0;
    _a0 = false;
    _a1 = false;
    mCollisionHitNormal.set(0.0f, 0.0f, 0.0f);
    mCollisionHitPos.set(0.0f, 0.0f, 0.0f);

    const sead::Vector3f externalTrans = *mTransPtr;
    sead::Vector3f pos = mTrans;
    f32 size = mSize;
    sead::Quatf quat = sead::Quatf::unit;
    sead::Quatf targetQuat = sead::Quatf::unit;
    mMtx.toQuat(quat);
    mMtxPtr->toQuat(targetQuat);

    const f32 targetSize = mCollisionShapeKeeper->getBoundingRadius() * mCollisionShapeScale;
    sead::Vector3f contactMove(0.0f, 0.0f, 0.0f);
    calcMovePowerByContact(&contactMove, externalTrans);
    clear();

    const f32 firstCheckRange =
        sead::Mathf::min(mCollisionShapeKeeper->getCheckStepRange() * 0.99f, 35.0f);
    const sead::Vector3f contactTarget = externalTrans + contactMove;
    const sead::Vector3f contactDelta = contactTarget - pos;
    bool movedByContact = false;
    if (!al::isNearZero(contactDelta, 0.001f) ||
        !al::isNearZero(size - targetSize, 0.001f)) {
        moveCollide(&pos, &size, &quat, contactTarget, targetSize, targetQuat, contactDelta,
                    firstCheckRange, false);
        movedByContact = true;
    }

    const sead::Vector3f moveTarget = pos + move;
    moveCollide(&pos, &size, &quat, moveTarget, targetSize, targetQuat, move,
                sead::Mathf::min(mCollisionShapeKeeper->getCheckStepRange(), 35.0f),
                movedByContact);

    const sead::Vector3f previousTrans = mTrans;
    mTrans = pos;
    mSize = targetSize;
    mMtx = *mMtxPtr;

    if (_70 >= 0.0f) {
        if (mTimeInAir < 99999)
            mTimeInAir++;
    } else {
        mTimeInAir = 0;
    }

    if (_a0) {
        if (_70 >= 0.0f && rs::isCollisionCodePress(*_68) &&
            al::isNearZeroOrGreater(alCollisionUtil::getCollisionMovingReaction(_68).dot(
                                        alCollisionUtil::getCollisionHitNormal(_68)),
                                    0.001f)) {
            mCollisionHitNormal = mCollidedGroundNormal;
            mCollisionHitPos = _198;
            _a1 = false;
        } else if (_8c >= 0.0f && rs::isCollisionCodePress(*_88) &&
                   al::isNearZeroOrGreater(alCollisionUtil::getCollisionMovingReaction(_88).dot(
                                               alCollisionUtil::getCollisionHitNormal(_88)),
                                           0.001f)) {
            mCollisionHitNormal = alCollisionUtil::getCollisionHitNormal(_88);
            mCollisionHitPos = alCollisionUtil::getCollisionHitPos(_88);
            _a1 = false;
        } else {
            _a0 = false;
        }
    }

    if (_a1) {
        if (_7c >= 0.0f && rs::isCollisionCodePress(*_78) &&
            alCollisionUtil::getCollisionMovingReaction(_78).dot(
                alCollisionUtil::getCollisionHitNormal(_78)) > 0.0f) {
            mCollisionHitNormal = alCollisionUtil::getCollisionHitNormal(_78);
            mCollisionHitPos = alCollisionUtil::getCollisionHitPos(_78);
            _a0 = false;
        } else {
            _a1 = false;
        }
    }

    if (mIsDuringRecovery) {
        mTrans = externalTrans + move;
        mSize = targetSize;
        mMtx = *mMtxPtr;
        return mCollidedFixReaction;
    }

    if (!al::isNearZero(mCutCollideAffectDir, 0.001f)) {
        sead::Vector3f actualMove = pos - previousTrans;
        al::verticalizeVec(&actualMove, mCutCollideAffectDir, actualMove);
        actualMove += mCutCollideAffectDir * mCutCollideAffectDir.dot(move);
        mTrans = externalTrans + actualMove;
    }

    mCollidedFixReaction = pos - externalTrans - contactMove - move;
    return mCollidedFixReaction;
}

bool PlayerCollider::calcMovePowerByContact(sead::Vector3f* movePower,
                                            const sead::Vector3f& contactPos) {
    if (_70 < 0.0f || !alCollisionUtil::isCollisionMoving(_68))
        return false;

    al::Triangle triangle = _68->triangle;
    triangle.calcForceMovePower(movePower, contactPos);
    const sead::Vector3f& faceNormal = triangle.getFaceNormal();
    if (faceNormal.dot(*movePower) > 0.0f)
        al::verticalizeVec(movePower, faceNormal, *movePower);
    return true;
}

// NON_MATCHING: implementation is 124 bytes smaller; recover remaining retry and interpolation edge handling.
void PlayerCollider::moveCollide(sead::Vector3f* pos, f32* size, sead::Quatf* quat,
                                 const sead::Vector3f& targetPos, f32 targetSize,
                                 const sead::Quatf& targetQuat, const sead::Vector3f& moveVec,
                                 f32 checkStepRange, bool skipFirstStep) {
    al::SpherePoseInterpolator interpolator{};
    interpolator.startInterp(*pos, targetPos, *size, targetSize, *quat, targetQuat, checkStepRange);
    if (skipFirstStep)
        interpolator.nextStep();

    sead::Vector3f accumulatedFix(0.0f, 0.0f, 0.0f);
    if (!findCollidePos(&interpolator)) {
        interpolator.calcInterp(pos, size, quat, nullptr);
        return;
    }

    s32 retryCount = 0;
    sead::Vector3f previousFixDir(0.0f, 0.0f, 0.0f);
    while (true) {
        sead::Vector3f remainMove(0.0f, 0.0f, 0.0f);
        interpolator.calcInterp(pos, size, quat, &remainMove);

        sead::Vector3f fixVec(0.0f, 0.0f, 0.0f);
        sead::Vector3f collideVec(0.0f, 0.0f, 0.0f);
        calcResultVec(&fixVec, &collideVec, accumulatedFix);
        *pos += fixVec;

        if (interpolator.isCurrentStepEnd())
            break;

        sead::Vector3f fixDir(0.0f, 0.0f, 0.0f);
        if (!al::isNearZero(fixVec, 0.0000001f)) {
            const f32 fixLength = fixVec.length();
            if (fixLength > 0.0f) {
                fixDir = fixVec / fixLength;
                const f32 dot = fixDir.dot(remainMove);
                if (dot >= 0.0f)
                    remainMove -= fixDir * sead::Mathf::min(dot, fixLength);
                else
                    remainMove -= fixDir * dot;
            }
        }

        const f32 moveDot = moveVec.dot(remainMove);
        if (moveDot < 0.0f && !al::isNearZero(moveDot, 0.001f))
            break;
        if (al::isNearZero(remainMove, 0.001f) && previousFixDir.dot(fixDir) < 0.0f)
            break;

        const sead::Vector3f startPos = *pos - fixVec;
        const sead::Vector3f endPos = *pos + remainMove;
        interpolator.startInterp(startPos, endPos, *size, targetSize, *quat, targetQuat,
                                 checkStepRange);
        interpolator.nextStep();

        sead::Vector3f interpPos(0.0f, 0.0f, 0.0f);
        interpolator.calcInterpPos(&interpPos);
        accumulatedFix = startPos - interpPos + collideVec;

        if (mIsInFastMoveCollisionArea) {
            if (((_108 & 0x100) && accumulatedFix.x < 0.0f && collideVec.x < 0.0f) ||
                ((_108 & 0x80) && accumulatedFix.x > 0.0f && collideVec.x > 0.0f))
                accumulatedFix.x = 0.0f;
            if (((_108 & 0x400) && accumulatedFix.y < 0.0f && collideVec.y < 0.0f) ||
                ((_108 & 0x200) && accumulatedFix.y > 0.0f && collideVec.y > 0.0f))
                accumulatedFix.y = 0.0f;
            if (((_108 & 0x1000) && accumulatedFix.z < 0.0f && collideVec.z < 0.0f) ||
                ((_108 & 0x800) && accumulatedFix.z > 0.0f && collideVec.z > 0.0f))
                accumulatedFix.z = 0.0f;
        }

        if (!findCollidePos(&interpolator)) {
            interpolator.calcInterp(pos, size, quat, nullptr);
            break;
        }

        previousFixDir = fixDir;
        if (++retryCount > 100)
            break;
    }
}

bool PlayerCollider::findCollidePos(al::SpherePoseInterpolator* interpolator) {
    while (true) {
        if (interpolator->isInterpolationEnd())
            return false;

        sead::Vector3f pos(0.0f, 0.0f, 0.0f);
        f32 size = 0.0f;
        sead::Quatf quat = sead::Quatf::unit;
        sead::Vector3f remainMove(0.0f, 0.0f, 0.0f);
        interpolator->calcInterp(&pos, &size, &quat, &remainMove);
        const f32 radiusScale = interpolator->calcRadiusBaseScale(size) * mCollisionShapeScale;
        mCollidePosMtx.fromQuat(quat);
        mCollidePosMtx.setTranslation(pos);
        if (mCollisionMultiShape->check(mCollisionShapeKeeper, &mCollidePosMtx, radiusScale,
                                        remainMove, mCollisionPartsFilter))
            return true;
        interpolator->nextStep();
    }
}

// NON_MATCHING: implementation is 1044 bytes smaller; recover omitted press-strike correction and exact result aggregation.
void PlayerCollider::calcResultVec(sead::Vector3f* fixResult,
                                   sead::Vector3f* collideResult,
                                   const sead::Vector3f& previousFix) {
    const s32 previousFlags = _108;
    _108 = 0;

    const s32 resultCount = mCollisionShapeKeeper->getNumCollidedShapeResults();
    for (s32 i = 0; i < resultCount; i++) {
        const CollidedShapeResult* result = mCollisionShapeKeeper->getCollidedShapeResult(i);
        if (result->isArrow()) {
            const al::HitInfo& hitInfo = **result->getArrowHitInfo();
            if (isGroundNormal(hitInfo.triangle.getFaceNormal(), *mGravityPtr, _1b0))
                _108 |= 0x40;
            continue;
        }

        if (result->isSphere()) {
            const al::HitInfo& hitInfo = **result->getSphereHitInfo();
            if (!hitInfo.isCollisionAtFace())
                continue;
            const s32 hitType = classifyHitNormal(hitInfo.triangle.getFaceNormal(), *mGravityPtr,
                                                  _1b0);
            _108 |= hitType == 0 ? 1 : hitType == 1 ? 2 : 4;
            continue;
        }

        if (result->isDisk()) {
            const al::HitInfo& hitInfo = **result->getDiskHitInfo();
            if (!hitInfo.isCollisionAtFace())
                continue;
            const s32 hitType = classifyHitNormal(hitInfo.triangle.getFaceNormal(), *mGravityPtr,
                                                  _1b0);
            _108 |= hitType == 0 ? 8 : hitType == 1 ? 0x10 : 0x20;
        }
    }

    bool hasGroundPos = false;
    bool hasGroundNormal = false;
    calcGroundArrowAverage(&hasGroundPos, &_198, &hasGroundNormal, &mCollidedGroundNormal,
                           mCollisionShapeKeeper);

    sead::Vector3f staticMin(0.0f, 0.0f, 0.0f);
    sead::Vector3f staticMax(0.0f, 0.0f, 0.0f);
    sead::Vector3f movingMin(0.0f, 0.0f, 0.0f);
    sead::Vector3f movingMax(0.0f, 0.0f, 0.0f);
    sead::BitFlag32 flags(_108);
    for (s32 i = 0; i < resultCount; i++) {
        const CollidedShapeResult* result = mCollisionShapeKeeper->getCollidedShapeResult(i);
        if (result->isArrow())
            calcResultVecArrow(&flags, &staticMin, &staticMax, &movingMin, &movingMax, result);
        else if (result->isSphere())
            calcResultVecSphere(&flags, &staticMin, &staticMax, &movingMin, &movingMax, result);
        else if (result->isDisk())
            calcResultVecDisk(&flags, &staticMin, &staticMax, &movingMin, &movingMax, result);
    }
    _108 = flags.getDirect();

    if (_70 >= 0.0f) {
        if (!hasGroundPos)
            _198 = _68->collisionHitPos;
        if (!hasGroundNormal)
            mCollidedGroundNormal = _68->triangle.getFaceNormal();
    }

    const sead::Vector3f staticSum = staticMin + staticMax;
    sead::Vector3f combinedMin;
    combinedMin.set(sead::Mathf::min(staticMin.x, movingMin.x),
                    sead::Mathf::min(staticMin.y, movingMin.y),
                    sead::Mathf::min(staticMin.z, movingMin.z));
    sead::Vector3f combinedMax;
    combinedMax.set(sead::Mathf::max(staticMax.x, movingMax.x),
                    sead::Mathf::max(staticMax.y, movingMax.y),
                    sead::Mathf::max(staticMax.z, movingMax.z));
    sead::Vector3f fix = combinedMin + combinedMax;

    if (fix.x > 0.0f && (_108 & 0x100))
        fix.x = staticSum.x;
    else if (fix.x < 0.0f && (_108 & 0x80))
        fix.x = staticSum.x;
    if (fix.y > 0.0f && (_108 & 0x400))
        fix.y = staticSum.y;
    else if (fix.y < 0.0f && (_108 & 0x200))
        fix.y = staticSum.y;
    if (fix.z > 0.0f && (_108 & 0x1000))
        fix.z = staticSum.z;
    else if (fix.z < 0.0f && (_108 & 0x800))
        fix.z = staticSum.z;

    fix.set(stabilizeFixAxis(fix.x, previousFix.x, 0x80, 0x100, previousFlags, _108),
            stabilizeFixAxis(fix.y, previousFix.y, 0x200, 0x400, previousFlags, _108),
            stabilizeFixAxis(fix.z, previousFix.z, 0x800, 0x1000, previousFlags, _108));

    sead::Vector3f overlap(0.0f, 0.0f, 0.0f);
    const sead::Vector3f range = combinedMax - combinedMin;
    if (!al::isNearZero(range.x, 0.001f) && sead::Mathf::abs(fix.x) < range.x)
        overlap.x = fix.x <= 0.0f ? -(fix.x + range.x) : range.x - fix.x;
    if (!al::isNearZero(range.y, 0.001f) && sead::Mathf::abs(fix.y) < range.y)
        overlap.y = fix.y <= 0.0f ? -(fix.y + range.y) : range.y - fix.y;
    if (!al::isNearZero(range.z, 0.001f) && sead::Mathf::abs(fix.z) < range.z)
        overlap.z = fix.z <= 0.0f ? -(fix.z + range.z) : range.z - fix.z;

    sead::Vector3f parallel(0.0f, 0.0f, 0.0f);
    sead::Vector3f vertical(0.0f, 0.0f, 0.0f);
    if (!al::isNearZero(overlap, 0.001f))
        al::separateVectorParallelVertical(&parallel, &vertical, *mGravityPtr, overlap);
    _a0 = !al::isNearZero(parallel, 0.001f);
    _a1 = !al::isNearZero(vertical, 0.001f);

    *fixResult = fix;
    *collideResult = staticSum;
}

// NON_MATCHING: size and control flow match, but floating accumulator registers differ; recover original accumulator declaration order.
void PlayerCollider::calcGroundArrowAverage(bool* hasGroundPos, sead::Vector3f* groundPos,
                                              bool* hasGroundNormal,
                                              sead::Vector3f* groundNormal,
                                              const CollisionShapeKeeper* shapeKeeper) {
    *hasGroundPos = false;
    *hasGroundNormal = false;
    groundPos->set(0.0f, 0.0f, 0.0f);
    groundNormal->set(0.0f, 0.0f, 0.0f);

    const s32 resultCount = shapeKeeper->getNumCollidedShapeResults();
    if (resultCount >= 1) {
        for (u32 i = 0; i != resultCount; i++)
            collectGroundArrowHit(&_168, &mHitDistanceArray, &mHitValueArray,
                                  shapeKeeper->getCollidedShapeResult(i), *mGravityPtr, _1b0);
    }
    const s32 supportResultCount = shapeKeeper->getNumCollidedShapeSupportResults();
    if (supportResultCount >= 1) {
        for (u32 i = 0; i != supportResultCount; i++)
            collectGroundArrowHit(&_168, &mHitDistanceArray, &mHitValueArray,
                                  shapeKeeper->getCollidedShapeSupportResult(i), *mGravityPtr, _1b0);
    }

    sead::Vector3f posSum(0.0f, 0.0f, 0.0f);
    sead::Vector3f normalSum(0.0f, 0.0f, 0.0f);
    s32 posCount = 0;
    s32 normalCount = 0;
    for (s32 i = 0; i < 3; i++) {
        if (mHitDistanceArray[i] < 0.0f)
            continue;

        if (mHitDistanceArray[i] >= mHitValueArray[i]) {
            posCount++;
            posSum += alCollisionUtil::getCollisionHitPos(_168[i]);
        }

        const sead::Vector3f& normal = alCollisionUtil::getCollisionHitNormal(_168[i]);
        bool isDuplicate = false;
        for (s32 j = 0; j < i; j++) {
            if (mHitDistanceArray[j] >= 0.0f)
                isDuplicate |= al::isNearDirection(
                    normal, alCollisionUtil::getCollisionHitNormal(_168[j]), 0.01f);
        }
        if (!isDuplicate) {
            normalSum += normal;
            normalCount++;
        }
    }

    if (posCount > 0) {
        *groundPos = posSum * (1.0f / posCount);
        *hasGroundPos = true;
    }
    if (normalCount > 0) {
        *groundNormal = normalSum * (1.0f / normalCount);
        al::normalize(groundNormal);
        *hasGroundNormal = true;
    }
}

// NON_MATCHING: implementation is 512 bytes smaller; recover moving-collision and ground-arrow correction paths.
void PlayerCollider::calcResultVecArrow(sead::BitFlag32* flags, sead::Vector3f* staticMin,
                                          sead::Vector3f* staticMax,
                                          sead::Vector3f* movingMin,
                                          sead::Vector3f* movingMax,
                                          const CollidedShapeResult* result) {
    if ((_108 & 0x40) == 0)
        return;

    const al::HitInfo& hitInfo = **result->getArrowHitInfo();
    const sead::Vector3f& normal = hitInfo.triangle.getFaceNormal();
    if (!isGroundNormal(normal, *mGravityPtr, _1b0))
        return;

    const CollisionShapeInfoArrow* shapeInfo = result->getShapeInfoArrow();
    sead::Vector3f arrowDirection = shapeInfo->getArrowWorld();
    al::tryNormalizeOrZero(&arrowDirection);
    const f32 penetration = sead::Mathf::max(hitInfo._70 - shapeInfo->getRadius(), 0.0f);
    sead::Vector3f fix = mCollidedGroundNormal *
                         (-penetration * arrowDirection.dot(mCollidedGroundNormal));
    accumulateHitFix(flags, staticMin, staticMax, movingMin, movingMax, &fix,
                     mCollidedGroundNormal, hitInfo);

    if (_70 < hitInfo._70) {
        *_68 = hitInfo;
        _70 = hitInfo._70;
    }
    collectHitInfoArray(hitInfo, 0);
}

// NON_MATCHING: implementation is 1124 bytes smaller; recover support-ground and hit-normal branch details.
void PlayerCollider::calcResultVecSphere(sead::BitFlag32* flags, sead::Vector3f* staticMin,
                                           sead::Vector3f* staticMax,
                                           sead::Vector3f* movingMin,
                                           sead::Vector3f* movingMax,
                                           const CollidedShapeResult* result) {
    const al::SphereHitInfo& sphereHit = result->getSphereHitInfo();
    const al::HitInfo& hitInfo = **sphereHit;
    const CollisionShapeInfoSphere* shapeInfo = result->getShapeInfoSphere();
    sead::Vector3f normal = hitInfo.triangle.getNormal(0);
    const s32 hitType = classifyHitNormal(normal, *mGravityPtr, _1b0);

    sead::Vector3f fix(0.0f, 0.0f, 0.0f);
    sead::Vector3f fixNormal(0.0f, 0.0f, 0.0f);
    if (hitType == 0) {
        if (!mIsValidGroundSupport || shapeInfo->isIgnoreGround())
            return;

        if (mCollisionShapeKeeper->hasShapeArrow()) {
            if (shapeInfo->isSupportGround()) {
                sead::Vector3f supportDelta = hitInfo.collisionHitPos - hitInfo._80;
                al::verticalizeVec(&supportDelta, shapeInfo->getUpWorld(), supportDelta);
                if (supportDelta.length() < shapeInfo->getSupportGroundRangeWorld())
                    sphereHit.calcFixVectorNormal(&fix, &fixNormal);
                else
                    sphereHit.calcFixVectorNormal(&fix, &fixNormal);
            } else {
                sphereHit.calcFixVectorNormal(&fix, &fixNormal);
            }
        } else if ((_108 & 1) != 0) {
            sphereHit.calcFixVectorNormal(&fix, &fixNormal);
        } else {
            sphereHit.calcFixVector(&fix, &fixNormal);
        }
    } else if (hitType == 1) {
        if (isNeedWallBorderCheck(hitInfo) &&
            rs::calcExistCollisionBorder(this, hitInfo.collisionHitPos, normal))
            return;

        if ((_108 & 2) != 0) {
            sphereHit.calcFixVectorNormal(&fix, &fixNormal);
        } else {
            sphereHit.calcFixVector(&fix, &fixNormal);
            if (!hitInfo.isCollisionAtFace()) {
                al::verticalizeVec(&fix, *mGravityPtr, fix);
                al::tryNormalizeOrZero(&fixNormal, fix);
            }
        }
    } else {
        if (shapeInfo->is69())
            return;
        if ((_108 & 4) != 0)
            sphereHit.calcFixVectorNormal(&fix, &fixNormal);
        else
            sphereHit.calcFixVector(&fix, &fixNormal);
    }

    accumulateHitFix(flags, staticMin, staticMax, movingMin, movingMax, &fix, fixNormal, hitInfo);

    if (hitType == 0) {
        if (_70 < hitInfo._70) {
            *_68 = hitInfo;
            _70 = hitInfo._70;
        }
    } else if (hitType == 1) {
        if (_7c < hitInfo._70) {
            *_78 = hitInfo;
            _7c = hitInfo._70;
        }
    } else if (_8c < hitInfo._70) {
        *_88 = hitInfo;
        _8c = hitInfo._70;
    }
    collectHitInfoArray(hitInfo, hitType);
}

// NON_MATCHING: implementation is 644 bytes smaller; recover support-ground and edge-contact branch details.
void PlayerCollider::calcResultVecDisk(sead::BitFlag32* flags, sead::Vector3f* staticMin,
                                         sead::Vector3f* staticMax,
                                         sead::Vector3f* movingMin,
                                         sead::Vector3f* movingMax,
                                         const CollidedShapeResult* result) {
    const al::DiskHitInfo& diskHit = result->getDiskHitInfo();
    const al::HitInfo& hitInfo = **diskHit;
    const CollisionShapeInfoDisk* shapeInfo = result->getShapeInfoDisk();
    const sead::Vector3f& normal = hitInfo.triangle.getNormal(0);
    const s32 hitType = classifyHitNormal(normal, *mGravityPtr, _1b0);

    sead::Vector3f fix(0.0f, 0.0f, 0.0f);
    sead::Vector3f fixNormal(0.0f, 0.0f, 0.0f);
    if (hitType == 0) {
        if (!mIsValidGroundSupport || shapeInfo->isIgnoreGround())
            return;

        if (!mCollisionShapeKeeper->hasShapeArrow()) {
            if ((_108 & 8) != 0)
                diskHit.calcFixVectorNormal(&fix, &fixNormal);
            else
                diskHit.calcFixVector(&fix, &fixNormal);
        } else if (shapeInfo->isSupportGround()) {
            if ((_108 & 0x40) == 0 && (_108 & 8) == 0) {
                sead::Vector3f supportDelta = hitInfo.collisionHitPos - hitInfo._80;
                al::verticalizeVec(&supportDelta, shapeInfo->getUpWorld(), supportDelta);
                if (supportDelta.length() >= shapeInfo->getSupportGroundRangeWorld())
                    return;
            }
            diskHit.calcFixVectorNormal(&fix, &fixNormal);
        } else {
            diskHit.calcFixVectorNormal(&fix, &fixNormal);
        }
    } else if (hitType == 1) {
        if (isNeedWallBorderCheck(hitInfo) &&
            rs::calcExistCollisionBorder(this, hitInfo.collisionHitPos, normal))
            return;

        if ((_108 & 0x10) != 0) {
            diskHit.calcFixVectorNormal(&fix, &fixNormal);
        } else {
            if (shapeInfo->isSupportGround() && (_108 & 0x40) != 0 &&
                al::isNearZeroOrGreater(mCollisionShapeKeeper->get54() +
                                            mGravityPtr->dot(hitInfo.collisionHitPos -
                                                             mCollidePosMtx.getTranslation()),
                                        0.001f))
                return;
            diskHit.calcFixVectorNormal(&fix, &fixNormal);
            if (!hitInfo.isCollisionAtFace()) {
                al::verticalizeVec(&fix, *mGravityPtr, fix);
                al::tryNormalizeOrZero(&fixNormal, fix);
            }
        }
    } else {
        diskHit.calcFixVectorNormal(&fix, &fixNormal);
    }

    accumulateHitFix(flags, staticMin, staticMax, movingMin, movingMax, &fix, fixNormal, hitInfo);

    if (hitType == 0) {
        if (_70 < hitInfo._70) {
            *_68 = hitInfo;
            _70 = hitInfo._70;
        }
    } else if (hitType == 1) {
        if (_7c < hitInfo._70) {
            *_78 = hitInfo;
            _7c = hitInfo._70;
        }
    } else if (_8c < hitInfo._70) {
        *_88 = hitInfo;
        _8c = hitInfo._70;
    }
    collectHitInfoArray(hitInfo, hitType);
}

// NON_MATCHING: implementation is 4 bytes smaller; recover the original bounds-checked pointer replacement form.
void PlayerCollider::collectHitInfoArray(const al::HitInfo& hitInfo, s32 arrayIndex) {
    sead::PtrArray<al::HitInfo>* hitInfoArray = nullptr;
    if (arrayIndex == 0)
        hitInfoArray = &_128[0];
    else if (arrayIndex == 1)
        hitInfoArray = &_128[1];
    else if (arrayIndex == 2)
        hitInfoArray = &_128[2];
    else
        return;

    s32 replaceIndex = -1;
    bool shouldReplace = false;
    for (s32 i = 0; i < hitInfoArray->size(); i++) {
        if ((*hitInfoArray)[i]->triangle.getCollisionParts() !=
            hitInfo.triangle.getCollisionParts())
            continue;

        replaceIndex = i;
        if (al::isNearZeroOrGreater((*hitInfoArray)[i]->_70 - hitInfo._70, 0.001f))
            return;
        shouldReplace = true;
        break;
    }

    if (_160 >= static_cast<u32>(hitInfoArray->capacity()))
        return;

    _158[_160] = hitInfo;
    if (shouldReplace)
        hitInfoArray->replace(replaceIndex, &_158[_160]);
    else if (!hitInfoArray->isFull())
        hitInfoArray->pushBack(&_158[_160]);
    _160++;
}

bool PlayerCollider::isNeedWallBorderCheck(const al::HitInfo& hitInfo) const {
    bool isNeed;
    if (mWallBorderCheckType != 2) {
        isNeed = false;
        if (mWallBorderCheckType == 1)
            return !hitInfo.isCollisionAtFace();
    } else {
        isNeed = true;
    }
    return isNeed;
}

void PlayerCollider::setWallBorderCheckTypeNone() {
    mWallBorderCheckType = 0;
}

void PlayerCollider::setWallBorderCheckTypeNoFace() {
    mWallBorderCheckType = 1;
}

void PlayerCollider::setWallBorderCheckTypeAll() {
    mWallBorderCheckType = 2;
}

void PlayerCollider::setCollisionPartsFilter(const al::CollisionPartsFilterBase* filter) {
    mCollisionPartsFilter = filter;
}

void PlayerCollider::calcBoundingCenter(sead::Vector3f* center) const {
    center->setRotated(*mMtxPtr,
                       mCollisionShapeKeeper->getBoundingCenter() * mCollisionShapeScale);
    *center += *mTransPtr;
}

void PlayerCollider::validateCorrectMovePartsCheck() {
    mCollisionMultiShape->validateCorrectMovePartsCheck();
}

al::CollisionDirector* PlayerCollider::getCollisionDirector() const {
    return mCollisionDirector;
}
