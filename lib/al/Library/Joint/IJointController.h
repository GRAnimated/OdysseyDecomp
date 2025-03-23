#pragma once

#include <math/seadMatrix.h>

class IJointController {
    virtual void calcJointCallback(s32, sead::Matrix34f*) = 0;
};
