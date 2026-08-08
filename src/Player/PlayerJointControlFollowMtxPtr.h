#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>

#include "Library/Joint/JointControllerBase.h"

class PlayerJointControlFollowMtxPtr : public al::JointControllerBase {
public:
    PlayerJointControlFollowMtxPtr(const sead::Matrix34f*);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f*) override;
    const char* getCtrlTypeName() const override;

private:
    bool mIsValid;
    u8 mPadding29[7];
    const sead::Matrix34f* mFollowMtx;
};

static_assert(sizeof(PlayerJointControlFollowMtxPtr) == 0x38);
