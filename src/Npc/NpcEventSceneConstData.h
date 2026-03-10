#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

class NpcEventSceneConstData {
public:
    NpcEventSceneConstData();

    sead::FixedSafeString<32> mBalloonLayoutName;
    sead::FixedSafeString<32> mTalkLayoutName;
    sead::FixedSafeString<32> mTalkNameLayoutName;
    const char* mBalloonName;
    const char* mBalloonLayoutType;
    const char* mTalkTextPaneName;
    const char* mDefaultName;
    f32 _c8;
    f32 _cc;
    void* _d0;
    f32 _d8;
    s32 _dc;
    s32 _e0;
    bool _e4;
};

static_assert(sizeof(NpcEventSceneConstData) == 0xE8);
