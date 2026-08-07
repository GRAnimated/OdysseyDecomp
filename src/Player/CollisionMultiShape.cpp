#include "Player/CollisionMultiShape.h"

#include "Library/Collision/CollisionCheckInfo.h"
#include "Library/Collision/CollisionParts.h"
#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/CollisionPartsTriangle.h"
#include "Library/Collision/ICollisionPartsKeeper.h"
#include "Library/Collision/KCollisionServer.h"
#include "Library/Math/MathUtil.h"

#include "Player/CollidedShapeResult.h"
#include "Player/CollisionShapeInfo.h"
#include "Player/CollisionShapeKeeper.h"
#include "Util/CollisionShapeFunction.h"

namespace alKCollisionFunc {
void calcSphereHitPos(sead::Vector3f* hitPos, const al::KCollisionServer* server,
                      const sead::Vector3f& center, const al::KCPrismData& prismData,
                      const al::KCPrismHeader* prismHeader, u8 collisionLocation);
void calcDiskHitPos(sead::Vector3f* hitPos, const al::KCollisionServer* server,
                    const sead::Vector3f& center, f32 radius, const sead::Vector3f& axis,
                    const al::KCPrismData& prismData, const al::KCPrismHeader* prismHeader,
                    u8 collisionLocation);
}  // namespace alKCollisionFunc

namespace {
// NON_MATCHING: exact size, but first mismatch at 0x71003F78C4 swaps the X22/X23 shape/parts allocation; next source-level hypothesis is parameter lifetime/order in the generic branch.
void searchPrism(al::KCollisionServer* server, const CollisionShapeInfoBase* shapeInfo,
                 const al::CollisionParts* collisionParts, const sead::Vector3f& offset,
                 sead::IDelegate2<const al::KCPrismData*, const al::KCPrismHeader*>& callback,
                 const al::KCPrismHeader** prismHeader,
                 sead::PtrArray<const al::KCPrismData>* prismArray) {
    *prismHeader = nullptr;
    prismArray->clear();
    const f32 scale = collisionParts->getMtxScale();

    if (CollisionShapeFunction::isShapeArrow(shapeInfo)) {
        const CollisionShapeInfoArrow* arrow = CollisionShapeFunction::getShapeInfoArrow(shapeInfo);
        sead::Vector3f start = arrow->getStartRelative() + offset;
        server->searchPrismArrow(start, arrow->getArrowRelative(), callback);
        return;
    }

    if (CollisionShapeFunction::isShapeDisk(shapeInfo)) {
        const CollisionShapeInfoDisk* disk = CollisionShapeFunction::getShapeInfoDisk(shapeInfo);
        (void)shapeInfo->getBoundingCenterWorld();
        sead::Vector3f center = disk->getCenterRelative() + offset;
        server->searchPrismDisk(center, disk->getAxisRelative(),
                                scale * disk->getHalfHeightWorld(),
                                scale * disk->getRadiusWorld(), callback);
        return;
    }

    if (CollisionShapeFunction::isShapeSphere(shapeInfo))
        shapeInfo = CollisionShapeFunction::getShapeInfoSphere(shapeInfo);

    sead::Vector3f center =
        collisionParts->getBaseInvMtx() * shapeInfo->getBoundingCenterWorld() + offset;
    server->searchPrism(&center, scale * shapeInfo->getBoundingRadiusWorld(), callback);
}

bool isBackFace(const al::CollisionParts* parts, const al::HitInfo& hitInfo,
                const sead::Vector3f& move) {
    if (parts->get_15c() != 0 && !parts->isMoving())
        return false;
    if (al::isNearZero(move, 0.001f))
        return false;
    return hitInfo.triangle.getFaceNormal().dot(move) > 0.0f;
}

void setMovingReaction(al::HitInfo* hitInfo, const al::CollisionParts* parts,
                       const sead::Vector3f& localReaction) {
    if (parts->get_15c() != 0 && !parts->isMoving())
        return;
    hitInfo->collisionMovingReaction.setRotated(parts->getBaseMtx(), localReaction);
    al::parallelizeVec(&hitInfo->collisionMovingReaction, hitInfo->triangle.getFaceNormal(),
                       hitInfo->collisionMovingReaction);
}
}  // namespace

CollisionMultiShape::CollisionMultiShape(const al::IUseCollision* useCollision, s32 maxParts)
    : mUseCollision(useCollision), mCheckPos(0.0f, 0.0f, 0.0f), mShapeKeeper(nullptr),
      mVelocity(0.0f, 0.0f, 0.0f), _2c(0.0f, 0.0f, 0.0f), _38(0), _3c(1.0f), _40(0.0f, 0.0f, 0.0f),
      mCollisionParts(nullptr), mKCollisionServer(nullptr), mKCPrismHeader(nullptr),
      mKCPrismDataArray(), _78(true) {
    mKCPrismDataArray.allocBuffer(maxParts, nullptr);
}

bool CollisionMultiShape::check(
    CollisionShapeKeeper* shapeKeeper, const sead::Matrix34f* matrix, f32 scale,
    const sead::Vector3f& velocity,
    const al::CollisionPartsFilterBase* collisionPartsFilter) {
    mShapeKeeper = shapeKeeper;
    mShapeKeeper->clearResult();
    mShapeKeeper->calcWorldShapeInfo(*matrix, scale);
    mVelocity = velocity;

    al::SphereCheckInfo checkInfo(&mCheckPos, mShapeKeeper->mBoundingRadius);
    if (collisionPartsFilter)
        checkInfo.collisionPartsFilter = collisionPartsFilter;
    mCheckPos.setMul(*matrix, mShapeKeeper->mBoundingCenter);
    sead::Delegate1<CollisionMultiShape, al::CollisionParts*> callback(
        this, &CollisionMultiShape::callbackFromParts);
    alCollisionUtil::getCollisionPartsKeeper(mUseCollision)->searchWithSphere(checkInfo, callback);
    return mShapeKeeper->mNumCollideResult > 0;
}

// NON_MATCHING: exact size, but disk culling first differs at 0x71003F6A90 in center/axis load scheduling; next source-level hypothesis is the disk vector-expression evaluation order.
void CollisionMultiShape::callbackFromParts(al::CollisionParts* collisionParts) {
    mCollisionParts = collisionParts;
    _3c = collisionParts->getMtxScale();
    mKCollisionServer = collisionParts->getKCollisionServer();
    mShapeKeeper->calcRelativeShapeInfo(collisionParts->getBaseInvMtx());

    const s32 shapeCount = mShapeKeeper->mCollisionShape.size();
    if (shapeCount < 1)
        return;

    for (s32 i = 0; i < shapeCount; ++i) {
        _40.set(0.0f, 0.0f, 0.0f);
        _38 = i;
        const CollisionShapeInfoBase* shapeInfo = mShapeKeeper->getShapeInfoBase(i);

        bool interpolateParts;
        if (collisionParts->get_15c() != 0)
            interpolateParts = collisionParts->isMoving();
        else
            interpolateParts = true;

        bool isFarAway;
        if (CollisionShapeFunction::isShapeArrow(shapeInfo)) {
            sead::Vector3f partsCenter;
            partsCenter.set(collisionParts->getBaseMtx().m[0][3],
                            collisionParts->getBaseMtx().m[1][3],
                            collisionParts->getBaseMtx().m[2][3]);
            const f32 partsRadius = collisionParts->getBoundingSphereRange();
            const CollisionShapeInfoArrow* arrow =
                CollisionShapeFunction::getShapeInfoArrow(shapeInfo);
            if (al::isNearCollideSphereAabb(partsCenter, partsRadius, arrow->getWorldAabb())) {
                isFarAway = !al::checkHitSegmentSphere(partsCenter, arrow->getStartWorld(),
                                                       arrow->getEndWorld(), partsRadius, nullptr,
                                                       nullptr);
            } else {
                isFarAway = true;
            }
        } else if (CollisionShapeFunction::isShapeDisk(shapeInfo)) {
            sead::Vector3f partsCenter;
            partsCenter.set(collisionParts->getBaseMtx().m[0][3],
                            collisionParts->getBaseMtx().m[1][3],
                            collisionParts->getBaseMtx().m[2][3]);
            const f32 partsRadius = collisionParts->getBoundingSphereRange();
            const CollisionShapeInfoDisk* disk = CollisionShapeFunction::getShapeInfoDisk(shapeInfo);
            sead::Vector3f centerToParts = disk->getCenterWorld() - partsCenter;
            const f32 axialRange = partsRadius + disk->getHalfHeightWorld();
            const f32 axisDistance = disk->getAxisWorld().dot(centerToParts);
            const f32 minusAxisDistance = -axisDistance;
            const f32 absAxisDistance = axisDistance <= 0.0f ? minusAxisDistance : axisDistance;
            if (absAxisDistance > axialRange) {
                isFarAway = true;
            } else {
                centerToParts += disk->getAxisWorld() * minusAxisDistance;
                isFarAway = centerToParts.length() > partsRadius + disk->getRadiusWorld();
            }
        } else {
            isFarAway = alCollisionUtil::isFarAway(
                *collisionParts, shapeInfo->getBoundingCenterWorld(),
                shapeInfo->getBoundingRadiusWorld());
        }

        if (interpolateParts) {
            if (isFarAway)
                continue;

            sead::Vector3f posPrev(0.0f, 0.0f, 0.0f);
            posPrev.setMul(collisionParts->getPrevBaseInvMtx(),
                           shapeInfo->getBoundingCenterWorld());
            sead::Vector3f posCurrent(0.0f, 0.0f, 0.0f);
            posCurrent.setMul(collisionParts->getBaseInvMtx(),
                              shapeInfo->getBoundingCenterWorld());

            sead::Vector3f relativeMove = posCurrent - posPrev;
            _2c.setRotated(collisionParts->getBaseMtx(), relativeMove);
            _2c += mVelocity;

            sead::Delegate2<CollisionMultiShape, const al::KCPrismData*,
                            const al::KCPrismHeader*>
                callback(this, &CollisionMultiShape::callbackFromServer);
            const f32 step = shapeInfo->getCheckStepRangeWorld() * 0.99f;
            const f32 stepRange = 35.0f < step ? 35.0f : step;
            al::SphereInterpolator interpolator;
            interpolator.startInterp(posPrev, posCurrent, shapeInfo->getBoundingRadiusWorld(),
                                     shapeInfo->getBoundingRadiusWorld(), stepRange);

            while (interpolator.getCurrentStep() != 1.0f ||
                   interpolator.getPrevStep() != 1.0f) {
                const s32 resultCount = mShapeKeeper->mNumCollideResult;
                sead::Vector3f searchPos;
                sead::Vector3f remainMove;
                f32 searchRadius;
                interpolator.calcInterp(&searchPos, &searchRadius, &remainMove);
                _40.set(-remainMove.x, -remainMove.y, -remainMove.z);
                if (interpolator.getCurrentStep() >= 1.0f)
                    _2c.set(0.0f, 0.0f, 0.0f);

                sead::Vector3f offset(0.0f, 0.0f, 0.0f);
                offset.setSub(searchPos, posCurrent);
                searchPrism(mKCollisionServer, shapeInfo, collisionParts, offset, callback,
                            &mKCPrismHeader, &mKCPrismDataArray);
                if (_78 && resultCount < mShapeKeeper->mNumCollideResult)
                    break;
                interpolator.nextStep();
            }
        } else if (!isFarAway) {
            sead::Delegate2<CollisionMultiShape, const al::KCPrismData*,
                            const al::KCPrismHeader*>
                callback(this, &CollisionMultiShape::callbackFromServer);
            searchPrism(mKCollisionServer, shapeInfo, collisionParts, sead::Vector3f::zero,
                        callback, &mKCPrismHeader, &mKCPrismDataArray);
        }
    }
}

// NON_MATCHING: target is 0x9B0 bytes and current output is 0x9B4; the arrow locals now follow target order, but start X/Y still emit separate stores instead of the target STP; next source-level hypothesis is the original start-vector construction idiom.
void CollisionMultiShape::callbackFromServer(const al::KCPrismData* prismData,
                                              const al::KCPrismHeader* prismHeader) {
    if (mKCPrismHeader == prismHeader) {
        if (mKCPrismDataArray.indexOf(prismData) >= 0)
            return;
    } else {
        mKCPrismHeader = prismHeader;
        mKCPrismDataArray.clear();
    }
    if (!mKCPrismDataArray.isFull())
        mKCPrismDataArray.pushBack(prismData);

    if (!mShapeKeeper->isShapeArrow(_38)) {
        if (mShapeKeeper->isShapeSphere(_38)) {
            al::SphereHitInfo sphereHitInfo;
            al::HitInfo* hitInfo = *sphereHitInfo;
            hitInfo->triangle.fillData(*mCollisionParts, prismData, prismHeader);
            if (isBackFace(mCollisionParts, *hitInfo, _2c))
                return;

            const CollisionShapeInfoSphere* shape = mShapeKeeper->getShapeInfoSphere(_38);
            const sead::Vector3f center = shape->getCenterRelative() + _40;
            const sead::Vector3f centerForHit = center;
            const f32 radius = shape->getBoundingRadiusWorld() * _3c;
            const bool isScaleNearZero = al::isNearZero(_3c, 0.001f);
            f32 hitDepth = 0.0f;
            u8 collisionLocation = 0;
            if (!mKCollisionServer->KCHitSphereForPlayer(prismData, prismHeader, &centerForHit,
                                                         radius, _3c, &hitDepth,
                                                         &collisionLocation))
                return;

            sead::Vector3f hitPosLocal;
            alKCollisionFunc::calcSphereHitPos(&hitPosLocal, mKCollisionServer, center, *prismData,
                                               prismHeader, collisionLocation);
            hitInfo->_70 = isScaleNearZero ? 0.0f : hitDepth / _3c;
            hitInfo->collisionHitPos.setMul(mCollisionParts->getBaseMtx(), hitPosLocal);
            hitInfo->_80.setMul(mCollisionParts->getBaseMtx(), center);
            hitInfo->collisionLocation = static_cast<al::CollisionLocation>(collisionLocation);
            setMovingReaction(hitInfo, mCollisionParts, _40);

            CollidedShapeResult result(shape);
            result.setSphereHitInfo(sphereHitInfo);
            if (!mShapeKeeper->isCollidedResultFull())
                mShapeKeeper->registerCollideResult(result);
            return;
        }

        if (!mShapeKeeper->isShapeDisk(_38))
            return;

        al::DiskHitInfo diskHitInfo;
        al::HitInfo* hitInfo = *diskHitInfo;
        hitInfo->triangle.fillData(*mCollisionParts, prismData, prismHeader);
        if (isBackFace(mCollisionParts, *hitInfo, _2c))
            return;

        const CollisionShapeInfoDisk* shape = mShapeKeeper->getShapeInfoDisk(_38);
        const sead::Vector3f center = shape->getCenterRelative() + _40;
        const f32 radius = shape->getRadiusWorld() * _3c;
        const f32 halfHeight = shape->getHalfHeightWorld() * _3c;
        const bool isScaleNearZero = al::isNearZero(_3c, 0.001f);
        f32 hitDepth = 0.0f;
        u8 collisionLocation = 0;
        if (!mKCollisionServer->KCHitDisk(prismData, prismHeader, &center, radius, _3c,
                                          halfHeight, shape->getAxisRelative(), &hitDepth,
                                          &collisionLocation))
            return;

        sead::Vector3f hitPosLocal;
        alKCollisionFunc::calcDiskHitPos(&hitPosLocal, mKCollisionServer,
                                         shape->getCenterRelative(), shape->getRadiusWorld(),
                                         shape->getAxisRelative(), *prismData, prismHeader,
                                         collisionLocation);
        hitInfo->_70 = isScaleNearZero ? 0.0f : hitDepth / _3c;
        hitInfo->collisionHitPos.setMul(mCollisionParts->getBaseMtx(), hitPosLocal);
        hitInfo->_80.setMul(mCollisionParts->getBaseMtx(), center);
        hitInfo->collisionLocation = static_cast<al::CollisionLocation>(collisionLocation);
        setMovingReaction(hitInfo, mCollisionParts, _40);

        CollidedShapeResult result(shape);
        result.setDiskHitInfo(diskHitInfo);
        if (!mShapeKeeper->isCollidedResultFull())
            mShapeKeeper->registerCollideResult(result);
        return;
    }

    const CollisionShapeInfoArrow* shape = mShapeKeeper->getShapeInfoArrow(_38);
    al::ArrowHitInfo arrowHitInfo;
    al::HitInfo* hitInfo = *arrowHitInfo;
    hitInfo->triangle.fillData(*mCollisionParts, prismData, prismHeader);
    if (isBackFace(mCollisionParts, *hitInfo, _2c))
        return;

    f32 hitDistance = 0.0f;
    u8 collisionLocation = 0;
    const sead::Vector3f start = shape->getStartRelative() + _40;
    if (!mKCollisionServer->KCHitArrow(prismData, prismHeader, start,
                                       shape->getArrowRelative(), &hitDistance,
                                       &collisionLocation))
        return;

    sead::Vector3f hitPosLocal = start + shape->getArrowRelative() * hitDistance;
    hitInfo->_70 = (1.0f - hitDistance) * shape->getArrowWorld().length();
    hitInfo->collisionHitPos.setMul(mCollisionParts->getBaseMtx(), hitPosLocal);
    hitInfo->collisionLocation = static_cast<al::CollisionLocation>(collisionLocation);
    setMovingReaction(hitInfo, mCollisionParts, _40);

    CollidedShapeResult result(shape);
    result.setArrowHitInfo(arrowHitInfo);
    if (hitInfo->_70 < shape->getRadius()) {
        if (!mShapeKeeper->isCollidedSupportResultFull())
            mShapeKeeper->registerCollideSupportResult(result);
    } else if (!mShapeKeeper->isCollidedResultFull()) {
        mShapeKeeper->registerCollideResult(result);
    }
}
