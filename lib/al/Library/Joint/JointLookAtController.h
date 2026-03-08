#pragma once

#include <math/seadVector.h>

namespace al {
struct JointLookAtInfo;

class JointLookAtController {
public:
    void appendJoint(JointLookAtInfo* info);
    void requestJointLookAt(const sead::Vector3f& target);
};
}  // namespace al
