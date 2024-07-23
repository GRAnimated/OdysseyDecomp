#pragma once

#include <basis/seadTypes.h>

#include "Library/Sequence/Sequence.h"
#include "System/GameDataHolderAccessor.h"

class HakoniwaStateDemoOpening;
class HakoniwaStateDemoEnding;
class HakoniwaStateDemoWorldWarp;
class HakoniwaStateSimpleDemo;
class HakoniwaStateBootLoadData;
class HakoniwaStateDeleteScene;
class WorldResourceLoader;
class StageScene;
class GameDataHolderAccessor;
class BootLayout;
class TimeBalloonSequenceInfo;
class CollectBgmPlayer;
class LoadLayoutCtrl;

namespace al {
class WipeHolder;
class SequenceInitInfo;
class Scene;
class AudioDirector;
class LayoutKit;
class SimpleLayoutAppearWaitEnd;
class AsyncFunctorThread;
class SeadAudioPlayer;
class AudioBusSendFader;
class SimpleAudioUser;

}  // namespace al

class HakoniwaSequence : public al::Sequence {
public:
    HakoniwaSequence(const char* name);

    virtual void init(al::SequenceInitInfo const& info);
    virtual void update();
    virtual void drawMain() const;

    void updatePadSystem();
    void destroySceneHeap(bool);
    void initSystem();
    void isEnableSave() const;

    void exeBootLoadData();
    void exeDemoOpening();
    void exeLoadWorldResource();
    void exeLoadWorldResourceWithBoot();
    void exeLoadStage();
    void exePlayStage();
    void exeDemoWorldWarp();
    void exeDemoEnding();
    void exeDestroy();
    void exeMiss();
    void exeMissCoinSub();
    void exeMissEnd();
    void exeDemoLava();
    void exeFadeToNewGame();
    void exeChangeLanguage();
    void exeWaitWriteData();
    void exeWaitLoadData();
    void exeWaitWriteDataModeChange();
    void exeWaitLoadDataModeChange();

    virtual bool isDisposable() const;
    virtual al::Scene* getCurrentScene() const;

private:
    al::Scene* mCurrentScene;
    GameDataHolderAccessor mGameDataHolderAccessor;
    al::GamePadSystem* mGamePadSystem;
    HakoniwaStateDemoOpening* mStateDemoOpening;
    HakoniwaStateDemoEnding* mStateDemoEnding;
    HakoniwaStateDemoWorldWarp* mStateDemoWorldWarp;
    HakoniwaStateSimpleDemo* mStateSimpleDemo;
    HakoniwaStateBootLoadData* mStateBootLoadData;
    HakoniwaStateDeleteScene* mStateDeleteScene;
    al::LayoutKit* mLayoutKit;
    bool field_100;
    sead::FixedSafeString<128> mStageName;
    s32 mScenarioNum;
    s32 field_1A4;
    al::ScreenCaptureExecutor* mScreenCaptureExecutor;
    al::WipeHolder* mWipeHolder;
    bool mIsMissEnd;
    al::SimpleLayoutAppearWaitEnd* mCounterMiss;
    s32 mCurrentCoins;
    s32 mFinalCoins;
    BootLayout* mBootLayout;
    al::EffectSystem* mEffectSystem;
    al::AsyncFunctorThread* mInitThread;
    bool mIsInitialized;
    al::SeadAudioPlayer* mSeAudioPlayer;
    al::SeadAudioPlayer* mBgmAudioPlayer;
    al::AudioBusSendFader* mAudioBusSendFader;
    WorldResourceLoader* mResourceLoader;
    sead::Heap* mPlayerResourceHeap;
    sead::FixedSafeString<128> mCapName;
    sead::FixedSafeString<128> mCostumeName;
    al::SimpleAudioUser* mPlayerAudioUser;
    bool mIsHackEnd;
    TimeBalloonSequenceInfo* mBalloonSeqInfo;
    CollectBgmPlayer* mCollectBgmPlayer;
    sead::FixedSafeString<128> mLanguage;
    s32 mFileId;
    LoadLayoutCtrl* mLoadLayoutCtrl;
    bool mIsKidsMode;
};
