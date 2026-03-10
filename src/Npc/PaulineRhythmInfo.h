#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}

struct RhythmEntry {
    f32 beat = -1.0f;
    s32 id = -1;
};

class PaulineAtCeremony;

class PaulineRhythmInfo {
public:
    PaulineRhythmInfo(al::LiveActor* actor, const u8* danceData, const u8* lipData,
                      const u8* faceData, const u8* eyelineData, const u8* reSingData);

    void initAnimInfo(const u8* danceData, const u8* lipData, const u8* faceData,
                      const u8* eyelineData, const u8* reSingData);
    void update(bool forceReset);
    bool isLooping();
    void resetRhythmInfo(f32 beat);
    void updateDance();
    void updateReSing();
    void updateLipSync();
    void updateFace();
    void updateEyeline();
    s32 getDanceAnimId(s32 index);
    f32 getDanceBeat(s32 index);

private:
    friend class PaulineAtCeremony;
    friend class PaulineAudience;
    al::LiveActor* mActor = nullptr;
    f32 mCurrentBeat = -1.0f;
    f32 mPrevBeat = -1.0f;

    RhythmEntry* mDanceEntries = nullptr;
    f32 mDanceBeat = -1.0f;
    s32 mDanceAnimId = -1;
    s32 mDanceCount = -1;
    s32 mDanceIndex = 0;

    RhythmEntry* mLipEntries = nullptr;
    f32 mLipBeat = -1.0f;
    s32 mLipType = -1;
    s32 mLipCount = -1;
    s32 mLipIndex = 0;

    RhythmEntry* mFaceEntries = nullptr;
    f32 mFaceBeat = -1.0f;
    s32 mFaceType = -1;
    s32 mFaceCount = -1;
    s32 mFaceIndex = 0;

    RhythmEntry* mEyelineEntries = nullptr;
    f32 mEyelineBeat = -1.0f;
    s32 mEyelineType = -1;
    s32 mEyelineCount = -1;
    s32 mEyelineIndex = 0;

    f32* mReSingEntries = nullptr;
    f32 mReSingBeat = -1.0f;
    bool mIsReSing = false;
    s32 mReSingCount = -1;
    s32 mReSingIndex = 0;

    bool mIsLoopingFlag = false;
    bool _89 = false;
};

static_assert(sizeof(PaulineRhythmInfo) == 0x90);
