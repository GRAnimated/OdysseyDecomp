#pragma once

#include <math/seadMatrix.h>

#include "Library/HostIO/HioNode.h"
#include "Library/Joint/IJointController.h"

namespace al {
class JointControllerBase : public HioNode, public IJointController {
public:
    JointControllerBase(s32);

    virtual void appendJointId(s32);
    virtual const char* getCtrlTypeName() const = 0;
    virtual bool tryInvalidateConstraints();
    virtual bool tryValidateConstraints();

    void findNextId(s32*, s32) const;
    bool isExistId(s32) const;

private:
    void* filler[5];
};
}  // namespace al
