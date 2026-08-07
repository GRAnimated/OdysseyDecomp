#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class HitSensor;
}

class PlayerEyeSensorHitHolder;
class YoshiTongue;

class YoshiStateHackPlay {
public:
    bool attackSensor(al::HitSensor* self, al::HitSensor* other);
    void updatePrevMovement();
    void updateAfterMovement();
    void prepareEndHack();
    bool isActiveHeadCorrection() const;
    bool isCollisionShapeTongueJump() const;
    bool tryGetLookAtTonguePos(sead::Vector3f* position) const;
    bool tryCalcTonguePullForce(f32* force, sead::Vector3f* direction) const;
    void calcGroundPoseRate(f32* frontRate, f32* sideRate) const;
    void startFruitShineGetDemo();

private:
    unsigned char _0[0x58];
    YoshiTongue* mTongue;
    unsigned char _60[0x10];
    PlayerEyeSensorHitHolder* mEyeSensorHitHolder;
};
