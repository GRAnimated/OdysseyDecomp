#include "Sequence/HakoniwaStateBootLoadData.h"

/*
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
    al::GamePadSystem* mGamePadSystem;
    al::WipeHolder* mWipeHolder;
    BootLayout* mBootLayout;
    al::ScreenCaptureExecutor* mScreenCaptureExecutor;
    WorldResourceLoader* mWorldResourceLoader;
    TitleMenuScene* mTitleMenuScene;
    GameDataHolder* mGameDataHolder;
    sead::FixedSafeString<128> mLanguage;
    bool mIsNewGame;
    HakoniwaStateDeleteScene* mStateDeleteScene;
    bool field_100;
    bool mIsStartLoad;
    al::AsyncFunctorThread* mAsyncFunctorThread;
};

*/

#include "Layout/BootLayout.h"
#include "Library/Memory/HeapUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Thread/AsyncFunctorThread.h"
#include "System/GameDataHolder.h"
#include "Util/StageInputFunction.h"

namespace {
NERVE_HOST_TYPE_IMPL(HakoniwaStateBootLoadData, Boot);
NERVE_HOST_TYPE_IMPL(HakoniwaStateBootLoadData, DestroyMenu);
NERVE_HOST_TYPE_IMPL(HakoniwaStateBootLoadData, BootLayoutInitScene);
NERVE_HOST_TYPE_IMPL(HakoniwaStateBootLoadData, BootLayout);
NERVE_HOST_TYPE_IMPL(HakoniwaStateBootLoadData, Menu);

HostTypeNrvBootLayout NrvBootLayout;  // TODO: BootLayout nerve in a macro conflicts with the name
                                      // of the BootLayout class
NERVES_MAKE_NOSTRUCT(HostType, Menu);
NERVES_MAKE_STRUCT(HostType, Boot, DestroyMenu, BootLayoutInitScene);
}  // namespace

HakoniwaStateBootLoadData::HakoniwaStateBootLoadData(
    HakoniwaSequence* sequence, al::GamePadSystem* gamePadSystem, al::WipeHolder* wipeHolder,
    al::ScreenCaptureExecutor* screenCaptureExecutor, WorldResourceLoader* worldResourceLoader,
    BootLayout* bootLayout, al::LayoutInitInfo const& layoutInitInfo,
    GameDataHolder* gameDataHolder, HakoniwaStateDeleteScene* stateDeleteScene,
    al::AsyncFunctorThread* asyncFunctorThread)
    : al::HostStateBase<HakoniwaSequence>("起動", sequence), mGamePadSystem(gamePadSystem),
      mWipeHolder(wipeHolder), mBootLayout(bootLayout),
      mScreenCaptureExecutor(screenCaptureExecutor), mWorldResourceLoader(worldResourceLoader),
      mGameDataHolder(gameDataHolder), mStateDeleteScene(stateDeleteScene),
      mAsyncFunctorThread(asyncFunctorThread) {}

const char* HakoniwaStateBootLoadData::getLanguage() const {
    return mLanguage.cstr();
}

bool HakoniwaStateBootLoadData::isChangeLanguage() const {
    return !mLanguage.isEmpty();
}

bool HakoniwaStateBootLoadData::isNewGame() const {
    return mIsNewGame;
}

void HakoniwaStateBootLoadData::startLoad() {
    mIsStartLoad = true;
}

void HakoniwaStateBootLoadData::init() {
    initNerve(&NrvHostType.Boot, 2);
    // TODO: Remove cast here
    al::addNerveState(this, (al::NerveStateBase*)mStateDeleteScene, &NrvHostType.DestroyMenu,
                      "シーン破棄[タイトルメニュー]");
}

void HakoniwaStateBootLoadData::appear() {
    setDead(false);
    mIsNewGame = false;
    field_100 = false;
    if (mIsStartLoad)
        al::setNerve(this, &NrvHostType.BootLayoutInitScene);
    else
        al::setNerve(this, &NrvHostType.Boot);
}

void HakoniwaStateBootLoadData::exeBoot() {
    if (al::isFirstStep(this))
        mAsyncFunctorThread->start();
    if (al::isGreaterEqualStep(this, 60))
        al::setNerve(this, &NrvBootLayout);
}

void HakoniwaStateBootLoadData::exeBootLayout() {
    updatePadSystem();
    if (al::isFirstStep(this))
        mBootLayout->appear();
    if (!al::isLessEqualStep(this, 540) && mAsyncFunctorThread->isDone())
        al::setNerve(this, &NrvHostType.BootLayoutInitScene);
}

void HakoniwaStateBootLoadData::updatePadSystem() {
    if (!ControllerAppletFunction::tryReconnectGamePad(mGamePadSystem) &&
        mGameDataHolder->isSeparatePlay() &&
        ControllerAppletFunction::connectControllerSinglePlay(mGamePadSystem))
        mGameDataHolder->setSeparatePlay(false);
}

/*
void HakoniwaStateBootLoadData::exeBootLayoutInitScene() {
    updatePadSystem();

    if (al::isFirstStep(this)) {
        al::createSceneHeap(nullptr, true);
    }
}
*/