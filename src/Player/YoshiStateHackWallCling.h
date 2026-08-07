#pragma once

#include <math/seadVector.h>

namespace al {
class CollisionParts;
}

class YoshiStateHackWallCling {
public:
    void setup(const al::CollisionParts* collisionParts, const sead::Vector3f& position,
               const sead::Vector3f& normal);
};
