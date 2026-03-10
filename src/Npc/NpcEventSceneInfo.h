#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

class NpcEventTalkInfo;

class NpcEventSceneInfo {
public:
    void* _00 = nullptr;
    NpcEventTalkInfo* _08 = nullptr;
    bool _10 = false;
    bool _11 = false;
    bool _12 = true;
    bool _13 = false;
    bool _14 = false;
    s32 _18 = -1;
    sead::Vector3f _1c = sead::Vector3f::zero;
    sead::Vector3f _28 = sead::Vector3f::zero;
    s32 _34 = 0;
};

static_assert(sizeof(NpcEventSceneInfo) == 0x38);
