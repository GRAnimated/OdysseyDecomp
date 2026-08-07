#include "Player/YoshiTongueTipConnector.h"

#include "Library/Collision/Collider.h"
#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/PartsConnectorUtil.h"
#include "Library/Math/MatrixUtil.h"

YoshiTongueTipConnector::YoshiTongueTipConnector(const al::LiveActor* actor)
    : mCollisionParts(nullptr), mMtxConnector(al::createMtxConnector(actor)),
      mBaseMtx(sead::Matrix34f::ident), mIsGroundAttached(false) {}

void YoshiTongueTipConnector::reset() {
    mCollisionParts = nullptr;
    mBaseMtx.makeIdentity();
    al::disconnectMtxConnector(mMtxConnector);
    mIsGroundAttached = false;
}

void YoshiTongueTipConnector::attachCollision(const al::CollisionParts* collisionParts,
                                               const sead::Vector3f& front,
                                               const sead::Vector3f& up,
                                               const sead::Vector3f& position,
                                               const sead::Vector3f& polygonNormal,
                                               const sead::Vector3f& gravity) {
    mCollisionParts = collisionParts;
    al::attachMtxConnectorToCollisionParts(mMtxConnector, collisionParts);
    al::makeMtxFrontUpPos(&mBaseMtx, front, up, position);
    mIsGroundAttached = al::isFloorPolygon(polygonNormal, gravity);
}

bool YoshiTongueTipConnector::tryCalcConnect(sead::Vector3f* front, sead::Vector3f* up,
                                             sead::Vector3f* position) {
    if (!mCollisionParts)
        return false;
    if (!al::isCollisionValid(mCollisionParts))
        return false;

    sead::Matrix34f connectMtx = sead::Matrix34f::ident;
    al::calcConnectMtx(&connectMtx, mMtxConnector, mBaseMtx);
    connectMtx.getBase(*up, 1);
    connectMtx.getBase(*front, 2);
    connectMtx.getTranslation(*position);
    return true;
}
