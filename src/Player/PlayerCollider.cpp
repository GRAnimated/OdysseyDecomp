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

// NON_MATCHING: target is 632 bytes while current is 628 with the exact 7/7 semantic call sequence; next source-level hypothesis is recovering the target conditional/FP lifetime shape around the ground-normal test.
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
    const f32 absDot = dot <= 0.0f ? -dot : dot;
    if (!(dot < 0.0f && absDot >= sead::Mathf::cos(sead::Mathf::deg2rad(groundAngle))))
        return;

    const CollisionShapeInfoArrow* shapeInfo = result->getShapeInfoArrow();
    const s32 index = shapeInfo->getIndex();
    if ((*hitDistances)[index] >= hitInfo._70)
        return;

    *(*hitInfos)[index] = hitInfo;
    (*hitDistances)[index] = hitInfo._70;
    (*hitValues)[index] = result->getShapeInfoArrow()->getRadius();
}

}  // namespace

// NON_MATCHING: target is 1776 bytes while current is 1700; next source-level hypothesis is recovering the original PtrArray/buffer initialization expression and constructor temporary order.
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

// NON_MATCHING: target is 1748 bytes while current is 1668; next source-level hypothesis is recovering the final press-state branches and exact temporary lifetime order.
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

// NON_MATCHING: target is 1276 bytes while current is 1152 with all 15 semantic calls present;
// target-faithful unordered floating-point continuation predicates are restored, but loop/block placement keeps
// the retry startInterp/nextStep/calcInterpPos calls emitted ahead of the main calcInterp/calcResultVec block.
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
                if (dot >= 0.0f) {
                    const f32 removeLength = dot < fixLength ? dot : fixLength;
                    remainMove -= fixDir * removeLength;
                } else {
                    remainMove -= fixDir * dot;
                }
            }
        }

        const f32 moveDot = moveVec.dot(remainMove);
        if (!(moveDot >= 0.0f || al::isNearZero(moveDot, 0.001f)))
            break;
        if (al::isNearZero(remainMove, 0.001f) && !(previousFixDir.dot(fixDir) >= 0.0f))
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

// NON_MATCHING: target is 3252 bytes while current is 3268 with an exact 57/57 semantic call
// sequence after restoring press-code contact selection, strike-clearance correction, cumulative
// _a0/_a1 collision bits, and the target-inline per-axis stabilization logic. Remaining mismatch is
// broad stack/register lifetime (current 0x1A0 frame versus target 0x190).
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

    if (previousFlags & 0x100) {
        if (fix.x > 0.0f && fix.x > previousFix.x &&
            (previousFix.x >= 0.0f || al::isNearZero(previousFix.x, 0.001f))) {
            fix.x = (_108 & 0x80) ? (fix.x + previousFix.x) * 0.5f : previousFix.x;
        }
    } else if ((previousFlags & 0x80) && fix.x < 0.0f && fix.x < previousFix.x &&
               (previousFix.x <= 0.0f || al::isNearZero(previousFix.x, 0.001f))) {
        fix.x = (_108 & 0x100) ? (fix.x + previousFix.x) * 0.5f : previousFix.x;
    }

    if (previousFlags & 0x400) {
        if (fix.y > 0.0f && fix.y > previousFix.y &&
            (previousFix.y >= 0.0f || al::isNearZero(previousFix.y, 0.001f))) {
            fix.y = (_108 & 0x200) ? (fix.y + previousFix.y) * 0.5f : previousFix.y;
        }
    } else if ((previousFlags & 0x200) && fix.y < 0.0f && fix.y < previousFix.y &&
               (previousFix.y <= 0.0f || al::isNearZero(previousFix.y, 0.001f))) {
        fix.y = (_108 & 0x400) ? (fix.y + previousFix.y) * 0.5f : previousFix.y;
    }

    if (previousFlags & 0x1000) {
        if (fix.z > 0.0f && fix.z > previousFix.z &&
            (previousFix.z >= 0.0f || al::isNearZero(previousFix.z, 0.001f))) {
            fix.z = (_108 & 0x800) ? (fix.z + previousFix.z) * 0.5f : previousFix.z;
        }
    } else if ((previousFlags & 0x800) && fix.z < 0.0f && fix.z < previousFix.z &&
               (previousFix.z <= 0.0f || al::isNearZero(previousFix.z, 0.001f))) {
        fix.z = (_108 & 0x1000) ? (fix.z + previousFix.z) * 0.5f : previousFix.z;
    }

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
    bool addParallelCollision = false;
    bool addVerticalCollision = false;
    if (!al::isNearZero(overlap, 0.001f)) {
        al::separateVectorParallelVertical(&parallel, &vertical, *mGravityPtr, overlap);
        addParallelCollision = !al::isNearZero(parallel, 0.001f);
        const bool isVerticalZero = al::isNearZero(vertical, 0.001f);
        addVerticalCollision = !isVerticalZero;

        if (!isVerticalZero || addParallelCollision) {
            sead::Vector3f pressPos(0.0f, 0.0f, 0.0f);
            sead::Vector3f pressNormal(0.0f, 0.0f, 0.0f);
            bool hasPressContact = false;
            bool canCorrect = !isVerticalZero;

            if (addParallelCollision || isVerticalZero) {
                if (_70 >= 0.0f && rs::isCollisionCodePress(*_68)) {
                    pressPos = _198;
                    pressNormal = mCollidedGroundNormal;
                    hasPressContact = true;
                    canCorrect = true;
                } else if (_8c >= 0.0f && rs::isCollisionCodePress(*_88)) {
                    pressPos = alCollisionUtil::getCollisionHitPos(_88);
                    pressNormal = alCollisionUtil::getCollisionHitNormal(_88);
                    hasPressContact = true;
                    canCorrect = true;
                }
            }

            bool shouldCorrect = false;
            if (!isVerticalZero) {
                if (_7c >= 0.0f && rs::isCollisionCodePress(*_78)) {
                    if (!hasPressContact || vertical.length() > parallel.length()) {
                        pressPos = alCollisionUtil::getCollisionHitPos(_78);
                        pressNormal = alCollisionUtil::getCollisionHitNormal(_78);
                    }
                    shouldCorrect = canCorrect;
                } else {
                    shouldCorrect = canCorrect && hasPressContact;
                }
            } else {
                shouldCorrect = canCorrect;
            }

            if (shouldCorrect) {
                sead::Vector3f localCenter = mCollisionShapeKeeper->getBoundingCenter();
                localCenter.y += 35.0f;
                const sead::Vector3f checkPos = mCollidePosMtx * localCenter;
                sead::Vector3f projected = checkPos - pressPos;
                al::verticalizeVec(&projected, pressNormal, projected);

                sead::Vector3f pushDir(0.0f, 0.0f, 0.0f);
                if (al::tryNormalizeOrZero(&pushDir, projected)) {
                    const f32 pushRemain = 50.0f - projected.length();
                    const f32 pushDistance = pushRemain < 0.0f ? 0.0f : pushRemain;
                    const sead::Vector3f strikeOrigin =
                        checkPos + pushDir * pushDistance + pressNormal;
                    const sead::Vector3f strikeArrow = pressNormal * -200.0f;
                    if (!alCollisionUtil::checkStrikeArrow(this, strikeOrigin, strikeArrow, nullptr,
                                                           nullptr)) {
                        fix += pushDir * (pushDistance * 0.05f);
                        addParallelCollision = false;
                        addVerticalCollision = false;
                    }
                }
            }
        }
    }

    _a0 |= addParallelCollision;
    _a1 |= addVerticalCollision;
    *fixResult = fix;
    *collideResult = staticSum;
}

// NON_MATCHING: target/current are both 776 bytes, but floating accumulator registers differ; next source-level hypothesis is restoring the original position/normal accumulator declaration and update order.
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
        for (s32 i = 0; i != resultCount; i++)
            collectGroundArrowHit(&_168, &mHitDistanceArray, &mHitValueArray,
                                  shapeKeeper->getCollidedShapeResult(i), *mGravityPtr, _1b0);
    }
    const s32 supportResultCount = shapeKeeper->getNumCollidedShapeSupportResults();
    if (supportResultCount >= 1) {
        for (s32 i = 0; i != supportResultCount; i++)
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

// NON_MATCHING: target is 1220 bytes while current is 1180 with the exact 10/10 semantic call sequence after removing the non-target accumulateHitFix abstraction; next source-level hypothesis is recovering target stack/vector lifetimes around fix calculation.
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
    const f32 penetrationValue = hitInfo._70 - shapeInfo->getRadius();
    const f32 penetration = penetrationValue < 0.0f ? 0.0f : penetrationValue;
    sead::Vector3f fix = mCollidedGroundNormal *
                         (-penetration * arrowDirection.dot(mCollidedGroundNormal));
    if (!alCollisionUtil::isCollisionMoving(&hitInfo)) {
        includeVectorBounds(staticMin, staticMax, fix);
        updateDirectionFlags(flags, fix, mCollidedGroundNormal);
    } else {
        f32 length = 0.0f;
        sead::Vector3f direction(0.0f, 0.0f, 0.0f);
        if (al::separateScalarAndDirection(&length, &direction, fix) ||
            direction.dot(hitInfo.collisionMovingReaction) >= 0.0f) {
            includeVectorBounds(movingMin, movingMax, fix);
            includeVectorBounds(movingMin, movingMax, hitInfo.collisionMovingReaction);
        } else {
            const f32 adjustedLengthValue = direction.dot(hitInfo.collisionMovingReaction) + length;
            const f32 adjustedLength = adjustedLengthValue < 0.0f ? 0.0f : adjustedLengthValue;
            fix = direction * adjustedLength;
            includeVectorBounds(movingMin, movingMax, fix);
        }
    }

    if (_70 < hitInfo._70) {
        *_68 = hitInfo;
        _70 = hitInfo._70;
    }
    collectHitInfoArray(hitInfo, 0);
}

// NON_MATCHING: target is 2960 bytes while current is 2896 with the exact 36/36 semantic call sequence after recovering ground-edge correction, branch-local shape queries, repeated hit classification, and direct updateDirectionFlags factoring; next source-level hypothesis is matching remaining support-ground/frame lifetimes.
void PlayerCollider::calcResultVecSphere(sead::BitFlag32* flags, sead::Vector3f* staticMin,
                                         sead::Vector3f* staticMax,
                                         sead::Vector3f* movingMin,
                                         sead::Vector3f* movingMax,
                                         const CollidedShapeResult* result) {
    const al::SphereHitInfo& sphereHit = result->getSphereHitInfo();
    const al::HitInfo& hitInfo = **sphereHit;
    sead::Vector3f normal = hitInfo.triangle.getNormal(0);

    sead::Vector3f fix(0.0f, 0.0f, 0.0f);
    sead::Vector3f fixNormal(0.0f, 0.0f, 0.0f);
    bool skipHitInfo = false;
    if (!isGroundNormal(normal, *mGravityPtr, _1b0)) {
        const f32 normalDot = normal.dot(*mGravityPtr);
        const bool isWall = !al::isNearZero(normal, 0.001f) &&
                            sead::Mathf::abs(normalDot) <
                                sead::Mathf::cos(sead::Mathf::deg2rad(_1b0));
        if (isWall) {
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
            if (result->getShapeInfoSphere()->is69())
                return;
            if ((_108 & 4) != 0)
                sphereHit.calcFixVectorNormal(&fix, &fixNormal);
            else
                sphereHit.calcFixVector(&fix, &fixNormal);
        }
    } else {
        if (!mIsValidGroundSupport || result->getShapeInfoSphere()->isIgnoreGround())
            return;

        if (!mCollisionShapeKeeper->hasShapeArrow()) {
            if ((_108 & 1) != 0)
                sphereHit.calcFixVectorNormal(&fix, &fixNormal);
            else
                sphereHit.calcFixVector(&fix, &fixNormal);
        } else if (!result->getShapeInfoSphere()->isSupportGround()) {
            sphereHit.calcFixVectorNormal(&fix, &fixNormal);
            skipHitInfo = true;
        } else {
            const sead::Vector3f& supportUp = result->getShapeInfoSphere()->getUpWorld();
            const f32 supportRange =
                result->getShapeInfoSphere()->getSupportGroundRangeWorld();
            sead::Vector3f supportDelta = hitInfo.collisionHitPos - hitInfo._80;
            al::verticalizeVec(&supportDelta, supportUp, supportDelta);
            if (supportDelta.length() < supportRange) {
                sphereHit.calcFixVectorNormal(&fix, &fixNormal);
            } else if ((_108 & 0x40) == 0) {
                sphereHit.calcFixVectorNormal(&fix, &fixNormal);
                skipHitInfo = true;
            } else {
                const bool isGroundHeightValid = al::isNearZeroOrGreater(
                    mCollisionShapeKeeper->get58() +
                        mGravityPtr->dot(hitInfo.collisionHitPos - _198),
                    0.001f);
                sphereHit.calcFixVectorNormal(&fix, &fixNormal);
                skipHitInfo = !isGroundHeightValid;
            }
        }

        if (skipHitInfo) {
            if ((_108 & 0x40) != 0 &&
                (hitInfo.collisionHitPos - _198).dot(mCollidedGroundNormal) < 2.5f)
                return;

            sead::Vector3f edgeDirection = hitInfo._80 - hitInfo.collisionHitPos;
            if (!al::tryNormalizeOrZero(&edgeDirection))
                return;

            fix = edgeDirection * fix.dot(edgeDirection);
            al::verticalizeVec(&fix, *mGravityPtr, fix);
            if (!al::tryNormalizeOrZero(&fixNormal, fix))
                return;

            normal = fixNormal;
        }
    }

    if (!alCollisionUtil::isCollisionMoving(&hitInfo)) {
        includeVectorBounds(staticMin, staticMax, fix);
        updateDirectionFlags(flags, fix, normal);
    } else {
        f32 length = 0.0f;
        sead::Vector3f direction(0.0f, 0.0f, 0.0f);
        if (al::separateScalarAndDirection(&length, &direction, fix) ||
            direction.dot(hitInfo.collisionMovingReaction) >= 0.0f) {
            includeVectorBounds(movingMin, movingMax, fix);
            includeVectorBounds(movingMin, movingMax, hitInfo.collisionMovingReaction);
        } else {
            const f32 adjustedLengthValue = direction.dot(hitInfo.collisionMovingReaction) + length;
            const f32 adjustedLength = adjustedLengthValue < 0.0f ? 0.0f : adjustedLengthValue;
            fix = direction * adjustedLength;
            includeVectorBounds(movingMin, movingMax, fix);
        }
    }

    if (skipHitInfo)
        return;

    const s32 resultHitType = classifyHitNormal(normal, *mGravityPtr, _1b0);
    if (resultHitType == 0) {
        if (_70 < hitInfo._70) {
            *_68 = hitInfo;
            _70 = hitInfo._70;
        }
    } else if (resultHitType == 1) {
        if (_7c < hitInfo._70) {
            *_78 = hitInfo;
            _7c = hitInfo._70;
        }
    } else if (_8c < hitInfo._70) {
        *_88 = hitInfo;
        _8c = hitInfo._70;
    }
    collectHitInfoArray(hitInfo, resultHitType);
}

// NON_MATCHING: target and current are both 2552 bytes with the exact 31/31 semantic call sequence after recovering target branch structure, support-ground face rejection, getter lifetimes, repeated hit classification, and direct updateDirectionFlags factoring; next source-level hypothesis is reducing remaining frame/register-allocation differences.
void PlayerCollider::calcResultVecDisk(sead::BitFlag32* flags, sead::Vector3f* staticMin,
                                       sead::Vector3f* staticMax,
                                       sead::Vector3f* movingMin,
                                       sead::Vector3f* movingMax,
                                       const CollidedShapeResult* result) {
    const al::DiskHitInfo& diskHit = result->getDiskHitInfo();
    const al::HitInfo& hitInfo = **diskHit;
    const sead::Vector3f& normal = hitInfo.triangle.getNormal(0);

    sead::Vector3f fix(0.0f, 0.0f, 0.0f);
    sead::Vector3f fixNormal(0.0f, 0.0f, 0.0f);
    if (!isGroundNormal(normal, *mGravityPtr, _1b0)) {
        const f32 normalDot = normal.dot(*mGravityPtr);
        const bool isWall = !al::isNearZero(normal, 0.001f) &&
                            sead::Mathf::abs(normalDot) <
                                sead::Mathf::cos(sead::Mathf::deg2rad(_1b0));
        if (isWall) {
            if (isNeedWallBorderCheck(hitInfo) &&
                rs::calcExistCollisionBorder(this, hitInfo.collisionHitPos, normal))
                return;

            if ((_108 & 0x10) == 0) {
                if (result->getShapeInfoDisk()->isSupportGround() && (_108 & 0x40) != 0 &&
                    al::isNearZeroOrGreater(
                        mCollisionShapeKeeper->get54() +
                            mGravityPtr->dot(hitInfo.collisionHitPos -
                                             mCollidePosMtx.getTranslation()),
                        0.001f))
                    return;

                diskHit.calcFixVectorNormal(&fix, &fixNormal);
                if (!hitInfo.isCollisionAtFace()) {
                    al::verticalizeVec(&fix, *mGravityPtr, fix);
                    al::tryNormalizeOrZero(&fixNormal, fix);
                }
            } else {
                diskHit.calcFixVectorNormal(&fix, &fixNormal);
            }
        } else {
            diskHit.calcFixVectorNormal(&fix, &fixNormal);
        }
    } else {
        if (!mIsValidGroundSupport || result->getShapeInfoDisk()->isIgnoreGround())
            return;

        if (!mCollisionShapeKeeper->hasShapeArrow()) {
            if ((_108 & 8) != 0)
                diskHit.calcFixVectorNormal(&fix, &fixNormal);
            else
                diskHit.calcFixVector(&fix, &fixNormal);
        } else {
            if (!result->getShapeInfoDisk()->isSupportGround() || (_108 & 0x40) != 0)
                return;

            if ((_108 & 8) != 0) {
                if (!hitInfo.isCollisionAtFace())
                    return;
            } else {
                const sead::Vector3f& supportUp = result->getShapeInfoDisk()->getUpWorld();
                const f32 supportRange =
                    result->getShapeInfoDisk()->getSupportGroundRangeWorld();
                sead::Vector3f supportDelta = hitInfo.collisionHitPos - hitInfo._80;
                al::verticalizeVec(&supportDelta, supportUp, supportDelta);
                if (supportDelta.length() >= supportRange)
                    return;
            }
            diskHit.calcFixVectorNormal(&fix, &fixNormal);
        }
    }

    if (!alCollisionUtil::isCollisionMoving(&hitInfo)) {
        includeVectorBounds(staticMin, staticMax, fix);
        updateDirectionFlags(flags, fix, fixNormal);
    } else {
        f32 length = 0.0f;
        sead::Vector3f direction(0.0f, 0.0f, 0.0f);
        if (al::separateScalarAndDirection(&length, &direction, fix) ||
            direction.dot(hitInfo.collisionMovingReaction) >= 0.0f) {
            includeVectorBounds(movingMin, movingMax, fix);
            includeVectorBounds(movingMin, movingMax, hitInfo.collisionMovingReaction);
        } else {
            const f32 adjustedLengthValue = direction.dot(hitInfo.collisionMovingReaction) + length;
            const f32 adjustedLength = adjustedLengthValue < 0.0f ? 0.0f : adjustedLengthValue;
            fix = direction * adjustedLength;
            includeVectorBounds(movingMin, movingMax, fix);
        }
    }

    const s32 resultHitType = classifyHitNormal(normal, *mGravityPtr, _1b0);
    if (resultHitType == 0) {
        if (_70 < hitInfo._70) {
            *_68 = hitInfo;
            _70 = hitInfo._70;
        }
    } else if (resultHitType == 1) {
        if (_7c < hitInfo._70) {
            *_78 = hitInfo;
            _7c = hitInfo._70;
        }
    } else if (_8c < hitInfo._70) {
        *_88 = hitInfo;
        _8c = hitInfo._70;
    }
    collectHitInfoArray(hitInfo, resultHitType);
}

// NON_MATCHING: target is 616 bytes while current is 612; next source-level hypothesis is recovering the original bounds-checked PtrArray selection/replacement expression.
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
