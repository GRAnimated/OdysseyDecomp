#pragma once

#include <math/seadVector.h>

namespace al {
class ByamlIter;
}

class TalkNpcEyeLineAnimParam {
public:
    TalkNpcEyeLineAnimParam();

    void init(const al::ByamlIter& iter);
    const char* getEyeMoveAnimName();
    const char* getEyeResetAnimName();

    bool mIsValid;
    const char* mBaseJointName;
    f32 mMinDegreeH;
    f32 mMaxDegreeH;
    f32 mMinDegreeV;
    f32 mMaxDegreeV;
    sead::Vector3f mLocalAxisFront;
    f32 mOffsetY;
};

static_assert(sizeof(TalkNpcEyeLineAnimParam) == 0x30);
