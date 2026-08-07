#include "Player/PlayerCollisionCheckSphereMove.h"

#include "Library/Collision/CollisionPartsKeeperUtil.h"

u32 PlayerCollisionCheckSphereMove::getNum() const {
    return mNum;
}

al::CollisionDirector* PlayerCollisionCheckSphereMove::getCollisionDirector() const {
    return mCollisionDirector;
}


f32 PlayerCollisionCheckSphereMove::getTValue(u32 index) const {
    return mTValues[mIndices[index]];
}

const sead::Vector3f& PlayerCollisionCheckSphereMove::getPos(u32 index) const {
    return mPositions[mIndices[index]];
}

const al::CollisionParts* PlayerCollisionCheckSphereMove::getCollisionParts(u32 index) const {
    return mTriangles[mIndices[index]].getCollisionParts();
}

const char* PlayerCollisionCheckSphereMove::getMapCodeName(u32 index) const {
    return al::getFloorCodeName(mTriangles[mIndices[index]]);
}

const char* PlayerCollisionCheckSphereMove::getWallCodeName(u32 index) const {
    return al::getWallCodeName(mTriangles[mIndices[index]]);
}

const char* PlayerCollisionCheckSphereMove::getMaterialCodeName(u32 index) const {
    return al::getMaterialCodeName(mTriangles[mIndices[index]]);
}
