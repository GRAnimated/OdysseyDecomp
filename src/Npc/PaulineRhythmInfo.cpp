#include "Npc/PaulineRhythmInfo.h"

#include "Library/Bgm/BgmLineFunction.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Yaml/ByamlIter.h"

namespace rs {
void registerRhyhtmInfoListener(al::IUseSceneObjHolder*);
f32 getCurrentBeat(al::IUseSceneObjHolder*);
bool isLooping(al::IUseSceneObjHolder*);
}  // namespace rs

PaulineRhythmInfo::PaulineRhythmInfo(al::LiveActor* actor, const u8* danceData, const u8* lipData,
                                     const u8* faceData, const u8* eyelineData,
                                     const u8* reSingData)
    : mActor(actor) {
    rs::registerRhyhtmInfoListener(mActor);
    initAnimInfo(danceData, lipData, faceData, eyelineData, reSingData);
}

// NON_MATCHING: stack slot offsets differ (entryIter/sample at sp+0x08/0x1c vs sp+0x10/0x0c);
// ReSing init loop unrolls 4 floats per iteration vs target's 8
void PaulineRhythmInfo::initAnimInfo(const u8* danceData, const u8* lipData, const u8* faceData,
                                     const u8* eyelineData, const u8* reSingData) {
    {
        al::ByamlIter danceIter(danceData);
        s32 danceSize = danceIter.getSize();
        mDanceCount = danceSize;
        if (danceSize < 1)
            return;

        mDanceEntries = new RhythmEntry[danceSize];
        if (mDanceCount >= 1) {
            s32 sample;
            s32 danceAnimId;
            for (s32 i = 0; i < mDanceCount; i++) {
                al::ByamlIter entryIter;
                danceIter.tryGetIterByIndex(&entryIter, i);
                entryIter.tryGetIntByKey(&sample, "Sample");
                mDanceEntries[i].beat = (f32)sample / 480.0f;
                entryIter.tryGetIntByKey(&danceAnimId, "DanceAnimId");
                mDanceEntries[i].id = danceAnimId;
            }
        }
    }
    mDanceIndex = 0;

    {
        al::ByamlIter lipIter(lipData);
        s32 lipSize = lipIter.getSize();
        mLipCount = lipSize;
        if (lipSize < 1)
            return;

        mLipEntries = new RhythmEntry[lipSize];
        if (mLipCount >= 1) {
            s32 sample;
            s32 lipType;
            for (s32 i = 0; i < mLipCount; i++) {
                al::ByamlIter entryIter;
                lipIter.tryGetIterByIndex(&entryIter, i);
                entryIter.tryGetIntByKey(&sample, "Sample");
                mLipEntries[i].beat = (f32)sample / 480.0f;
                entryIter.tryGetIntByKey(&lipType, "LipType");
                mLipEntries[i].id = lipType;
            }
        }
    }
    mLipIndex = 0;

    {
        al::ByamlIter faceIter(faceData);
        s32 faceSize = faceIter.getSize();
        mFaceCount = faceSize;
        if (faceSize < 1)
            return;

        mFaceEntries = new RhythmEntry[faceSize];
        if (mFaceCount >= 1) {
            s32 sample;
            s32 faceType;
            for (s32 i = 0; i < mFaceCount; i++) {
                al::ByamlIter entryIter;
                faceIter.tryGetIterByIndex(&entryIter, i);
                entryIter.tryGetIntByKey(&sample, "Sample");
                mFaceEntries[i].beat = (f32)sample / 480.0f;
                entryIter.tryGetIntByKey(&faceType, "FaceType");
                mFaceEntries[i].id = faceType;
            }
        }
    }
    mFaceIndex = 0;

    {
        al::ByamlIter eyelineIter(eyelineData);
        s32 eyelineSize = eyelineIter.getSize();
        mEyelineCount = eyelineSize;
        if (eyelineSize < 1)
            return;

        mEyelineEntries = new RhythmEntry[eyelineSize];
        if (mEyelineCount >= 1) {
            s32 sample;
            s32 eyelineType;
            for (s32 i = 0; i < mEyelineCount; i++) {
                al::ByamlIter entryIter;
                eyelineIter.tryGetIterByIndex(&entryIter, i);
                entryIter.tryGetIntByKey(&sample, "Sample");
                mEyelineEntries[i].beat = (f32)sample / 480.0f;
                entryIter.tryGetIntByKey(&eyelineType, "EyelineType");
                mEyelineEntries[i].id = eyelineType;
            }
        }
    }
    mEyelineIndex = 0;

    {
        al::ByamlIter reSingIter(reSingData);
        s32 reSingSize = reSingIter.getSize();
        mReSingCount = reSingSize;
        if (reSingSize < 1)
            return;

        f32* reSingEntries = new f32[reSingSize];
        for (s32 i = 0; i < reSingSize; i++)
            reSingEntries[i] = -1.0f;
        mReSingEntries = reSingEntries;

        if (mReSingCount >= 1) {
            s32 sample;
            for (s32 i = 0; i < mReSingCount; i++) {
                al::ByamlIter entryIter;
                reSingIter.tryGetIterByIndex(&entryIter, i);
                entryIter.tryGetIntByKey(&sample, "Sample");
                mReSingEntries[i] = (f32)sample / 480.0f;
            }
        }
    }
    mReSingIndex = 0;
}

// NON_MATCHING: regswaps from inlined updateDance/LipSync/Face/ReSing differences
void PaulineRhythmInfo::update(bool forceReset) {
    mPrevBeat = mCurrentBeat;
    mIsReSing = false;
    _89 = false;
    mDanceAnimId = -1;
    mLipType = -1;
    mFaceType = -1;
    mEyelineType = -1;

    mCurrentBeat = rs::getCurrentBeat(mActor);
    if (mCurrentBeat < 0.0f)
        return;

    mIsLoopingFlag = rs::isLooping(mActor);
    if (mIsLoopingFlag) {
        resetRhythmInfo(al::getLoopStartBeat(mActor));
    } else if (mCurrentBeat - mPrevBeat > 0.5f || forceReset) {
        resetRhythmInfo(mCurrentBeat);
        _89 = true;
    }

    updateDance();
    updateReSing();
    updateLipSync();
    updateFace();
    updateEyeline();
}

bool PaulineRhythmInfo::isLooping() {
    return rs::isLooping(mActor);
}

void PaulineRhythmInfo::resetRhythmInfo(f32 beat) {
    if (mDanceCount >= 1) {
        for (s32 i = 0; i < mDanceCount; i++) {
            if (mDanceEntries[i].beat > beat) {
                mDanceIndex = i == 0 ? 0 : i - 1;
                break;
            }
        }
    }

    if (mLipCount >= 1) {
        for (s32 i = 0; i < mLipCount; i++) {
            if (mLipEntries[i].beat > beat) {
                mLipIndex = i == 0 ? 0 : i - 1;
                break;
            }
        }
    }

    if (mFaceCount >= 1) {
        for (s32 i = 0; i < mFaceCount; i++) {
            if (mFaceEntries[i].beat > beat) {
                mFaceIndex = i == 0 ? 0 : i - 1;
                break;
            }
        }
    }

    if (mEyelineCount >= 1) {
        for (s32 i = 0; i < mEyelineCount; i++) {
            if (mEyelineEntries[i].beat > beat) {
                mEyelineIndex = i == 0 ? 0 : i - 1;
                break;
            }
        }
    }

    if (mReSingCount >= 1) {
        for (s32 i = 0; i < mReSingCount; i++) {
            if (mReSingEntries[i] > beat) {
                mReSingIndex = i == 0 ? 0 : i - 1;
                break;
            }
        }
    }
}

// NON_MATCHING: regswaps in loop; compiler hoists nextIdx-1 subtraction out of loop
void PaulineRhythmInfo::updateDance() {
    if (mDanceCount < 1 || mDanceIndex < 0 || mDanceIndex >= mDanceCount)
        return;

    s32 startIdx = mDanceIndex;
    s32 nextIdx = startIdx + 1;
    f32 currentBeat = mCurrentBeat;
    RhythmEntry* entry = &mDanceEntries[startIdx];
    s64 offset = 0;

    for (;;) {
        f32 beat = entry->beat;
        if (currentBeat >= beat - 0.2f) {
            if (nextIdx + (s32)offset == mDanceCount) {
                mDanceBeat = beat;
                mDanceAnimId = entry->id;
                mDanceIndex = -1;
                return;
            }
            if (currentBeat < entry[1].beat - 0.2f) {
                mDanceBeat = beat;
                mDanceIndex = startIdx + 1 + (s32)offset;
                mDanceAnimId = entry->id;
                return;
            }
        }
        offset++;
        entry++;
        if (nextIdx + offset - 1 >= mDanceCount)
            return;
    }
}

// NON_MATCHING: compiler reads count+index as packed 64-bit; 32-bit vs 64-bit loop ops
void PaulineRhythmInfo::updateReSing() {
    if (mReSingCount < 1 || mReSingIndex < 0 || mReSingIndex >= mReSingCount)
        return;

    f32 currentBeat = mCurrentBeat;
    s32 nextIdx = mReSingIndex + 1;
    f32* ptr = &mReSingEntries[nextIdx];

    for (;;) {
        f32 beat = *(ptr - 1);
        if (currentBeat >= beat - 0.25f) {
            if (mReSingCount == nextIdx) {
                mIsReSing = true;
                mReSingBeat = beat;
                mReSingIndex = -1;
                return;
            }
            if (currentBeat < *ptr - 0.25f) {
                mReSingBeat = beat;
                mReSingIndex = nextIdx;
                mIsReSing = true;
                return;
            }
        }
        if (nextIdx >= mReSingCount)
            return;
        nextIdx++;
        ptr++;
    }
}

// NON_MATCHING: regswaps in loop; compiler hoists nextIdx-1 subtraction out of loop
void PaulineRhythmInfo::updateLipSync() {
    if (mLipCount < 1 || mLipIndex < 0 || mLipIndex >= mLipCount)
        return;

    s32 startIdx = mLipIndex;
    s32 nextIdx = startIdx + 1;
    f32 currentBeat = mCurrentBeat;
    RhythmEntry* entry = &mLipEntries[startIdx];
    s64 offset = 0;

    for (;;) {
        f32 beat = entry->beat;
        if (currentBeat >= beat - 0.2f) {
            if (nextIdx + (s32)offset == mLipCount) {
                mLipBeat = beat;
                mLipType = entry->id;
                mLipIndex = -1;
                return;
            }
            if (currentBeat < entry[1].beat - 0.2f) {
                mLipBeat = beat;
                mLipIndex = startIdx + 1 + (s32)offset;
                mLipType = entry->id;
                return;
            }
        }
        offset++;
        entry++;
        if (nextIdx + offset - 1 >= mLipCount)
            return;
    }
}

// NON_MATCHING: regswaps in loop; compiler hoists nextIdx-1 subtraction out of loop
void PaulineRhythmInfo::updateFace() {
    if (mFaceCount < 1 || mFaceIndex < 0 || mFaceIndex >= mFaceCount)
        return;

    s32 startIdx = mFaceIndex;
    s32 nextIdx = startIdx + 1;
    f32 currentBeat = mCurrentBeat;
    RhythmEntry* entry = &mFaceEntries[startIdx];
    s64 offset = 0;

    for (;;) {
        f32 beat = entry->beat;
        if (currentBeat >= beat - 0.2f) {
            if (nextIdx + (s32)offset == mFaceCount) {
                mFaceBeat = beat;
                mFaceType = entry->id;
                mFaceIndex = -1;
                return;
            }
            if (currentBeat < entry[1].beat - 0.2f) {
                mFaceBeat = beat;
                mFaceIndex = startIdx + 1 + (s32)offset;
                mFaceType = entry->id;
                return;
            }
        }
        offset++;
        entry++;
        if (nextIdx + offset - 1 >= mFaceCount)
            return;
    }
}

// NON_MATCHING: regswaps in loop; compiler hoists nextIdx-1 subtraction out of loop
void PaulineRhythmInfo::updateEyeline() {
    if (mEyelineCount < 1 || mEyelineIndex < 0 || mEyelineIndex >= mEyelineCount)
        return;

    s32 startIdx = mEyelineIndex;
    s32 nextIdx = startIdx + 1;
    f32 currentBeat = mCurrentBeat;
    RhythmEntry* entry = &mEyelineEntries[startIdx];
    s64 offset = 0;

    for (;;) {
        f32 beat = entry->beat;
        if (currentBeat >= beat - 0.2f) {
            if (nextIdx + (s32)offset == mEyelineCount) {
                mEyelineBeat = beat;
                mEyelineType = entry->id;
                mEyelineIndex = -1;
                return;
            }
            if (currentBeat < entry[1].beat - 0.2f)
                break;
        }
        offset++;
        entry++;
        if (nextIdx + offset - 1 >= mEyelineCount)
            return;
    }

    mEyelineBeat = entry->beat;
    mEyelineIndex = startIdx + 1 + (s32)offset;
    mEyelineType = entry->id;
}

s32 PaulineRhythmInfo::getDanceAnimId(s32 index) {
    if (index < 0)
        return -1;
    return mDanceEntries[index].id;
}

f32 PaulineRhythmInfo::getDanceBeat(s32 index) {
    if (index < 0)
        return -1.0f;
    return mDanceEntries[index].beat;
}
