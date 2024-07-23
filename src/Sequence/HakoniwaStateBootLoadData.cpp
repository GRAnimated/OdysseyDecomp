#include "Sequence/HakoniwaStateBootLoadData.h"

#include <heap/seadHeapMgr.h>
#include <thread/seadThread.h>

#include "Library/Audio/System/AudioKeeperFunction.h"
#include "Library/Bgm/BgmLineFunction.h"
#include "Library/Memory/HeapUtil.h"
#include "Library/Memory/SceneHeapSetter.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Play/Layout/WipeHolder.h"
#include "Library/Screen/ScreenCaptureExecutor.h"
#include "Library/Sequence/Sequence.h"
#include "Library/Thread/AsyncFunctorThread.h"

#include "Layout/BootLayout.h"
#include "Scene/TitleMenuScene.h"
#include "Sequence/HakoniwaSequence.h"
#include "Sequence/HakoniwaStateDeleteScene.h"
#include "Sequence/WorldResourceLoader.h"
#include "System/GameDataFunction.h"
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

s32 cDefaultPriority = sead::Thread::cDefaultPriority;
const s32 cThreadPriority = sead::Thread::cDefaultPriority + 3;

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
    al::addNerveState(this, mStateDeleteScene, &NrvHostType.DestroyMenu,
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

void HakoniwaStateBootLoadData::exeBootLayoutInitScene() {
    updatePadSystem();

    if (al::isFirstStep(this)) {
        al::createSceneHeap(nullptr, true);
        al::SceneHeapSetter sceneHeapSetter;

        mTitleMenuScene = new TitleMenuScene();
        alAudioSystemFunction::resetDataDependedStage((getHost())->getAudioDirector(), nullptr, 1);
        al::setSceneAndUseInitThread(getHost(), mTitleMenuScene, cThreadPriority, 0, 1,
                                     "Sequence=ProductSequence", nullptr);

        sead::ScopedCurrentHeapSetter setter(sceneHeapSetter.getLastHeap());
    }
    if (al::tryEndSceneInitThread(getHost())) {
        al::setSequenceAudioKeeperToSceneSeDirector(getHost(), mTitleMenuScene);
        mTitleMenuScene->appear();
        if (mIsStartLoad)
            mTitleMenuScene->startLoadDirect(false);
        (getHost())->setCurrentScene(mTitleMenuScene);
        al::setSequenceNameForActorPickTool(getHost(), mTitleMenuScene);
        al::getSceneHeap()->adjust();
        mScreenCaptureExecutor->offDraw();
        if (mWipeHolder->getField18())
            mWipeHolder->startOpen(-1);
        al::setNerve(this, &Menu);
    }
}

void HakoniwaStateBootLoadData::exeMenu() {
    updatePadSystem();
    if (al::isFirstStep(this)) {
        mBootLayout->kill();
        s32 currentWorldId = GameDataFunction::getCurrentWorldId(mGameDataHolder);
        s32 scenarioNo = GameDataFunction::calcNextScenarioNo(mGameDataHolder);
        if (scenarioNo == -1)
            scenarioNo = 1;
        mWorldResourceLoader->requestLoadWorldHomeStageResource(currentWorldId, scenarioNo);
    }

    mTitleMenuScene->setLoadPercent(mWorldResourceLoader->calcLoadPercent());

    if (mTitleMenuScene->isCancelLoadWorldResource()) {
        field_100 = true;
        mWorldResourceLoader->cancelLoadWorldResource();
    }

    if (!mIsStartLoad && al::isStep(this, 195))
        al::startBgm(getHost(), "Title", -1, 0);

    if (field_100) {
        if (!mWorldResourceLoader->isEndLoadWorldResource())
            return;

        field_100 = false;
        mWorldResourceLoader->tryDestroyWorldResource();
        mTitleMenuScene->startLoadWorldResource();
        s32 currentWorldId = GameDataFunction::getCurrentWorldId(mGameDataHolder);
        s32 scenarioNo = GameDataFunction::calcNextScenarioNo(mGameDataHolder);
        if (scenarioNo == -1)
            scenarioNo = 1;
        mWorldResourceLoader->requestLoadWorldHomeStageResource(currentWorldId, scenarioNo);
    }
    bool v19 = mTitleMenuScene->isChangeLanguage();
    bool test = false;
    bool isNewGame = mTitleMenuScene->isNewGame();
    if (v19 || isNewGame) {
        if (isNewGame)
            mIsNewGame = true;
        if (v19)
            mLanguage.format("%s", mTitleMenuScene->getLanguage());
        mWorldResourceLoader->cancelLoadWorldResource();
        test = true;
    }

    if (mTitleMenuScene->isEnableKill()) {
        if (test || mWorldResourceLoader->isEndLoadWorldResource()) {
            mTitleMenuScene->kill();
            mScreenCaptureExecutor->requestCapture(true, 0);
            al::setNerve(this, &NrvHostType.DestroyMenu);
        }
    }
}

void HakoniwaStateBootLoadData::exeDestroyMenu() {
    if (al::isFirstStep(this)) {
        mWipeHolder->startClose("FadeBlack", -1);
        mStateDeleteScene->start(mTitleMenuScene, true, false, 0);
    }
    if (al::updateNerveState(this)) {
        mTitleMenuScene = nullptr;
        al::stopBgm(getHost(), "Title", 140);
        kill();
    }
}
