#include "Library/Collision/CollisionParts.h"

#include "Library/Collision/KCollisionServer.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/MatrixUtil.h"

namespace al {

void CollisionParts::calcInvMtxScale() {
    calcMtxScale(&mMtxScaleVec, mBaseInvMtx);
    mMtxScale = (mMtxScaleVec.x + mMtxScaleVec.y + mMtxScaleVec.z) / 3.0f;
    mInvMtxScale = 1.0f / mMtxScale;
}

const LiveActor* CollisionParts::getConnectedHost() const {
    return getSensorHost(mConnectedSensor);
}

void CollisionParts::updateBoundingSphereRange(sead::Vector3f scale) {
    updateBoundingSphereRangePrivate((scale.x + scale.y + scale.z) / 3.0f);
}

void CollisionParts::onJoinList() {}

void CollisionParts::updateBoundingSphereRange() {
    sead::Matrix34f mtx;
    mtx = *mSyncCollisionMtx;
    updateBoundingSphereRangePrivate(makeEqualScale(&mtx));
}

void CollisionParts::syncMtx(const sead::Matrix34f& mtx) {
    mSyncMtx = mtx;
    makeEqualScale(&mSyncMtx);
}

void CollisionParts::syncMtx() {
    mSyncMtx = *mSyncCollisionMtx;
    makeEqualScale(&mSyncMtx);
}

void CollisionParts::updateScale() {
    const f32 scale = makeEqualScale(&mSyncMtx);
    _150.set(scale, scale, scale);
    if (!isNearZero(scale - mBaseMtxScale))
        updateBoundingSphereRangePrivate(scale);
}

void CollisionParts::updateBoundingSphereRangePrivate(f32 scale) {
    mBaseMtxScale = scale;
    mBoundingSphereRange = mKCollisionServer->getFarthestVertexDistance() * scale;
}

// NON_MATCHING: target loads all three base-matrix translation components before the position
// pair, while Clang schedules the Z translation load after that pair; all arithmetic and the PL
// comparison match at 76/76. Next source-level hypothesis: find a sead translation/distance idiom
// that keeps the three matrix loads grouped without forcing codegen.
bool CollisionParts::checkBoundingSphereRange(const sead::Vector3f& position, f32 radius) {
    sead::Vector3f center;
    mBaseMtx.getTranslation(center);
    const sead::Vector3f delta = position - center;
    const f32 range = mBoundingSphereRange + radius;
    return !(range * range < delta.squaredLength());
}

void updateCollisionParts(CollisionParts* collisionParts) {
    if (!collisionParts->isValidCollision())
        return;

    collisionParts->updateMtx();
    if (collisionParts->_16a)
        collisionParts->_16a = false;
}

void updateCollisionPartsList(sead::TList<CollisionParts*>* collisionPartsList) {
    for (CollisionParts* collisionParts : *collisionPartsList)
        updateCollisionParts(collisionParts);
}

// NON_MATCHING: target retains a direct call to the empty onJoinList() hook after insertion, while
// Clang eliminates that visible-empty same-TU call, leaving current 152 bytes versus 160. Next
// source-level hypothesis: recover a natural declaration/TU shape that keeps the hook call without
// forcing inline/noinline attributes.
void pushBackCollisionParts(sead::TList<CollisionParts*>* collisionPartsList,
                            CollisionParts* collisionParts) {
    collisionPartsList->pushBack(collisionParts);
    collisionParts->onJoinList();
    if (!collisionParts->isValidCollision())
        collisionPartsList->erase(collisionParts);
}

}  // namespace al
