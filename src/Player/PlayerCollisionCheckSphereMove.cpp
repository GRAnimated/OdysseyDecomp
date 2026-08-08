#include "Player/PlayerCollisionCheckSphereMove.h"

#include <prim/seadDelegate.h>

#include "Library/Collision/CollisionPartsKeeperUtil.h"

PlayerCollisionCheckSphereMove::PlayerCollisionCheckSphereMove(
    al::CollisionDirector* collisionDirector, s32 maxNum)
    : mCollisionDirector(collisionDirector), mMaxNum(maxNum),
      mTValues(new f32[maxNum]), mPositions(new sead::Vector3f[maxNum]),
      mTriangles(new al::Triangle[maxNum]),
      mHitInfos(new alCollisionUtil::SphereMoveHitInfo[maxNum]),
      mIndices(new u32[maxNum]), mCollisionParts() {
    mCollisionParts.allocBuffer(100, nullptr);
}


bool PlayerCollisionCheckSphereMove::checkSphereMove(const sead::Vector3f& start,
                                                       const sead::Vector3f& move, f32 radius) {
    mCollisionParts.clear();
    sead::Delegate1<PlayerCollisionCheckSphereMove, al::CollisionParts*> callback(
        this, &PlayerCollisionCheckSphereMove::gatherCollisionParts);

    alCollisionUtil::searchCollisionParts(this, start + move * 0.5f, move.length() * 0.5f + radius, callback,
                                          mCollisionPartsFilter);

    sead::PtrArray<al::CollisionParts>* collisionParts =
        alCollisionUtil::getCollisionPartsPtrArray(this);
    alCollisionUtil::validateCollisionPartsPtrArray(this, &mCollisionParts);
    mNum = alCollisionUtil::checkStrikeSphereMove(this, mHitInfos, mMaxNum, start, radius, move,
                                                   mCollisionPartsFilter, nullptr);
    if (collisionParts)
        alCollisionUtil::validateCollisionPartsPtrArray(this, collisionParts);
    else
        alCollisionUtil::invalidateCollisionPartsPtrArray(this);

    for (u32 i = 0; i < mNum; i++) {
        mTValues[i] = mHitInfos[i]._0;
        mPositions[i].set(mHitInfos[i]._8);
        mTriangles[i] = mHitInfos[i].triangle;
        mIndices[i] = i;
    }

    if (mNum >= 2) {
        for (u32 i = 0; i < mNum - 1; i++) {
            u32 minIndex = i;
            for (u32 j = i + 1; j < mNum; j++) {
                if (mTValues[mIndices[minIndex]] > mTValues[mIndices[j]])
                    minIndex = j;
            }
            if (minIndex != i) {
                u32 tmp = mIndices[i];
                mIndices[i] = mIndices[minIndex];
                mIndices[minIndex] = tmp;
            }
        }
    }
    return mNum != 0;
}

void PlayerCollisionCheckSphereMove::gatherCollisionParts(al::CollisionParts* collisionParts) {
    mCollisionParts.pushBack(collisionParts);
}

u32 PlayerCollisionCheckSphereMove::getNum() const {
    return mNum;
}

f32 PlayerCollisionCheckSphereMove::getTValue(u32 index) const {
    return mTValues[mIndices[index]];
}

const sead::Vector3f& PlayerCollisionCheckSphereMove::getPos(u32 index) const {
    return mPositions[mIndices[index]];
}

const sead::Vector3f& PlayerCollisionCheckSphereMove::getNormal(u32 index) const {
    return mTriangles[mIndices[index]].getNormal(0);
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

al::CollisionDirector* PlayerCollisionCheckSphereMove::getCollisionDirector() const {
    return mCollisionDirector;
}
