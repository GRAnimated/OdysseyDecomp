#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class HitSensor;
}

class YoshiStateHackPlay {
public:
    bool attackSensor(al::HitSensor* self, al::HitSensor* other);
    void updatePrevMovement();
    void updateAfterMovement();
    bool isActiveHeadCorrection() const;
    bool isCollisionShapeTongueJump() const;
    bool tryGetLookAtTonguePos(sead::Vector3f* position) const;
    bool tryCalcTonguePullForce(f32* force, sead::Vector3f* direction) const;
    void calcGroundPoseRate(f32* frontRate, f32* sideRate) const;
    void startFruitShineGetDemo();
};
