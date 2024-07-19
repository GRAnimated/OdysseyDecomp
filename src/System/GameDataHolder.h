#pragma once

#include <prim/seadSafeString.h>

#include "Library/Message/MessageSystem.h"
#include "Library/Scene/GameDataHolderBase.h"

#include "System/GameDataFile.h"
#include "System/WorldList.h"

namespace al {
class PlacementId;
}
class SaveDataAccessSequence;
class GameConfigData;
class TempSaveData;
class CapMessageBossData;
class AchievementHolder;
class MapDataHolder;
class UniqObjInfo;
class QuestInfoHolder;
class GameSequenceInfo;
class TimeBalloonSequenceInfo;

class GameDataHolder : public al::GameDataHolderBase {
public:
    GameDataHolder(const al::MessageSystem*);
    GameDataHolder();

    virtual ~GameDataHolder();

    virtual char* getSceneObjName() const;
    virtual al::MessageSystem* getMessageSystem() const;

    GameConfigData* getConfigData() const { return mConfigData; }

    WorldList* getWorldList() const { return mWorldList; }

    void setPlayingFileId(s32 file);
    void intitalizeData();
    void initialzeDataCommon();
    void resetTempSaveData(bool);
    void initializeDataId(s32);
    void readByamlData(s32, const char*);
    s32 tryFindEmptyFileId() const;

    bool isRequireSave() const;
    void setRequireSave();
    void setRequireSaveFalse();
    void setRequireSaveFrame();
    void updateRequireSaveFrame();
    bool isInvalidSaveForMoonGet() const;
    void invalidateSaveForMoonGet();
    void validateSaveForMoonGet();
    void setLanguage(const char*);
    char* getLanguage() const;

    void resetLocationName();
    void changeNextStageWithDemoWorldWarp(const char*);
    bool tryChangeNextStageWithWorldWarpHole(const char*);
    void returnPrevStage();
    char* getNextStageName() const;
    char* getNextStageName(s32 idx) const;
    GameDataFile* getGameDataFile(s32 idx) const;
    u64 getNextPlayerStartId() const;
    char* getCurrentStageName() const;
    char* tryGetCurrentStageName() const;
    char* getCurrentStageName(s32 idx) const;
    void setCheckpointId(const al::PlacementId*);
    char* tryGetRestartPointIdString() const;
    void endStage();
    void startStage(const char*, s32);
    void onObjNoWriteSaveData(const al::PlacementId*);
    void offObjNoWriteSaveData(const al::PlacementId*);
    bool isOnObjNoWriteSaveData(const al::PlacementId*) const;
    void onObjNoWriteSaveDataResetMiniGame(const al::PlacementId);
    void offObjNoWriteSaveDataResetMiniGame(const al::PlacementId);
    bool isOnObjNoWriteSaveDataResetMiniGame(const al::PlacementId) const;
    void onObjNoWriteSaveDataInSameScenario(const al::PlacementId);
    bool isOnObjNoWriteSaveDataInSameScenario(const al::PlacementId) const;
    void writeTempSaveDataToHash(const char*, bool);

    void resetMiniGameData();
    s32 getPlayingFileId() const;
    void requestSetPlayingFileId(s32);
    void receiveSetPlayingFileIdMsg();

    s32 findUnlockShineNum(bool*, s32) const;
    s32 calcBeforePhaseWorldNumMax(s32) const;
    bool isFindKoopaNext(s32) const;
    bool isBossAttackedHomeNext(s32) const;
    void playScenarioStartCamera(s32);
    bool isPlayAlreadyScenarioStartCamera() const;

    const char* getCoinCollectArchiveName(s32) const;
    const char* getCoinCollectEmptyArchiveName(s32) const;
    const char* getCoinCollect2DArchiveName(s32) const;
    const char* getCoinCollect2DEmptyArchiveName(s32) const;
    s32 getShineAnimFrame(s32) const;
    s32 getCoinCollectNumMax(s32) const;

    void readFromSaveDataBufferCommonFileOnlyLanguage();
    void readFromSaveDataBuffer(const char* bufferName);

    void changeNextStage(const struct ChangeStageInfo*, s32);

    s32 findUseScenarioNo(const char*);

    GameSequenceInfo* getSequenceInfo() const { return mSequenceInfo; }

private:
    al::MessageSystem* mMessageSystem;
    GameDataFile** mGameDataFileArray;
    GameDataFile* mPrimaryGameDataFile;
    GameDataFile* field_28;
    s32 mPlayingFileId;
    SaveDataAccessSequence* mSaveDataAccessSequence;
    bool mIsRequireSave;
    s32 mRequireSameFrame;
    bool mIsInvalidSaveForMoonGet;
    bool mIsInvalidToChangeStage;
    bool field_4A;
    s32 field_4C;
    sead::FixedSafeString<32> mLanguage;
    void* field_88;
    sead::Heap* mHeap;
    void* field_98;
    GameConfigData* mConfigData;
    TempSaveData* mTempSaveData;
    TempSaveData* field_B0;
    CapMessageBossData* mCapMessageBossData;
    void* field_C0;
    s32 field_C8;
    void* field_D0;
    bool* field_D8;
    sead::PtrArrayImpl mStageLockList;
    sead::PtrArrayImpl mItemList;
    sead::PtrArrayImpl field_100;
    sead::PtrArrayImpl field_110;
    sead::PtrArrayImpl field_120;
    sead::PtrArrayImpl field_130;
    sead::PtrArrayImpl field_140;
    sead::PtrArrayImpl mHackObjList;
    sead::PtrArrayImpl field_160;
    s32** field_170;
    s32 field_178;
    void* mAchievementInfoReader;
    AchievementHolder* mAchievementHolder;
    WorldList* mWorldList;
    sead::PtrArrayImpl mChangeStageList;
    sead::PtrArrayImpl field_1A8;
    sead::PtrArrayImpl mInvalidOpenMapList;
    sead::PtrArrayImpl mTutorialLabels;
    void* field_1D8;
    MapDataHolder* mMapDataHolder;
    sead::PtrArrayImpl mWorldItemTypeList;
    s32** mCollectCoinNum;
    s32** mWorldLinkInfo;
    void* field_208;
    s32 field_210;
    UniqObjInfo* mUniqObjInfo;
    char field_220;
    s32 field_224;
    char field_228;
    s32 field_22C;
    s32 field_230;
    s32 field_234;
    void* field_238;
    s32 field_240;
    bool field_244;
    bool mIsSeparatePlay;
    QuestInfoHolder* mQuestInfoHolder;
    char field_250;
    GameSequenceInfo* mSequenceInfo;
    TimeBalloonSequenceInfo* mTimeBalloonSequenceInfo;
};
