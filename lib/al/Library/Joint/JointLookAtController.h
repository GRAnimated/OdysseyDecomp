#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include "Library/Joint/JointControllerBase.h"

namespace al {
class JointLookAtInfo;

class JointLookAtController : public JointControllerBase {
public:
    JointLookAtController(s32, const sead::Matrix34f*);

    void calcJointCallback(s32, sead::Matrix34f*) override;
    void appendJoint(al::JointLookAtInfo*);
    void requestJointLookAt(const sead::Vector3f&);
    bool invalidJoint(s32);
    void validAllJoint();
    const char* getCtrlTypeName() const override;

    void set_50_51() {
        bool orig = _50;
        _50 = false;
        _51 = orig;
    }

    void set_52(bool flag) { _52 = flag; }

private:
    void* filler[4];
    bool _50;
    bool _51;
    bool _52;
};

struct JointLookAtParam {
    JointLookAtParam(s32, f32, const sead::Vector2f&, const sead::Vector2f&, const sead::Vector3f&,
                     const sead::Vector3f&);
    JointLookAtParam();
};

}  // namespace al
