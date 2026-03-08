#pragma once

#include <math/seadQuat.h>

#include "Library/Joint/JointControllerBase.h"

namespace al {

class JointQuatController : public JointControllerBase {
public:
    const sead::Quatf* mQuat;
    bool mIsControl;
};

}  // namespace al
