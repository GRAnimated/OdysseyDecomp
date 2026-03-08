#pragma once

#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

class RadiconCar;

class RadiconCarStateAutoMove : public al::HostStateBase<RadiconCar> {
public:
    RadiconCarStateAutoMove(RadiconCar* car);

    void appear() override;
    void calcAccel(sead::Vector3f* accel) const;

    void exeWait();
    void exeMove();
};
