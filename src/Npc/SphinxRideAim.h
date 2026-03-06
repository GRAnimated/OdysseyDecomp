#pragma once

#include <math/seadVector.h>

namespace al {
class JointAimInfo;
}

class SphinxRide;

class SphinxRideAim {
public:
    SphinxRideAim(SphinxRide* sphinxRide);

    void updateLook();
    void updateTarget();
    void updateLookPos(f32 rate);
    void updateAimInfo();
    void updateReset();

private:
    SphinxRide* mSphinxRide;
    al::JointAimInfo* mHeadAimInfo = nullptr;
    al::JointAimInfo* mNeckAimInfo = nullptr;
    al::JointAimInfo* mSpineAimInfo = nullptr;
    sead::Vector3f mLookPos = sead::Vector3f::zero;
    sead::Vector3f mTargetPos = sead::Vector3f::zero;
};

static_assert(sizeof(SphinxRideAim) == 0x38);
