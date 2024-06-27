#include "Scene/StageSceneStatePauseMenu.h"
#include "Layout/FooterParts.h"
#include "Layout/MenuSelectParts.h"
#include "Layout/StageSceneLayout.h"
#include "Library/Base/StringUtil.h"
#include "Library/Bgm/BgmLineFunction.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Controller/KeyRepeatCtrl.h"
#include "Library/Draw/GraphicsFunction.h"
#include "Library/Layout/LayoutActorUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/LiveActor/ActorSceneInfo.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/LiveActor/SubActorKeeper.h"
#include "Library/Message/MessageHolder.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Obj/PartsModel.h"
#include "Library/Play/Layout/SimpleLayoutAppearWaitEnd.h"
#include "Library/Play/Layout/WipeSimple.h"
#include "Library/Scene/SceneUtil.h"
#include "Library/Se/SeFunction.h"
#include "Library/System/GameSystemInfo.h"
#include "Project/Scene/SceneInitInfo.h"
#include "Scene/SceneAudioSystemPauseController.h"
#include "Scene/StageScene.h"
#include "Scene/StageSceneStateEndSeparatePlay.h"
#include "Scene/StageSceneStateOption.h"
#include "Scene/StageSceneStateStartSeparatePlay.h"
#include "Sequence/GameSequenceInfo.h"
#include "System/SaveDataAccessFunction.h"
#include "Util/HelpFunction.h"
#include "Util/InputInterruptTutorialUtil.h"
#include "Util/SpecialBuildUtil.h"
#include "Util/StageInputFunction.h"
#include "nn/oe.h"

NERVE_IMPL(StageSceneStatePauseMenu, Appear);
NERVE_IMPL(StageSceneStatePauseMenu, StartSeparatePlay);
NERVE_IMPL(StageSceneStatePauseMenu, EndSeparatePlay);
NERVE_IMPL(StageSceneStatePauseMenu, Option);
NERVE_IMPL_(StageSceneStatePauseMenu, OptionFromHelp, Option);
NERVE_IMPL(StageSceneStatePauseMenu, Wait);
NERVE_IMPL(StageSceneStatePauseMenu, End);
NERVE_IMPL(StageSceneStatePauseMenu, WaitDraw);
NERVE_IMPL(StageSceneStatePauseMenu, Save);
NERVE_IMPL(StageSceneStatePauseMenu, ConfirmNewGame);
NERVE_IMPL(StageSceneStatePauseMenu, NotExistEmptyFile);
NERVE_IMPL(StageSceneStatePauseMenu, FadeBeforeHelp);
NERVE_IMPL(StageSceneStatePauseMenu, StartHelp);
NERVE_IMPL_(StageSceneStatePauseMenu, StartHelpFromOption, StartHelp);

NERVE_MAKE(StageSceneStatePauseMenu, StartHelp);
NERVES_MAKE_STRUCT(StageSceneStatePauseMenu, Appear, StartSeparatePlay, EndSeparatePlay, Option,
                   OptionFromHelp, Wait, End, WaitDraw, Save, ConfirmNewGame, NotExistEmptyFile,
                   FadeBeforeHelp, StartHelpFromOption);

StageSceneStatePauseMenu::StageSceneStatePauseMenu(
    const char* name, al::Scene* host, al::SimpleLayoutAppearWaitEnd* menuLayout,
    GameDataHolder* gameDataHolder, const al::SceneInitInfo& sceneInitInfo,
    const al::ActorInitInfo& actorInitInfo, const al::LayoutInitInfo& layoutInitInfo,
    al::WindowConfirm* windowConfirm, StageSceneLayout* stageSceneLayout, bool a11,
    SceneAudioSystemPauseController* sceneAudioSystemPauseController)
    : al::HostStateBase<al::Scene>(name, host), mMenuLayout(menuLayout),
      mGameDataHolder(gameDataHolder), mWindowConfirm(windowConfirm),
      mStageSceneLayout(stageSceneLayout),
      mSceneAudioSystemPauseController(sceneAudioSystemPauseController) {
    mMarioHigh = new al::LiveActor("メニュー用マリオモデル");
    al::initActorWithArchiveName(mMarioHigh, actorInitInfo, "MarioHigh", "PauseMenu");
    mMarioHigh->makeActorDead();
    al::PartsModel* capEyes = new al::PartsModel("メニュー用キャップ目");
    al::LiveActor* hatActor = al::getSubActor(mMarioHigh, "頭");
    capEyes->initPartsFixFile(hatActor, actorInitInfo, "CapManHeroHighEyes", nullptr, nullptr);
    capEyes->makeActorDead();
    al::onSyncAppearSubActor(hatActor, capEyes);
    al::startAction(capEyes, "Wait");
    mSelectParts =
        new MenuSelectParts("[ポーズメニュー]選択肢", mMenuLayout, mMarioHigh, layoutInitInfo, 6);
    mMenuGuide = new al::SimpleLayoutAppearWaitEnd("[ポーズメニュー]フッター", "MenuGuide",
                                                   layoutInitInfo, nullptr, false);
    mMenuRight = new al::SimpleLayoutAppearWaitEnd("[ポーズメニュー]権利", "MenuRight",
                                                   layoutInitInfo, nullptr, false);
    nn::oe::DisplayVersion displayVersion;
    nn::oe::GetDisplayVersion(&displayVersion);
    if (al::isEqualString(displayVersion.name, "1.0.0"))
        al::hidePane(mMenuRight, "TxtVersion");
    else
        al::setPaneStringFormat(mMenuRight, "TxtVersion", displayVersion.name);
    mFooterParts =
        new FooterParts(mMenuGuide, layoutInitInfo,
                        al::getSystemMessageString(mMenuGuide, "Footer", "MenuMessage_Footer"),
                        "TxtGuide", "ParFooter");
    mMenuGuide->kill();
    mMenuWipe = new al::WipeSimple("メニュー黒フェード", "FadeBlack", layoutInitInfo, nullptr);
    mMenuWipe->kill();
    mHelpWipe = new al::WipeSimple("ヘルプ黒フェード", "FadeBlack", layoutInitInfo, "BeforeWindow");
    mHelpWipe->kill();
    mPauseCameraCtrl = al::initAndCreatePauseCameraCtrl(getHost(), 10.0f);
    mKeyRepeatCtrl = new al::KeyRepeatCtrl();
    mKeyRepeatCtrl->init(30, 5);
    initNerve(&NrvStageSceneStatePauseMenu.Appear, 4);
    al::GamePadSystem* gamePadSystem = sceneInitInfo.gameSysInfo->gamePadSystem;
    mStateStartSeparatePlay = new StageSceneStateStartSeparatePlay(
        "おすそ分け開始", this, layoutInitInfo, mMenuWipe, gamePadSystem, mFooterParts);
    mStateEndSeparatePlay = new StageSceneStateEndSeparatePlay(
        "おすそ分け終了", this, layoutInitInfo, mMenuWipe, gamePadSystem);
    mStateOption = new StageSceneStateOption("オプション画面", getHost(), layoutInitInfo,
                                             mFooterParts, mGameDataHolder, a11);
    al::initNerveState(this, mStateStartSeparatePlay,
                       &NrvStageSceneStatePauseMenu.StartSeparatePlay, "おすそ分けプレイ開始");
    al::initNerveState(this, mStateEndSeparatePlay, &NrvStageSceneStatePauseMenu.EndSeparatePlay,
                       "おすそ分けプレイ終了");
    al::initNerveState(this, mStateOption, &NrvStageSceneStatePauseMenu.Option, "オプション画面");
    al::addNerveState(this, mStateOption, &NrvStageSceneStatePauseMenu.OptionFromHelp,
                      "オプション画面[ヘルプから遷移]");
    rs::registerGraphicsPresetPause(getHost());
    mHtmlViewer = sceneInitInfo.gameSysInfo->htmlViewer;
}

void StageSceneStatePauseMenu::appear() {
    mStartType = 0;
    setDead(false);
    if (rs::isModeE3LiveRom()) {
        if (rs::isSeparatePlay(getHost()))
            al::setNerve(this, &NrvStageSceneStatePauseMenu.EndSeparatePlay);
        else
            al::setNerve(this, &NrvStageSceneStatePauseMenu.StartSeparatePlay);
        return;
    }
    al::setNerve(this, &NrvStageSceneStatePauseMenu.Appear);
}

void StageSceneStatePauseMenu::kill() {
    rs::updateGyroText(getHost());
    setDead(true);
    killPauseMenu();
    killMarioModel();  // redundant as killPauseMenu() already does this
    if (!isNeedKillHost() && mSceneAudioSystemPauseController) {
        mSceneAudioSystemPauseController->resume(1);
        al::endPausePadRumble(getHost());
    }
    alGraphicsFunction::validateGpuStressAnalyzer(getHost());
}

void StageSceneStatePauseMenu::killPauseMenu() {
    mMenuLayout->kill();
    mMenuRight->kill();
    mMenuGuide->kill();
    killMarioModel();
}

void StageSceneStatePauseMenu::killMarioModel() {
    al::endCameraPause(mPauseCameraCtrl);
    if (mIsPauseMenu)
        al::setNearClipDistance(getHost(), mPrevNearClipDistance, 0);
    if (al::isAlive(mMarioHigh))
        mMarioHigh->kill();
}

bool StageSceneStatePauseMenu::isNeedKillHost() const {
    return mStateOption->get180() || mIsNewGame || mStateOption->isChangeLanguage() ||
           mStateOption->isModeSelectEnd();
}

void StageSceneStatePauseMenu::startNormal() {
    mStartType = 1;
    al::pausePadRumble(getHost());
    mSceneAudioSystemPauseController->pause(0);
    alGraphicsFunction::invalidateGpuStressAnalyzer(getHost());
}

void StageSceneStatePauseMenu::startAfterTitle() {
    mStartType = 2;
}

void StageSceneStatePauseMenu::killAllOptionLayout() {
    mStateOption->killAllLayouts();
}

bool StageSceneStatePauseMenu::isEndToCancel() const {
    return mSelectParts->isDecideSetting();
}

bool StageSceneStatePauseMenu::isEndToHelp() const {
    return false;
}

bool StageSceneStatePauseMenu::isLoadData() const {
    return mStateOption->get180();
}

void StageSceneStatePauseMenu::getSelectedFileId() const {
    return mStateOption->getSelectedFileId();
}

bool StageSceneStatePauseMenu::isChangeLanguage() const {
    return mStateOption->isChangeLanguage();
}

const char* StageSceneStatePauseMenu::getLanguage() const {
    return mStateOption->getLanguage();
}

bool StageSceneStatePauseMenu::isNewGame() const {
    return mIsNewGame;
}

bool StageSceneStatePauseMenu::isModeSelectEnd() const {
    return mStateOption->isModeSelectEnd();
}

#include "Library/Effect/EffectSystem.h"
#include "Library/LiveActor/LiveActorKit.h"

bool StageSceneStatePauseMenu::checkNeedKillByHostAndEnd() {
    if (isNeedKillHost() || isModeSelectEnd()) {
        getHost()->getLiveActorKit()->getEffectSystem()->setField69(false);
        mSceneAudioSystemPauseController->resume(0);
        alSeFunction::stopAllSe(getHost()->getAudioDirector(), 0);
        al::stopAllBgm(getHost(), 0);
        return true;
    }
    return false;
}

void StageSceneStatePauseMenu::startActionMario(char const* actionName) {
    mSelectParts->startActionMario(mMarioHigh, actionName);
}

al::LiveActor* StageSceneStatePauseMenu::getMarioActor() const {
    return mMarioHigh;
}

bool StageSceneStatePauseMenu::isDrawLayout() const {
    return !al::isNerve(this, &NrvStageSceneStatePauseMenu.Wait) &&
           !al::isNerve(this, &NrvStageSceneStatePauseMenu.Appear);
}

bool StageSceneStatePauseMenu::isDrawLayoutMain() const {
    return !al::isNerve(this, &NrvStageSceneStatePauseMenu.End) &&
           !al::isNerve(this, &NrvStageSceneStatePauseMenu.WaitDraw);
}

bool StageSceneStatePauseMenu::isDrawViewRenderer() const {
    if (mStateStartSeparatePlay->isDrawViewRenderer() ||
        mStateEndSeparatePlay->isDrawViewRenderer() ||
        al::isNerve(this, &NrvStageSceneStatePauseMenu.WaitDraw) ||
        al::isNerve(this, &NrvStageSceneStatePauseMenu.End))
        return true;
    if (al::isNerve(this, &NrvStageSceneStatePauseMenu.Appear))
        return al::isDead(mMarioHigh);
    return false;
}

bool StageSceneStatePauseMenu::isDrawChromakey() const {}

void StageSceneStatePauseMenu::exeAppear() {}

void StageSceneStatePauseMenu::setNormal() {}

void StageSceneStatePauseMenu::appearMarioModel() {}

void StageSceneStatePauseMenu::updatePlayerPose() {}

void StageSceneStatePauseMenu::exeWait() {}

void StageSceneStatePauseMenu::changeNerveAndReturn(const al::Nerve* nerve) {}

void StageSceneStatePauseMenu::exeFadeBeforeHelp() {}

#include "System/GameDataHolder.h"

// NON_MATCHING: callHelp should take in mGameDataHolder directly
void StageSceneStatePauseMenu::exeStartHelp() {
    al::updateKitListPrev(getHost());
    al::updateKitList(getHost(), "カメラ");
    rs::requestGraphicsPresetAndCubeMapPause(getHost());
    al::updateKitList(getHost(), "シャドウマスク");
    al::updateKitList(getHost(), "グラフィックス要求者");
    al::updateKitList(getHost(), "２Ｄ（ポーズ無視）");
    al::updateKitListPost(getHost());

    if (al::isFirstStep(this)) {
        bool test2 = al::isNerve(this, &NrvStageSceneStatePauseMenu.StartHelpFromOption);
        GameDataHolderAccessor accessor = getHost();
        bool test = rs::isSceneStatusInvalidSave(accessor);

        al::StringTmp<256> helpUrl;
        HelpFunction::callHelp(mHtmlViewer, accessor, test2, test, false,
                               &helpUrl);  // requires passing in mGameDataHolder directly to match,
                                           // cannot because of type

        if (al::isEqualString("http://localhost/ChangeMode", helpUrl.cstr())) {
            mStateOption->set51(true);
            al::setNerve(this, &NrvStageSceneStatePauseMenu.OptionFromHelp);
            return;
        }
        mHelpWipe->startOpen(-1);
    }

    if (mHelpWipe->isOpenEnd()) {
        mSelectParts->appearWait();
        al::setNerve(this, &NrvStageSceneStatePauseMenu.Wait);
    }
}

void helper(al::Scene* scene) {
    al::updateKitListPrev(scene);
    al::updateKitList(scene, "カメラ");
    al::updateKitList(scene, "シャドウマスク");
    al::updateKitList(scene, "グラフィックス要求者");
    al::updateKitList(scene, "２Ｄ（ポーズ無視）");
    al::updateKitListPost(scene);
}

void StageSceneStatePauseMenu::exeWaitDraw() {
    helper(getHost());
    if (al::isFirstStep(this))
        alGraphicsFunction::requestUpdateMaterialInfo(getHost());
    if (al::isGreaterEqualStep(this, 2))
        al::setNerve(this, &NrvStageSceneStatePauseMenu.End);
}

void StageSceneStatePauseMenu::exeEnd() {
    helper(getHost());
    if (al::isFirstStep(this)) {
        mMenuLayout->end();
        mMenuRight->end();
        mMenuGuide->end();
    }
    if (al::isDead(mMenuLayout))
        appear();
}

void StageSceneStatePauseMenu::exeStartSeparatePlay() {
    if (al::isFirstStep(this)) {
        mFooterParts->tryChangeTextFade(
            al::getSystemMessageString(mFooterParts, "Footer", "MenuMessage_Footer_SeparatePlay"));
    }
    al::updateKitListPrev(getHost());
    al::updateKitList(getHost(), "カメラ");
    if (mStateStartSeparatePlay->isNeedRequestGraphicsPreset())
        rs::requestGraphicsPresetAndCubeMapPause(getHost());
    al::updateKitList(getHost(), "シャドウマスク");
    al::updateKitList(getHost(), "グラフィックス要求者");
    al::updateKitList(getHost(), "２Ｄ（ポーズ無視）");
    al::updateKitListPost(getHost());

    if (al::updateNerveState(this)) {
        if (mStageSceneLayout)
            mStageSceneLayout->setDirtyFlagForPlayGuideMenu();
        if (this->mStateStartSeparatePlay->getField42() && !rs::isModeE3LiveRom()) {
            mSelectParts->appearWait();
            al::setNerve(this, &NrvStageSceneStatePauseMenu.Wait);
            return;
        }
        kill();
    }
}

void StageSceneStatePauseMenu::exeEndSeparatePlay() {
    al::updateKitListPrev(getHost());
    al::updateKitList(getHost(), "カメラ");
    if (mStateEndSeparatePlay->isNeedRequestGraphicsPreset())
        rs::requestGraphicsPresetAndCubeMapPause(getHost());
    al::updateKitList(getHost(), "シャドウマスク");
    al::updateKitList(getHost(), "グラフィックス要求者");
    al::updateKitList(getHost(), "２Ｄ（ポーズ無視）");
    al::updateKitListPost(getHost());

    if (al::updateNerveState(this)) {
        if (mStageSceneLayout)
            mStageSceneLayout->setDirtyFlagForPlayGuideMenu();

        if (mStateEndSeparatePlay->getField30() && !rs::isModeE3LiveRom()) {
            mSelectParts->appearWait();
            al::setNerve(this, &NrvStageSceneStatePauseMenu.Wait);
            return;
        }
        kill();
    }
}

void StageSceneStatePauseMenu::exeOption() {
    al::updateKitListPrev(getHost());
    rs::requestGraphicsPresetAndCubeMapPause(getHost());
    al::updateKitList(getHost(), "２Ｄ（ポーズ無視）");
    al::updateKitListPost(getHost());

    if (al::updateNerveState(this)) {
        if (mStateOption->isChangeLanguage() || mStateOption->get180()) {
            kill();
        } else {
            mSelectParts->appearWait();
            mFooterParts->tryChangeTextFade(
                al::getSystemMessageString(mMenuGuide, "Footer", "MenuMessage_Footer"));

            if (al::isNerve(this, &NrvStageSceneStatePauseMenu.OptionFromHelp))
                al::setNerve(this, &NrvStageSceneStatePauseMenu.StartHelpFromOption);
            else
                al::setNerve(this, &NrvStageSceneStatePauseMenu.Wait);
        }
    }
}

void StageSceneStatePauseMenu::exeSave() {
    al::updateKitListPrev(getHost());
    rs::requestGraphicsPresetAndCubeMapPause(getHost());
    al::updateKitList(getHost(), "２Ｄ（ポーズ無視）");
    al::updateKitListPost(getHost());
    if (al::isFirstStep(this))
        SaveDataAccessFunction::startSaveDataWriteWithWindow(mGameDataHolder);
    if (SaveDataAccessFunction::updateSaveDataAccess(mGameDataHolder, false)) {
        mSelectParts->appearWait();
        al::setNerve(this, &NrvStageSceneStatePauseMenu.Wait);
    }
}

void StageSceneStatePauseMenu::exeConfirmNewGame() {}

void StageSceneStatePauseMenu::exeNotExistEmptyFile() {}

void StageSceneStatePauseMenu::startPauseCamera() {
    al::startCameraPause(mPauseCameraCtrl);
}

void StageSceneStatePauseMenu::setAfterTitle() {
    mIsNormal = false;
    mSelectParts->appear(5);
    mSelectParts->setMainMenu(true);
    mSelectParts->setSelectMessage(
        0, al::getSystemMessageString(mMenuLayout, "MenuMessage", "StartGame"));
    mSelectParts->setSelectMessage(
        1, al::getSystemMessageString(mMenuLayout, "MenuMessage", "StartGameSeparatePlay"));
    mSelectParts->setSelectMessage(
        2, al::getSystemMessageString(mMenuLayout, "MenuMessage", "NewGame"));
}
