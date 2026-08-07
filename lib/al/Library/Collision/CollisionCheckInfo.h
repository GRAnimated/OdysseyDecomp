#pragma once

#include <math/seadVector.h>

namespace al {
class CollisionPartsFilterBase;
class TriangleFilterBase;

class CollisionCheckInfoBase {
public:
    const sead::Vector3f* position;
    const CollisionPartsFilterBase* collisionPartsFilter;
    const TriangleFilterBase* triangleFilter;
};

class SphereCheckInfo : public CollisionCheckInfoBase {
public:
    SphereCheckInfo(const sead::Vector3f* position, f32 radius,
                    const CollisionPartsFilterBase* collisionPartsFilter = nullptr,
                    const TriangleFilterBase* triangleFilter = nullptr)
        : CollisionCheckInfoBase{position, collisionPartsFilter, triangleFilter}, radius(radius) {}

    f32 radius;
};

static_assert(sizeof(CollisionCheckInfoBase) == 0x18);
static_assert(sizeof(SphereCheckInfo) == 0x20);
}  // namespace al
