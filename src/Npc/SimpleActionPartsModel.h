#pragma once

#include <prim/seadSafeString.h>

#include "Library/Obj/PartsModel.h"

class SimpleActionPartsModel : public al::PartsModel {
public:
    SimpleActionPartsModel(const char* name);

    void control() override;

    bool mIsActionPending = false;
    sead::FixedSafeString<64> mActionName;
    bool mIsBlendWeightDirty = false;
    f32 mBlendWeight0 = 1.0f;
    f32 mBlendWeight1 = 0.0f;
    bool mIsClearInterpole = false;
};
