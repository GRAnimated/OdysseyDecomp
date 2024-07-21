#pragma once

#include <prim/seadSafeString.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LayoutInitInfo;
class GamePadSystem;
class WipeHolder;
class ScreenCaptureExecutor;
class AsyncFunctorThread;
}  // namespace al
class BootLayout;
class HakoniwaSequence;
class WorldResourceLoader;
class GameDataHolder;
class HakoniwaStateDeleteScene;
class TitleMenuScene;

class HakoniwaStateBootLoadData : public al::HostStateBase<HakoniwaSequence> {
public:
    HakoniwaStateBootLoadData(HakoniwaSequence* sequence, al::GamePadSystem* gamePadSystem,
                              al::WipeHolder* wipeHolder,
                              al::ScreenCaptureExecutor* screenCaptureExecutor,
                              WorldResourceLoader* worldResourceLoader, BootLayout* bootLayout,
                              al::LayoutInitInfo const& layoutInitInfo,
                              GameDataHolder* gameDataHolder,
                              HakoniwaStateDeleteScene* stateDeleteScene,
                              al::AsyncFunctorThread* asyncFunctorThread);

    const char* getLanguage() const;
    bool isChangeLanguage() const;
    bool isNewGame() const;
    void startLoad();
    virtual void init() override;
    virtual void appear() override;
    void exeBoot();
    void exeBootLayout();
    void updatePadSystem();
    void exeBootLayoutInitScene();
    void exeMenu();
    void exeDestroyMenu();

private:
    al::GamePadSystem* mGamePadSystem = nullptr;
    al::WipeHolder* mWipeHolder = nullptr;
    BootLayout* mBootLayout = nullptr;
    al::ScreenCaptureExecutor* mScreenCaptureExecutor = nullptr;
    WorldResourceLoader* mWorldResourceLoader = nullptr;
    TitleMenuScene* mTitleMenuScene = nullptr;
    GameDataHolder* mGameDataHolder = nullptr;
    sead::FixedSafeString<128> mLanguage;
    bool mIsNewGame = false;
    HakoniwaStateDeleteScene* mStateDeleteScene = nullptr;
    bool field_100 = false;
    bool mIsStartLoad = false;
    al::AsyncFunctorThread* mAsyncFunctorThread = nullptr;
};
