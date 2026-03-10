#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

class HomeCapMovePoint {
public:
    const char* name;
    sead::Vector3f position;
    sead::Quatf direction;
};
