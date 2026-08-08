#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Joint/JointControllerBase.h"

namespace al {
class JointLookAtInfo;

class JointLookAtController : public JointControllerBase {
public:
    JointLookAtController(s32 jointCount, const sead::Matrix34f* baseMtx);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f* jointMtx) override;
    void appendJoint(JointLookAtInfo* info);
    void requestJointLookAt(const sead::Vector3f& target);
    void invalidJoint(s32 jointIndex);
    void validAllJoint();
    const char* getCtrlTypeName() const override;

    bool get_52() const { return _52; }
    void set_52(bool value) { _52 = value; }
    void updateRequestState() {
        _51 = _50;
        _50 = false;
    }

private:
    sead::Vector3f mTarget;
    u32 _34;
    const sead::Matrix34f* mBaseMtx;
    s32 mJointCount;
    s32 _44;
    JointLookAtInfo** mJointInfos;
    bool _50;
    bool _51;
    bool _52;
    bool _53;
    bool _54;
    u8 _55[3];
};

static_assert(sizeof(JointLookAtController) == 0x58);
}  // namespace al
