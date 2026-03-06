#pragma once

namespace al {
struct JointLookAtInfo;

class JointLookAtController {
public:
    void appendJoint(JointLookAtInfo* info);
};
}  // namespace al
