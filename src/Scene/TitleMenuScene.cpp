#include "Scene/TitleMenuScene.h"

#include <common/aglDrawContext.h>
#include <common/aglRenderBuffer.h>
#include <gfx/seadFrameBuffer.h>
#include <gfx/seadViewport.h>

#include "Demo/ProjectDemoDirector.h"
#include "Library/Area/AreaObjDirector.h"
#include "Library/Audio/AudioDirectorInitInfo.h"
#include "Library/Audio/System/AudioKeeperFunction.h"
#include "Library/Base/StringUtil.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Controller/InputFunction.h"
#include "Library/Controller/PadRumbleDirector.h"
#include "Library/Draw/ChromakeyDrawer.h"
#include "Library/Draw/GraphicsFunction.h"
#include "Library/Draw/GraphicsSystemInfo.h"
#include "Library/Draw/ViewRenderer.h"
#include "Library/Layout/LayoutActionFunction.h"
#include "Library/Layout/LayoutActorUtil.h"
#include "Library/Layout/LayoutInitInfo.h"
#include "Library/Layout/LayoutUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/LiveActor/LiveActorKit.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementInfo.h"
#include "Library/Play/Layout/SimpleLayoutAppearWaitEnd.h"
#include "Library/Play/Layout/WindowConfirm.h"
#include "Library/Play/Layout/WipeSimple.h"
#include "Library/Player/PlayerHolder.h"
#include "Library/Projection/Projection.h"
#include "Library/Scene/Scene.h"
#include "Library/Scene/SceneObjUtil.h"
#include "Library/Scene/SceneUtil.h"
#include "Library/Screen/ScreenFunction.h"
#include "Library/Se/SeFunction.h"

#include "Layout/BootLayout.h"
#include "Scene/ProjectActorFactory.h"
#include "Scene/ProjectAreaFactory.h"
#include "Scene/ProjectCameraPoserFactory.h"
#include "Scene/SceneObjFactory.h"
#include "Scene/StageScene.h"
#include "Scene/StageSceneStatePauseMenu.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolder.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/SpecialBuildUtil.h"
#include "Util/StageLayoutFunction.h"

namespace {
NERVE_IMPL(TitleMenuScene, Appear);
NERVE_IMPL(TitleMenuScene, LoadAppear);
NERVE_IMPL(TitleMenuScene, Menu);
NERVE_IMPL(TitleMenuScene, LoadWait);
NERVE_IMPL(TitleMenuScene, Wipe);

NERVE_MAKE(TitleMenuScene, LoadWait);
NERVES_MAKE_STRUCT(TitleMenuScene, Appear, LoadAppear, Menu, Wipe);
}  // namespace

TitleMenuScene::TitleMenuScene() : al::Scene("TitleMenuScene") {}

TitleMenuScene::~TitleMenuScene() {
    // Application::instance()->mFramework->field_268 = true;
    mChromakeyDrawer->finalize();
    delete this;
}

void TitleMenuScene::init(al::SceneInitInfo const& info) {
    initDrawSystemInfo(info);
    initAndLoadStageResource("SandWorldHomeStage", 1);
    al::SceneObjHolder* sceneObjHolder = SceneObjFactory::createSceneObjHolder();
    initSceneObjHolder(sceneObjHolder);

    al::setSceneObj(
        this, GameDataFunction::getGameDataHolder((al::GameDataHolderBase*)info.gameDataHolder),
        SceneObjID_GameDataHolder);

    initSceneStopCtrl();

    al::GraphicsInitArg graphicsInitArg(al::getSceneDrawContext(this),
                                        al::getSceneFrameBufferConsole(this));
    graphicsInitArg.dword_3c = true;
    graphicsInitArg.dword_10 = 1 << graphicsInitArg.mAtmosScatterViewNum;
    graphicsInitArg.field_6 = true;

    initLiveActorKitWithGraphics(graphicsInitArg, info, 1024, 1, 1);
    al::initGraphicsSystemInfo(this, "SandWorldHomeStage", 1);

    getLiveActorKit()->getGraphicsSystemInfo()->setField2F4(2);

    getLiveActorKit()->initHitSensorDirector();

    getLiveActorKit()->setDemoDirector(new ProjectDemoDirector(
        getLiveActorKit()->getPlayerHolder(), getLiveActorKit()->getGraphicsSystemInfo()));

    al::AudioDirectorInitInfo audioDirectorInitInfo;
    al::initAudioDirector3D(this, info, audioDirectorInitInfo);

    al::initSceneAudioKeeper(this, info, nullptr);
    alAudioSystemFunction::disableAudioMaximizer(info.gameSysInfo);

    ProjectCameraPoserFactory* projectCameraPoserFactory = new ProjectCameraPoserFactory();
    al::initCameraDirectorWithoutStageResource(this, projectCameraPoserFactory);

    al::setCameraAspect(this, al::getSceneFrameBufferMainAspect(this), -1.0f);

    al::AreaObjDirector* areaObjDirector = getLiveActorKit()->getAreaObjDirector();
    ProjectAreaFactory* projectAreaFactory = new ProjectAreaFactory();
    areaObjDirector->init(projectAreaFactory);

    al::initPadRumble(this, info);

    initLayoutKit(info);
    al::LayoutInitInfo layoutInitInfo;
    al::initLayoutInitInfo(&layoutInitInfo, this, info);

    al::PlacementInfo placementInfo;
    al::ActorInitInfo actorInitInfo;

    ProjectActorFactory* projectActorFactory = new ProjectActorFactory();

    al::initActorInitInfo(&actorInitInfo, this, &placementInfo, &layoutInitInfo,
                          projectActorFactory, nullptr,
                          (al::GameDataHolderBase*)info.gameDataHolder);

    initNerve(&NrvTitleMenuScene.Appear, 1);

    if (rs::isModeE3LiveRom())
        al::setNerve(this, &NrvTitleMenuScene.LoadAppear);

    mBootLayout = new BootLayout(layoutInitInfo);

    if (!rs::isModeE3LiveRom())
        mBootLayout->startWipe();

    mFadeBlack = new al::WipeSimple("フェード", "FadeBlack", layoutInitInfo, nullptr);
    mWindowConfirm = new al::WindowConfirm(layoutInitInfo, "WindowConfirm", "WindowConfirm");
    mMenuLayout =
        new al::SimpleLayoutAppearWaitEnd("ポーズメニュー", "Menu", layoutInitInfo, nullptr, false);
    mMenuLayout->kill();
    mLayoutContinueLoading = new al::SimpleLayoutAppearWaitEnd("ロード", "ContinueLoading",
                                                               layoutInitInfo, nullptr, false);
    mLayoutContinueLoading->kill();
    mLayoutParBG = new al::SimpleLayoutAppearWaitEnd(mLayoutContinueLoading, "ロード[BG]", "ParBG",
                                                     layoutInitInfo, nullptr);

    mStatePauseMenu = new StageSceneStatePauseMenu(
        "ポーズメニュー", this, mMenuLayout, (GameDataHolder*)info.gameDataHolder, info,
        actorInitInfo, layoutInitInfo, mWindowConfirm, nullptr, true, nullptr);

    al::initNerveState(this, mStatePauseMenu, &NrvTitleMenuScene.Menu, "ポーズメニュー");

    alPlayerFunction::registerPlayer(
        mStatePauseMenu->getMarioActor(),
        al::createPadRumbleKeeper(mStatePauseMenu->getMarioActor(), al::getMainControllerPort()));

    mChromakeyDrawer = new al::ChromakeyDrawer(getLiveActorKit()->getGraphicsSystemInfo(),
                                               al::getSceneExecuteDirector(this),
                                               getLiveActorKit()->getEffectSystem());

    endInit(actorInitInfo);
}

void TitleMenuScene::appear() {
    al::Scene::appear();
    // Application::instance()->mFramework->field_268 = false;
}

void TitleMenuScene::control() {}

void TitleMenuScene::drawMain() const {
    agl::DrawContext* drawContext = al::getSceneDrawContext(this);
    agl::RenderBuffer* frameBuffer = al::getSceneFrameBufferMain(this);

    sead::Viewport viewport(*frameBuffer);
    viewport.apply(drawContext, *frameBuffer);

    const sead::LookAtCamera& lookAtCamera = al::getLookAtCamera(this, 0);
    const al::Projection& projection = al::getProjection(this, 0);
    if (al::isLessEqualStep(this, 1))
        getLiveActorKit()->getGraphicsSystemInfo()->getViewRenderer()->drawView(
            0, getDrawSystemInfo(), projection, lookAtCamera, frameBuffer, viewport, true, false,
            false);

    viewport.apply(drawContext, *frameBuffer);
    frameBuffer->bind(drawContext);
    frameBuffer->clear(drawContext, 3, sead::Color4f::cBlack, al::getDepthClearValue(), 0);

    al::drawKit(this, "２Ｄバック（メイン画面）");
    mChromakeyDrawer->setPhysicalArea(frameBuffer->getPhysicalArea().getMax() -
                                      frameBuffer->getPhysicalArea().getMin());
    mChromakeyDrawer->drawChromakey(drawContext, projection, lookAtCamera, frameBuffer, viewport,
                                    "３Ｄ（クロマキーキャラクター）",
                                    "Ｚプリパス[ＮＰＣクロマキー]", nullptr);

    al::setRenderBuffer(getLayoutKit(), frameBuffer);

    sead::Viewport layoutViewport(0.0f, 0.0f, al::getLayoutDisplayWidth(),
                                  al::getLayoutDisplayHeight());
    layoutViewport.apply(drawContext, *frameBuffer);
    frameBuffer->bind(drawContext);

    al::drawKit(this, "２Ｄベース（メイン画面）");
    al::drawKit(this, "２Ｄオーバー（メイン画面）");
}

bool TitleMenuScene::isEnableKill() const {
    return al::isNerve(this, &LoadWait) && (al::isGreaterEqualStep(this, 140)) &&
           mEnableKillStep > 15;
}

bool TitleMenuScene::isChangeLanguage() const {
    return mStatePauseMenu->isChangeLanguage();
}

const char* TitleMenuScene::getLanguage() const {
    return mStatePauseMenu->getLanguage();
}

bool TitleMenuScene::isNewGame() const {
    if (mStatePauseMenu->isLoadData() &&
        GameDataFunction::isNewSaveDataByFileId(mMenuLayout, mStatePauseMenu->getSelectedFileId()))
        return true;
    else
        return mStatePauseMenu->isNewGame();
}

void TitleMenuScene::startLoadDirect(bool isGameLoad) {
    setScenario();
    mBootLayout->kill();
    mLayoutContinueLoading->startWait();

    if (isGameLoad)
        al::startAction(mLayoutContinueLoading, "GameLoad", "Type");
    else
        al::startAction(mLayoutContinueLoading, "TitleLoad", "Type");

    al::startAction(mLayoutContinueLoading, "Loop", "Loop");
    mLayoutParBG->startWait();
    al::setNerve(this, &LoadWait);
}

void TitleMenuScene::setScenario() {
    al::StringTmp<256> str1;
    al::StringTmp<256> str2;
    bool isStageMessage = false;

    if (rs::tryGetMapMainScenarioLabel(&str1, &str2, &isStageMessage, this)) {
        if (isStageMessage) {
            if (rs::trySetPaneStageMessageIfExist(mLayoutContinueLoading, "TxtScenario",
                                                  str1.cstr(), str2.cstr()))
                return;
        } else {
            if (rs::trySetPaneSystemMessageIfExist(mLayoutContinueLoading, "TxtScenario",
                                                   str2.cstr(), str1.cstr()))
                return;
        }
    }

    al::hidePane(mLayoutContinueLoading, "TxtGoal");
    al::hidePane(mLayoutContinueLoading, "TxtScenario");
}

void TitleMenuScene::setLoadPercent(f32 percent) {
    if (!mIsCancelLoadResource)
        mLoadPercent = percent;
}

bool TitleMenuScene::isCancelLoadWorldResource() const {
    return mIsCancelLoadResource;
}

void TitleMenuScene::startLoadWorldResource() {
    mIsCancelLoadResource = false;
    mLoadPercent = 0.0f;
    float_11c = 0.0f;
}

void TitleMenuScene::exeAppear() {
    if (al::isFirstStep(this)) {
        al::startSe(mBootLayout, "TitleCall");
        mStatePauseMenu->startPauseCamera();
    }
    if (al::isStep(this, 2)) {
        mMenuLayout->startWait();
        al::startAction(mMenuLayout, "Boot", "Boot");
        mStatePauseMenu->setAfterTitle();
    }
    if (!rs::isModeE3LiveRom() && al::isStep(this, 150)) {
        mStatePauseMenu->appearMarioModel();
        mStatePauseMenu->startActionMario("PauseMenuStart");
    }

    al::updateKitListPrev(this);
    al::updateKitList(this, "カメラ");
    rs::requestGraphicsPresetAndCubeMapPause(this);
    al::updateKitList(this, "デモ");
    al::updateKitList(this, "２Ｄ（ポーズ無視）");
    al::updateKitListPost(this);
    al::updateKitListPostOnNerveEnd(this);

    if (al::isGreaterEqualStep(this, 3) && al::isActionEnd(mMenuLayout, "Boot") &&
        al::isActionEnd(mStatePauseMenu->getMarioActor()))
        al::setNerve(this, &NrvTitleMenuScene.Menu);
}

void TitleMenuScene::exeMenu() {
    if (rs::isModeE3LiveRom()) {
        al::setNerve(this, &NrvTitleMenuScene.LoadAppear);
        return;
    }

    if (al::isFirstStep(this)) {
        mStatePauseMenu->startActionMario("PauseMenuContinue");
        mStatePauseMenu->startAfterTitle();
    }

    if (al::updateNerveState(this)) {
        setScenario();
        mLayoutContinueLoading->appear();
        al::startAction(mLayoutContinueLoading, "Loop", "Loop");
        mLayoutParBG->startWait();

        al::setActionFrame(mLayoutParBG, mBootLayout->getBgFrame(), nullptr);

        al::startAction(mLayoutContinueLoading, "TitleLoad", "Type");
        al::updateKitListPostOnNerveEnd(this);

        al::setNerve(this, &NrvTitleMenuScene.LoadAppear);
        return;
    }

    if (mStatePauseMenu->isModeSelectEnd() || mStatePauseMenu->isLoadData()) {
        mStatePauseMenu->kill();
        al::updateKitListPostOnNerveEnd(this);
        al::setNerve(this, &NrvTitleMenuScene.Wipe);
        return;
    }

    al::updateKitListPostOnNerveEnd(this);
}

void TitleMenuScene::exeWipe() {
    if (al::isFirstStep(this))
        mFadeBlack->startClose(-1);
    al::updateKit(this);
    al::updateGraphicsPrev(this);

    if (!mFadeBlack->isCloseEnd())
        return;

    mStatePauseMenu->killAllOptionLayout();
    mMenuLayout->kill();
    mFadeBlack->startOpen(-1);
    mWindowConfirm->kill();
    if (mStatePauseMenu->isLoadData()) {
        mIsCancelLoadResource = true;
        mLoadPercent = 0.0f;
        float_11c = 0.0f;
        GameDataHolderAccessor(this).getHolder()->receiveSetPlayingFileIdMsg();

        s32 fileId = GameDataHolderAccessor(this).getHolder()->getPlayingFileId();
        GameDataHolderAccessor(this).getHolder()->setPlayingFileId(fileId);

        GameDataHolderAccessor(this).getHolder()->setRequireSave();
    }

    setScenario();
    mLayoutContinueLoading->startWait();
    mLayoutParBG->startWait();
    al::setActionFrame(mLayoutParBG, mBootLayout->getBgFrame(), nullptr);
    al::startAction(mLayoutContinueLoading, "Loop", "Loop");

    al::setNerve(this, &LoadWait);
}

void TitleMenuScene::exeLoadAppear() {
    al::updateKit(this);
    al::updateGraphicsPrev(this);
    if (al::isFirstStep(this))
        al::setActionFrame(mLayoutParBG, mBootLayout->getBgFrame(), nullptr);
    if (mLayoutContinueLoading->isWait())
        al::setNerve(this, &LoadWait);
}

void TitleMenuScene::exeLoadWait() {
    if (al::isFirstStep(this)) {
        GameDataHolderAccessor accessor(this);
        if (GameDataFunction::isPayShineAllInAllWorld(accessor)) {
            al::startAction(mLayoutContinueLoading, "HomeComplete", "HomeType");
        } else {
            GameDataHolderAccessor accessor(this);
            if (GameDataFunction::isGameClear(accessor))
                al::startAction(mLayoutContinueLoading, "HomeClear", "HomeType");
            else
                al::startAction(mLayoutContinueLoading, "HomeNormal", "HomeType");
        }
    }

    f32 percent;
    if (!al::isGreaterEqualStep(this, 140))
        percent = al::lerpValue(0.0f, mLoadPercent, al::calcNerveRate(this, 140));
    else
        percent = mLoadPercent;
    float_11c = percent;

    if (percent > 100.0f)
        ++mEnableKillStep;

    al::startFreezeAction(mLayoutContinueLoading, "ProgressBar", std::min(percent, 100.0f),
                          "ProgressBar");
    al::updateKit(this);
    al::updateGraphicsPrev(this);
}
