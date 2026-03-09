#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Joint/JointControllerBase.h"

namespace al {
struct JointLookAtInfo;

class JointLookAtController : public JointControllerBase {
public:
    JointLookAtController(s32 maxJointCount, const sead::Matrix34f* baseMtxPtr);
    void calcJointCallback(s32, sead::Matrix34f*) override;
    const char* getCtrlTypeName() const override;
    void appendJoint(JointLookAtInfo* info);
    void requestJointLookAt(const sead::Vector3f& target);
    void invalidJoint(s32 jointIndex);
    void validAllJoint();

private:
    sead::Vector3f mLookAtTarget = sead::Vector3f::zero;
    const sead::Matrix34f* mBaseMtxPtr = nullptr;
    s32 mMaxJointCount = 0;
    s32 mJointCount = 0;
    JointLookAtInfo** mJoints = nullptr;
    bool mIsRequested = false;
    bool _51 = false;
    bool _52 = false;
    bool mIsValid = true;
    bool _54 = false;
};

}  // namespace al
