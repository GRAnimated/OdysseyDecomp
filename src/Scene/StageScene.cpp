#include "Scene/StageScene.h"

#include "Library/Area/AreaObj.h"
#include "Library/Area/AreaObjDirector.h"
#include "Library/Area/AreaObjGroup.h"
#include "Library/Area/TrafficAreaDirector.h"
#include "Library/Scene/DemoDirector.h"
#include "Library/Audio/AudioDirector.h"
#include "Library/Audio/AudioDirectorInitInfo.h"
#include "Library/Audio/AudioFunction.h"
#include "Library/Audio/System/AudioKeeperFunction.h"
#include "Library/Audio/System/SimpleAudioUser.h"
#include "Library/Bgm/BgmLineFunction.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Controller/InputFunction.h"
#include "Library/Controller/PadRumbleDirector.h"
#include "Project/Clipping/ClippingDirector.h"
#include "Library/Demo/EventFlowExecutorHolder.h"
#include "Library/Demo/DemoSyncedEventFunction.h"
#include "Library/Demo/DemoSyncedEventKeeper.h"
#include "Library/Draw/ChromakeyDrawer.h"
#include "Library/Draw/GraphicsFunction.h"
#include "Library/Draw/GraphicsSystemInfo.h"
#include "Library/Effect/EffectEnvUtil.h"
#include "Library/Effect/FootPrintServer.h"
#include "Library/Execute/ExecuteUtil.h"
#include "Library/Framework/GameFrameworkNx.h"
#include "Library/Layout/LayoutInitInfo.h"
#include "Library/Layout/LayoutTextureRenderer.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/LiveActorKit.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/ParabolicPath.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Network/NetworkSystem.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementId.h"
#include "Library/Placement/PlacementInfo.h"
#include "Library/Base/StringUtil.h"
#include "Library/Play/Layout/SimpleLayoutAppearWaitEnd.h"
#include "Library/Play/Layout/WipeHolder.h"
#include "Library/Play/Layout/WipeSimple.h"
#include "Library/Play/Layout/WindowConfirm.h"
#include "Library/Player/PlayerUtil.h"
#include "Library/Resource/ResourceFunction.h"
#include "Library/Scene/SceneObjUtil.h"
#include "Library/Scene/SceneUtil.h"
#include "Library/Se/SeDirector.h"
#include "Library/System/GameSystemInfo.h"
#include "Library/Thread/FunctorV0M.h"

#include "Amiibo/AmiiboNpcDirector.h"
#include "Amiibo/HelpAmiiboDirector.h"
#include "Boss/GiantWanderBoss.h"
#include "Demo/ProjectDemoDirector.h"
#include "Layout/CapMessageMoonNotifier.h"
#include "Layout/CinemaCaption.h"
#include "MapObj/Pyramid.h"
#include "MapObj/PlayerStartInfo.h"
#include "Layout/CollectBgmRegister.h"
#include "Layout/ControllerGuideSnapShotCtrl.h"
#include "Layout/InputSeparator.h"
#include "Layout/KidsModeLayoutAccessor.h"
#include "Layout/LocationNameCtrl.h"
#include "Layout/MapLayout.h"
#include "Layout/MiniGameMenu.h"
#include "Layout/PlayGuideSkip.h"
#include "Layout/StageSceneLayout.h"
#include "MapObj/RouteGuideDirector.h"
#include "Npc/CapManHeroDemoUtil.h"
#include "Npc/EventDemoCtrl.h"
#include "Npc/EventFlowSceneExecuteCtrl.h"
#include "Npc/NpcEventCtrlInfo.h"
#include "Npc/NpcEventDirector.h"
#include "Player/PlayerCameraTarget.h"
#include "Player/PlayerActorBase.h"
#include "Player/PlayerFactory.h"
#include "Player/PlayerInitInfo.h"
#include "Player/ProjectCameraInput.h"
#include "Scene/BgmAnimeSyncDirector.h"
#include "Scene/BirdGatheringSpotDirector.h"
#include "Scene/CheckpointFlagWatcher.h"
#include "Scene/CollectBgmPlayer.h"
#include "Scene/DemoSoundSynchronizer.h"
#include "Scene/InformationWindowDirector.h"
#include "Scene/OpeningStageStartDemo.h"
#include "Scene/PlayerStartInfoHolder.h"
#include "Scene/ProjectActorFactory.h"
#include "Scene/ProjectAreaFactory.h"
#include "Scene/ProjectCameraPoserFactory.h"
#include "Scene/ProjectItemDirector.h"
#include "Scene/ProjectSeNamedList.h"
#include "Scene/QuestInfoHolder.h"
#include "Scene/SafetyPointRecoveryCameraWatcher.h"
#include "Scene/ScenarioStartCameraHolder.h"
#include "Scene/SceneAudioSystemPauseController.h"
#include "Scene/SceneObjFactory.h"
#include "Scene/StageSceneStateCarryMeat.h"
#include "Scene/StageSceneStateCheckpointWarp.h"
#include "Scene/StageSceneStateCloset.h"
#include "Scene/StageSceneStateCollectBgm.h"
#include "Scene/StageSceneStateCollectionList.h"
#include "Scene/StageSceneStateGetLifeMaxUpItem.h"
#include "Scene/StageSceneStateGetShine.h"
#include "Scene/StageSceneStateGetShineGrand.h"
#include "Scene/StageSceneStateGetShineMain.h"
#include "Scene/StageSceneStateGetShineMainLast.h"
#include "Scene/StageSceneStateGetShineMainSandWorld.h"
#include "Scene/StageSceneStateGetShineMainWaterfallWorld.h"
#include "Scene/StageSceneStateGetShineMainWithCageShine.h"
#include "Scene/StageSceneStateMiniGameRanking.h"
#include "Scene/StageSceneStateMiss.h"
#include "Scene/StageSceneStatePauseMenu.h"
#include "Scene/StageSceneStateRadicon.h"
#include "Scene/StageSceneStateRaceManRace.h"
#include "Scene/StageSceneStateRecoverLife.h"
#include "Scene/StageSceneStateScenarioCamera.h"
#include "Scene/StageSceneStateShop.h"
#include "Scene/StageSceneStateSkipDemo.h"
#include "Scene/StageSceneStateSnapShot.h"
#include "Scene/StageSceneStateTalk.h"
#include "Scene/StageSceneStateTimeBalloon.h"
#include "Scene/StageSceneStateTitleLogo.h"
#include "Scene/TimeBalloonDirector.h"
#include "Scene/TimeBalloonSequenceInfo.h"
#include "Scene/StageSceneStateWarp.h"
#include "Scene/StageSceneStateWorldIntroCamera.h"
#include "Scene/StageSceneStateWorldMap.h"
#include "Scene/StageSceneStateYukimaruRace.h"
#include "Scene/StageSceneStateYukimaruRaceTutorial.h"
#include "Scene/SwitchKeepOn2DAreaGroup.h"
#include "Scene/SwitchKeepOn2DExceptDokanInAreaGroup.h"
#include "Scene/SwitchKeepOnIgnoreOffAreaTarget.h"
#include "Scene/SwitchKeepOnPlayerInWaterAreaGroup.h"
#include "Scene/SwitchOn2DAreaGroup.h"
#include "Scene/SwitchOnPlayerOnGroundAreaGroup.h"
#include "Scene/TemporaryScenarioCameraHolder.h"
#include "Sequence/GameSequenceInfo.h"
#include "System/Application.h"
#include "System/GameConfigData.h"
#include "System/GameDataFile.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolder.h"
#include "System/GameDataHolderAccessor.h"
#include "System/GameDataUtil.h"
#include "Util/AmiiboUtil.h"
#include "Util/CameraUtil.h"
#include "Util/CheckpointUtil.h"
#include "Util/DemoUtil.h"
#include "Util/PlayerUtil.h"
#include "Util/SpecialBuildUtil.h"
#include "Util/StageInputFunction.h"

namespace {

class ProjectTrafficAreaUpdateJudge : public al::ITrafficAreaUpdateJudge {
public:
    ProjectTrafficAreaUpdateJudge(al::DemoDirector* demoDirector)
        : mDemoDirector(demoDirector) {}
    bool judge() const override { return !mDemoDirector->isActiveDemo(); }

    al::DemoDirector* mDemoDirector;
};

NERVE_IMPL(StageScene, StartStageBgm);
NERVE_IMPL(StageScene, CollectBgm);
NERVE_IMPL(StageScene, CollectionList);
NERVE_IMPL(StageScene, MiniGameRanking);
NERVE_IMPL(StageScene, Shop);
NERVE_IMPL(StageScene, Closet);
NERVE_IMPL(StageScene, DemoShineGet);
NERVE_IMPL(StageScene, DemoTalk);
NERVE_IMPL(StageScene, Radicon);
NERVE_IMPL(StageScene, Warp);
NERVE_IMPL(StageScene, DemoGetLifeMaxUpItem);
NERVE_IMPL(StageScene, DemoWorldIntroCamera);
NERVE_IMPL_(StageScene, DemoWorldIntroCameraBeforeAppearElectricDemo, DemoWorldIntroCamera);
NERVE_IMPL(StageScene, SnapShot);
NERVE_IMPL(StageScene, DemoShineMainGet);
NERVE_IMPL(StageScene, DemoShineGrandGet);
NERVE_IMPL(StageScene, SkipDemo);
NERVE_IMPL(StageScene, Miss);
NERVE_IMPL(StageScene, DemoCarryMeat);
NERVE_IMPL(StageScene, RaceYukimaru);
NERVE_IMPL(StageScene, RaceYukimaruTutorial);
NERVE_IMPL(StageScene, RaceManRace);
NERVE_IMPL(StageScene, DemoTitleLogo);
NERVE_IMPL(StageScene, Pause);
NERVE_IMPL(StageScene, ArriveAtCheckpoint);
NERVE_IMPL(StageScene, TimeBalloon);
NERVE_IMPL(StageScene, DemoScenarioCamera);
NERVE_IMPL(StageScene, AppearFromWorldWarpHole);
NERVE_IMPL_(StageScene, DemoStageStartOpening, DemoStageStart);
NERVE_IMPL(StageScene, DemoStageStart);
NERVE_IMPL(StageScene, DemoPlayerDown);
NERVE_IMPL(StageScene, DemoStageStartCapManHeroTalk);
NERVE_IMPL_(StageScene, DemoCapManHeroTalkMoonRock, DemoStageStartCapManHeroTalk);
NERVE_IMPL(StageScene, DemoReturnToHome);
NERVE_IMPL(StageScene, DemoHackStart);
NERVE_IMPL(StageScene, Play);
NERVE_IMPL_(StageScene, DemoWithPlayerUseCoinNoFirst, DemoWithPlayerUseCoin);
NERVE_IMPL(StageScene, DemoDamageStopPlayer);
NERVE_IMPL(StageScene, DemoSceneStartPlayerWalk);
NERVE_IMPL(StageScene, DemoRiseMapParts);
NERVE_IMPL(StageScene, DemoShineChipComplete);
NERVE_IMPL(StageScene, DemoHackStartFirst);
NERVE_IMPL(StageScene, DemoNormalBazookaElectric);
NERVE_IMPL(StageScene, DemoWithPlayerUseCoin);
NERVE_IMPL(StageScene, DemoWithPlayer);
NERVE_IMPL(StageScene, DemoNormal);
NERVE_IMPL(StageScene, Amiibo);
NERVE_IMPL_(StageScene, DemoStageStartElectricDemo, DemoStageStart);
NERVE_IMPL(StageScene, WaitStartWarpForSession);
NERVE_IMPL(StageScene, DemoCountCoin);
NERVE_IMPL(StageScene, WarpToCheckpoint);
NERVE_IMPL(StageScene, DemoAppearFromHome);
NERVE_IMPL(StageScene, DemoRisePyramid);
NERVE_IMPL(StageScene, WaitWarpToCheckpoint);

NERVES_MAKE_STRUCT(StageScene, StartStageBgm, CollectBgm, CollectionList, MiniGameRanking, Shop,
                   Closet, DemoShineGet, DemoTalk, Radicon, Warp, DemoGetLifeMaxUpItem,
                   DemoWorldIntroCamera, DemoWorldIntroCameraBeforeAppearElectricDemo, SnapShot,
                   DemoShineMainGet, DemoShineGrandGet, SkipDemo, Miss, DemoCarryMeat, RaceYukimaru,
                   RaceYukimaruTutorial, RaceManRace, DemoTitleLogo, Pause, ArriveAtCheckpoint,
                   TimeBalloon, DemoScenarioCamera, AppearFromWorldWarpHole, DemoStageStartOpening,
                   DemoStageStart, DemoPlayerDown, DemoStageStartCapManHeroTalk,
                   DemoCapManHeroTalkMoonRock, DemoReturnToHome, DemoHackStart, Play,
                   DemoWithPlayerUseCoinNoFirst, DemoDamageStopPlayer, DemoSceneStartPlayerWalk,
                   DemoRiseMapParts, DemoShineChipComplete, DemoHackStartFirst,
                   DemoNormalBazookaElectric, DemoWithPlayerUseCoin, DemoWithPlayer, DemoNormal,
                   Amiibo, DemoStageStartElectricDemo, WaitStartWarpForSession, DemoCountCoin,
                   WarpToCheckpoint, DemoAppearFromHome, DemoRisePyramid, WaitWarpToCheckpoint)
}  // namespace

void StageScene::init(const al::SceneInitInfo& initInfo) {
    mStageName = initInfo.initStageName;
    mScenarioNo = initInfo.scenarioNo;
    initDrawSystemInfo(initInfo);
    al::initRandomSeedByString(mStageName.cstr());
    al::GameFrameworkNx* framework;
    u32 vBlankWaitInterval;
    if (rs::isModeE3MovieRom() || rs::isModeMovieRom()) {
        framework =
            sead::DynamicCast<al::GameFrameworkNx>(Application::instance()->getGameFramework());
        vBlankWaitInterval = 2;
    } else {
        framework =
            sead::DynamicCast<al::GameFrameworkNx>(Application::instance()->getGameFramework());
        vBlankWaitInterval = 1;
    }
    framework->setVBlankWaitInterval(vBlankWaitInterval);
    initLayoutKit(initInfo);
    al::addResourceCategory(sead::SafeString("シーン"), 0x200,
                            sead::HeapMgr::instance()->getCurrentHeap());
    al::SceneObjHolder* sceneObjHolder = SceneObjFactory::createSceneObjHolder();
    initSceneObjHolder(sceneObjHolder);
    al::createSceneObj(this, SceneObjID_KidsModeLayoutAccessor);
    al::createSceneObj(this, SceneObjID_GuidePosInfoHolder);
    al::createSceneObj(this, SceneObjID_SceneEventNotifier);
    initAndLoadStageResource(mStageName.cstr(), initInfo.scenarioNo);
    GameDataHolder* dataHolder = (GameDataHolder*)initInfo.gameDataHolder;
    mGameDataHolder = dataHolder;
    mTimeBalloonSequenceInfo = dataHolder->getTimeBalloonSequenceInfo();
    dataHolder->getQuestInfoHolder()->initSceneObjHolder(getSceneObjHolder());
    rs::tryStartRaceStage(this, mGameDataHolder);
    mGameDataHolder->startStage(mStageName.cstr(), initInfo.scenarioNo);
    mGameDataHolder->setIsWaitingForStageStart(false);
    al::setSceneObj(this, mGameDataHolder, SceneObjID_GameDataHolder);
    al::setSceneObj(this, mGameDataHolder->getQuestInfoHolder(), SceneObjID_QuestInfoHolder);
    al::createSceneObj(this, SceneObjID_PaintObjHolder);
    al::createSceneObj(this, SceneObjID_FukankunZoomObjHolder);
    al::createSceneObj(this, SceneObjID_alStageSyncCounter);
    al::createSceneObj(this, SceneObjID_GrowPlantDirector);
    al::createSceneObj(this, SceneObjID_CapManHeroDemoDirector);
    initScreenCoverCtrl();
    initSceneStopCtrl();
    al::GraphicsInitArg graphicsInitArg(al::getSceneDrawContext(this),
                                        al::getSceneFrameBufferConsole(this));
    al::PlacementInfo placementInfos;
    s32 placementInfoCount = 0;
    al::tryGetPlacementInfoAndCount(&placementInfos, &placementInfoCount,
                                    al::getStageInfoMap(this, 0), "ObjectList");
    bool foundMirror = false;
    if (placementInfoCount >= 1) {
        for (s32 i = 0; i < placementInfoCount; i++) {
            al::PlacementInfo placementInfo;
            al::getPlacementInfoByIndex(&placementInfo, placementInfos, i);
            foundMirror |= al::isObjectName(placementInfo, "Mirror");
        }
    }
    graphicsInitArg.atmosScatterViewNum = foundMirror;
    graphicsInitArg._3c = 2;
    graphicsInitArg._10 = 2 << (s32)foundMirror;
    initLiveActorKitWithGraphics(graphicsInitArg, initInfo, 5120, 4, 2);
    al::initGraphicsSystemInfo(this, mStageName.cstr(), mScenarioNo);
    if (rs::isModeJungleGymRom() || rs::isModeDiverRom() || rs::isModeE3MovieRom() ||
        rs::isModeMovieRom() || GameDataFunction::isHomeShipStage(mGameDataHolder))
        alGraphicsFunction::invalidateCameraBlurSystem(this);
    al::NetworkSystem* networkSystem = initInfo.gameSystemInfo->networkSystem;
    if (networkSystem) {
        al::DataStoreDirector* dataStoreDir = networkSystem->getDataStoreDirector();
        if (dataStoreDir)
            al::setSceneObj(this, dataStoreDir, SceneObjID_alDataStoreDirector);
        al::RankingDirector* rankingDir = networkSystem->getRankingDirector();
        if (rankingDir)
            al::setSceneObj(this, rankingDir, SceneObjID_alRankingDirector);
    }
    ProjectItemDirector* itemDir = new ProjectItemDirector();
    al::initItemDirector(this, itemDir);
    mProjectItemDirector = itemDir;
    al::initHitSensorDirector(this);
    ProjectDemoDirector* demoDir = new ProjectDemoDirector(
        al::getScenePlayerHolder(this), getLiveActorKit()->getGraphicsSystemInfo());
    al::initDemoDirector(this, demoDir);
    {
        al::AudioDirectorInitInfo audioDirInitInfo;
        al::GraphicsSystemInfo* graphicsSysInfo = getLiveActorKit()->getGraphicsSystemInfo();
        if (graphicsSysInfo)
            audioDirInitInfo.seDirectorInitInfo.occlusionCullingJudge =
                graphicsSysInfo->getOcclusionCullingJudge();
        al::initAudioDirector3D(this, initInfo, audioDirInitInfo);
    }
    alAudioSystemFunction::enableAudioMaximizer(initInfo.gameSystemInfo);
    mAudioSystemPauseController =
        new SceneAudioSystemPauseController(getAudioDirector());
    mDemoSoundSynchronizer = new DemoSoundSynchronizer(
        initInfo.gameSystemInfo->applicationMessageReciever, getAudioDirector());
    mStageStartAtmosSe = new al::SimpleAudioUser("StageStartAtmosSe", getAudioDirector());
    mSePlayArea = new al::SimpleAudioUser("SePlayArea", getAudioDirector());
    mSnapShotCameraCtrl = new al::SimpleAudioUser("SnapShotCameraCtrl", getAudioDirector());
    mSeNamedList = new ProjectSeNamedList();
    al::initSceneAudioKeeper(this, initInfo, "StageSceneDefault");
    getAudioDirector()->setPlayerHolder(al::getScenePlayerHolder(this));
    using StageSceneFunctor = al::FunctorV0M<StageScene*, void (StageScene::*)()>;
    al::setTriggerEventForStopAllBgm(this,
                                     StageSceneFunctor(this, &StageScene::stopCollectBgm));
    if (rs::isModeE3MovieRom()) {
        al::SeDirector* seDir = getAudioDirector()->getSeDirector();
        seDir->setMuteSeInPVList(mSeNamedList->getMuteSeInPVList());
        seDir->setMuteSeInPVListSize(mSeNamedList->getMuteSeInPVListSize());
    }
    if (rs::isModeMovieRom()) {
        al::SeDirector* seDir = getAudioDirector()->getSeDirector();
        seDir->setMuteSeInPVList(mSeNamedList->getMuteSeInPVList());
        seDir->setMuteSeInPVListSize(mSeNamedList->getMuteSeInPVListSize());
    }
    al::deactivateAudioEventController(this);
    BgmAnimeSyncDirector* bgmAnimeSyncDir = new BgmAnimeSyncDirector();
    mBgmAnimeSyncDirector = bgmAnimeSyncDir;
    al::setSceneObj(this, bgmAnimeSyncDir, SceneObjID_BgmAnimeSyncDirector);
    al::setSceneObj(this, mCollectBgmPlayer, SceneObjID_CollectBgmPlayer);
    if (getLiveActorKit()->getDemoDirector()) {
        al::DemoSyncedEventKeeper* demoSyncedEventKeeper =
            new al::DemoSyncedEventKeeper(getAudioDirector());
        mDemoSyncedEventKeeper = demoSyncedEventKeeper;
        alDemoSyncedEventFunction::tryCreateAndSetSeKeeper(demoSyncedEventKeeper,
                                                           getAudioDirector(), "StageScene");
    }
    al::initPadRumble(this, initInfo);
    al::initNatureDirector(this);
    alEffectEnvUtil::setNatureDirector(getLiveActorKit()->getEffectSystem(),
                                       getLiveActorKit()->getNatureDirector());
    ProjectActorFactory actorFactory;
    al::LayoutInitInfo layoutInitInfo;
    al::initLayoutInitInfo(&layoutInitInfo, this, initInfo);
    al::PlacementInfo placementInfo;
    al::ActorInitInfo actorInitInfo;
    al::initActorInitInfo(&actorInitInfo, this, &placementInfo, &layoutInitInfo, &actorFactory,
                          nullptr, mGameDataHolder);
    CapManHeroDemoUtil::initCapManHeroDemoDirector(this, actorInitInfo);
    al::LiveActor* checkpointWarpCapActor = new al::LiveActor("チェックポイントワープ用帽子");
    mCheckpointWarpCapActor = checkpointWarpCapActor;
    al::initActorWithArchiveName(checkpointWarpCapActor, actorInitInfo, "MarioCap",
                                 "CheckpointFlagWarp");
    mCheckpointWarpCapActor->makeActorDead();
    mCheckpointWarpParabolicPath = new al::ParabolicPath();
    mDemoShine = new al::LiveActor("デモ用シャイン");
    al::LiveActor* demoPowerStarActor = new al::LiveActor("デモ用パワースター");
    al::initChildActorWithArchiveNameNoPlacementInfo(demoPowerStarActor, actorInitInfo, "PowerStar",
                                                     "Demo");
    al::initChildActorWithArchiveNameNoPlacementInfo(mDemoShine, actorInitInfo, "PowerStar",
                                                     "Demo");
    mDemoShine->kill();
    demoPowerStarActor->kill();
    mDemoDotShine = new al::LiveActor("デモ用ドットシャイン");
    al::initChildActorWithArchiveNameNoPlacementInfo(mDemoDotShine, actorInitInfo, "ShineDot",
                                                     "Demo");
    mDemoDotShine->kill();
    mProjectItemDirector->initPlayerDeadCoin(mGameDataHolder, actorInitInfo);
    mGameDataHolder->setUnkNumber(0);
    mLayoutTextureRenderer = new al::LayoutTextureRenderer();
    al::setSceneObj(this, mLayoutTextureRenderer, SceneObjID_alLayoutTextureRenderer);
    al::createSceneObj(this, SceneObjID_HtmlViewerRequester);
    mHtmlViewer = initInfo.gameSystemInfo->htmlViewer;
    al::initCameraDirector(this, mStageName.cstr(), initInfo.scenarioNo,
                           new ProjectCameraPoserFactory());
    if (rs::isKidsMode(this))
        alCameraFunction::validateCameraAreaKids(this);
    al::initSceneCameraFovyDegree(this, 40.0f);
    al::setNearClipDistance(this, 25.0f, 0);
    al::setFarClipDistance(this, 500000.0f, 0);
    al::setCameraAspect(this, al::getSceneFrameBufferMainAspect(this),
                        al::getSceneFrameBufferMainAspect(this));
    al::initSnapShotCameraAudioKeeper(this, mSnapShotCameraCtrl);
    ProjectAreaFactory* areaFactory = new ProjectAreaFactory();
    al::initAreaObjDirector(this, areaFactory);
    EventFlowSceneExecuteCtrl* eventFlowExecCtrl = new EventFlowSceneExecuteCtrl();
    mNpcEventDirector =
        new NpcEventDirector(al::getScenePlayerHolder(this),
                             getLiveActorKit()->getCameraDirector(),
                             getLiveActorKit()->getCollisionDirector(),
                             initInfo.gameSystemInfo->messageSystem, eventFlowExecCtrl);
    mNpcEventDirector->init(actorInitInfo);
    al::setSceneObj(this, mNpcEventDirector->getNpcEventCtrlInfo(), SceneObjID_NpcEventCtrlInfo);
    al::setSceneObj(this, mNpcEventDirector->getEventDemoCtrl(), SceneObjID_EventDemoCtrl);
    al::createSceneObj(this, SceneObjID_TalkNpcParamHolder);
    al::createSceneObj(this, SceneObjID_TalkNpcSceneEventSwitcher);

    al::initPlacementAreaObj(this, actorInitInfo);

    if (rs::isKidsMode(this)) {
        auto* safetyWatcher = new SafetyPointRecoveryCameraWatcher(
            getLiveActorKit()->getCameraDirector(), al::getScenePlayerHolder(this),
            al::getSceneAreaObjDirector(this)->getAreaObjGroup("RecoveryTargetPosKidsArea"),
            al::getSceneAreaObjDirector(this)->getAreaObjGroup("ForceRecoveryKidsArea"));
        al::registerExecutorUser((al::IUseExecutor*)((char*)safetyWatcher + 24),
                                al::getSceneExecuteDirector(this), "フォロー");
    }

    al::initPlacementObjectMap(this, actorInitInfo, "NatureList");

    al::AreaObjDirector* areaObjDirector = al::getSceneAreaObjDirector(this);
    al::AreaObjGroup* trafficAreaGroup = areaObjDirector->getAreaObjGroup("TrafficArea");
    if (trafficAreaGroup) {
        auto* updateJudge =
            new ProjectTrafficAreaUpdateJudge(getLiveActorKit()->getDemoDirector());
        auto* trafficDir = new al::TrafficAreaDirector(trafficAreaGroup, updateJudge);
        al::registerExecutorUser(trafficDir, al::getSceneExecuteDirector(this), "フォロー");
        al::setSceneObj(this, trafficDir, SceneObjID_alTrafficAreaDirector);
    }

    const al::AreaObjGroup* birdAreaGroup =
        al::getSceneAreaObjDirector(this)->getAreaObjGroup("BirdGatheringSpotArea");
    if (birdAreaGroup) {
        auto* birdDir = new BirdGatheringSpotDirector();
        birdDir->init(actorInitInfo, birdAreaGroup, getLiveActorKit()->getCameraDirector(),
                      getSceneObjHolder(),
                      getLiveActorKit()->getClippingDirector()->getClippingJudge(),
                      mStageName.cstr());
        al::registerExecutorUser((al::IUseExecutor*)((char*)birdDir + 8),
                                al::getSceneExecuteDirector(this), "フォロー");
    }

    auto* infoWindowDir = new InformationWindowDirector(
        al::getSceneAreaObjDirector(this), getSceneObjHolder(), al::getScenePlayerHolder(this));
    infoWindowDir->init(actorInitInfo, layoutInitInfo);
    al::setSceneObj(this, infoWindowDir, SceneObjID_InformationWindowDirector);
    al::registerExecutorUser((al::IUseExecutor*)((char*)infoWindowDir + 8),
                            al::getSceneExecuteDirector(this), "フォロー");

    PlayerStartInfoHolder* playerStartInfoHolder =
        (PlayerStartInfoHolder*)al::createSceneObj(this, SceneObjID_PlayerStartInfoHolder);
    playerStartInfoHolder->init(getLiveActorKit()->getCameraDirector(), 128);

    al::AreaObjGroup* changeStageAreaGroup =
        al::getSceneAreaObjDirector(this)->getAreaObjGroup("ChangeStageArea");
    if (changeStageAreaGroup) {
        s32 areaCount = changeStageAreaGroup->getSize();
        if (areaCount >= 1) {
            for (s32 i = 0; i < areaCount; i++) {
                al::AreaObj* areaObj = changeStageAreaGroup->getAreaObj(i);
                const char* shineStageName = nullptr;
                const char* shineObjId = nullptr;
                s32 shineGotOnOff = 0;
                al::tryGetStringArg(&shineStageName, *areaObj->getPlacementInfo(),
                                    "ShineStageName");
                al::tryGetStringArg(&shineObjId, *areaObj->getPlacementInfo(), "ShineObjId");
                al::tryGetArg(&shineGotOnOff, *areaObj->getPlacementInfo(), "ShineGotOnOff");
                if (shineGotOnOff == 1) {
                    if (!GameDataFunction::isGotShine(mGameDataHolder, shineStageName,
                                                      shineObjId))
                        areaObj->invalidate();
                } else if (shineGotOnOff == 2) {
                    if (GameDataFunction::isGotShine(mGameDataHolder, shineStageName, shineObjId))
                        areaObj->invalidate();
                }
            }
        }
    }

    playerStartInfoHolder->registerChangeStageAreaGroup(changeStageAreaGroup, actorInitInfo);

    al::AreaObjGroup* warpAreaGroup =
        al::getSceneAreaObjDirector(this)->getAreaObjGroup("WarpArea");
    playerStartInfoHolder->registerWarpAreaGroup(warpAreaGroup, actorInitInfo);

    al::AreaObjGroup* restartAreaGroup =
        al::getSceneAreaObjDirector(this)->getAreaObjGroup("RestartArea");
    playerStartInfoHolder->registerRestartAreaGroup(restartAreaGroup);

    bool foundCactus = false;
    s32 stageInfoMapNum = al::getStageInfoMapNum(this);
    if (stageInfoMapNum >= 1) {
        for (s32 i = 0; i < stageInfoMapNum; i++) {
            const al::StageInfo* stageInfoMap = al::getStageInfoMap(this, i);
            al::PlacementInfo playerAffectPlacementInfo;
            s32 playerAffectCount = 0;
            al::tryGetPlacementInfoAndCount(&playerAffectPlacementInfo, &playerAffectCount,
                                            stageInfoMap, "PlayerAffectObjList");
            if (playerAffectCount >= 1) {
                for (s32 j = 0; j < playerAffectCount; j++) {
                    al::PlacementInfo pInfo;
                    al::getPlacementInfoByIndex(&pInfo, playerAffectPlacementInfo, j);
                    const char* objName = nullptr;
                    al::getObjectName(&objName, pInfo);
                    const char* modelName = nullptr;
                    alPlacementFunction::tryGetModelName(&modelName, pInfo);
                    if (al::isEqualString(objName, "HackMoveParts")) {
                        al::createPlacementActorFromFactory(actorInitInfo, &pInfo);
                        foundCactus |= al::isEqualString(modelName, "HackMoveCactus");
                    } else if (al::isEqualString(objName, "CactusMini") ||
                               al::isEqualString(objName, "Cactus")) {
                        al::createPlacementActorFromFactory(actorInitInfo, &pInfo);
                        foundCactus = true;
                    }
                }
            }
        }
    }

    al::LiveActor* pyramid = al::tryInitPlacementSingleObject(
        this, actorInitInfo, 0, "SceneWatchObjList", "SandWorldHomePyramidKai000");
    if (pyramid) {
        mPyramid = (Pyramid*)pyramid;
        mStateGetShineMainSandWorld =
            new StageSceneStateGetShineMainSandWorld("メインシャインゲットデモ[砂ワールド]", this, actorInitInfo);
    }

    al::initPlacementObjectMap(this, actorInitInfo, "RaceList");

    StageSceneStateGetShineMainWithCageShine* cageShineState =
        StageSceneStateGetShineMainWithCageShine::tryCreate(this, actorInitInfo);

    auto* checkpointFlagWatcher =
        new CheckpointFlagWatcher(getLiveActorKit()->getCameraDirector());
    checkpointFlagWatcher->initStageInfo(mStageName.cstr(), mScenarioNo);
    al::setSceneObj(this, checkpointFlagWatcher, SceneObjID_CheckpointFlagWatcher);
    al::registerExecutorUser((al::IUseExecutor*)((char*)checkpointFlagWatcher + 16),
                            al::getSceneExecuteDirector(this), "フォロー");

    al::initPlacementObjectMap(this, actorInitInfo, "CheckPointList");
    al::initPlacementObjectMap(this, actorInitInfo, "PlayerStartInfoList");

    mShoppingWatcherGroup =
        new al::DeriveActorGroup<ShoppingWatcher>("ショップ店員", 26);
    al::tryInitPlacementActorGroup(mShoppingWatcherGroup, this, actorInitInfo, 0,
                                   "SceneWatchObjList", "ShoppingWatcher");

    al::LiveActorGroup* doshiGroup = new al::LiveActorGroup("ドッシーグループ", 4);
    if (al::tryInitPlacementActorGroup(doshiGroup, this, actorInitInfo, 0, "SceneWatchObjList",
                                       "Doshi")) {
        for (s32 i = 0; i < doshiGroup->getActorCount(); i++) {
            al::LiveActor* doshiActor = doshiGroup->getActor(i);
            al::LiveActor* linked =
                *(al::LiveActor**)((char*)doshiActor + 0x108);
            if (linked)
                mShoppingWatcherGroup->registerActor(linked);
        }
    }

    al::LiveActor* collectionList = al::tryInitPlacementSingleObject(this, actorInitInfo, 0,
                                                                      "SceneWatchObjList",
                                                                      "CollectionList");
    if (collectionList)
        mCollectionList = (CollectionList*)collectionList;

    al::LiveActor* timeBalloonNpc = al::tryInitPlacementSingleObject(this, actorInitInfo, 0,
                                                                      "SceneWatchObjList",
                                                                      "TimeBalloonNpc");
    if (timeBalloonNpc)
        mTimeBalloonNpc = (TimeBalloonNpc*)timeBalloonNpc;

    mStateCloset = StageSceneStateCloset::tryCreate(this, actorInitInfo, layoutInitInfo);

    al::createSceneObj(this, SceneObjID_CapMessageDirector);
    auto* capMessageMoonNotifier = static_cast<CapMessageMoonNotifier*>(
        al::createSceneObj(this, SceneObjID_CapMessageMoonNotifier));
    capMessageMoonNotifier->initialize(al::getStageInfoMap(this, 0), actorInitInfo);

    auto* footPrintServer = new al::FootPrintServer(actorInitInfo, "FootPrint", 32);
    al::setSceneObj(this, (al::ISceneObj*)footPrintServer, SceneObjID_alFootPrintServer);

    al::PlacementInfo restartPlacementInfo;
    const char* restartPointId = GameDataFunction::tryGetRestartPointIdString(mGameDataHolder);
    const char* nextPlayerStartId = mGameDataHolder->getNextPlayerStartId();
    sead::FixedSafeString<256> playerStartId;
    const char* startId;
    if (restartPointId)
        startId = restartPointId;
    else
        startId = nextPlayerStartId;

    if (GameDataFunction::isWarpCheckpoint(mGameDataHolder)) {
        const char* checkpointStartId;
        if (restartPointId)
            checkpointStartId = restartPointId;
        else
            checkpointStartId = "Home";
        const al::PlacementInfo* checkpointRestartInfo =
            rs::tryFindCheckpointFlagPlayerRestartInfo(this, checkpointStartId);
        if (checkpointRestartInfo) {
            al::PlacementId placementIdForCheckpoint;
            al::tryGetPlacementId(&placementIdForCheckpoint, *checkpointRestartInfo);
            al::StringTmp<128> placementIdStr =
                al::makeStringPlacementId(&placementIdForCheckpoint);
            playerStartId.copy(placementIdStr);
        }
        startId = playerStartId.cstr();
    }

    s32 lastRaceRanking = GameDataFunction::getLastRaceRanking(GameDataHolderWriter(this));
    mGameDataHolder->reset_49();
    mGameDataHolder->reset_4a();

    bool restartPlacementInfoValid = restartPlacementInfo.getPlacementIter().isValid();
    const PlayerStartInfo* playerRestartInfo = nullptr;
    if (startId) {
        if (!restartPlacementInfoValid) {
            const PlayerStartInfo* foundInfo =
                playerStartInfoHolder->tryFindInitInfoByStartId(startId);
            playerRestartInfo = foundInfo;
            if (foundInfo) {
                const al::ByamlIter* iters = (const al::ByamlIter*)foundInfo;
                restartPlacementInfo.set(iters[0], iters[1]);
                al::CameraTicket* restartCamera =
                    *(al::CameraTicket**)((const char*)foundInfo + 32);
                if (lastRaceRanking <= 0 && restartCamera)
                    al::startCamera(this, restartCamera, -1);
                else
                    al::resetSceneInitEntranceCamera(this);
            }
        }
    }

    const char* checkpointWarpObjId =
        GameDataFunction::getCheckpointWarpObjId(mGameDataHolder);
    al::LiveActor* checkpointFlag = rs::tryFindCheckpointFlag(this, checkpointWarpObjId);
    bool isWarpCheckpoint = GameDataFunction::isWarpCheckpoint(mGameDataHolder);
    if (checkpointFlag && isWarpCheckpoint && al::isAlive(checkpointFlag))
        al::resetSceneInitEntranceCamera(this);

    const char* costumeTypeName =
        GameDataFunction::getCurrentCostumeTypeName(mGameDataHolder);
    const char* capTypeName = GameDataFunction::getCurrentCapTypeName(mGameDataHolder);
    if (costumeTypeName) {
        if (!al::isEqualString(costumeTypeName, ""))
            mCostumeName.format("%s", costumeTypeName);
    }
    if (capTypeName) {
        if (!al::isEqualString(capTypeName, ""))
            mCapTypeName.format("%s", capTypeName);
    }
    if (rs::isModeDiverOrJungleGymRom()) {
        mCostumeName.format("Mario");
        mCapTypeName.format("Mario");
    }

    PlayerFactory playerFactory;
    al::PlacementInfo playerListPlacementInfo;
    s32 playerListCount = 0;
    al::tryGetPlacementInfoAndCount(&playerListPlacementInfo, &playerListCount,
                                    al::getStageInfoMap(this), "PlayerList");
    if (playerListCount >= 1) {
        for (s32 i = 0; i < playerListCount; i++) {
            al::PlacementInfo playerPlacementInfo;
            al::getPlacementInfoByIndex(&playerPlacementInfo, playerListPlacementInfo, i);
            const char* changeStageId = nullptr;
            bool hasChangeStageId =
                al::tryGetStringArg(&changeStageId, playerPlacementInfo, "ChangeStageId");
            if (startId && !restartPlacementInfo.getPlacementIter().isValid() &&
                !hasChangeStageId == false &&
                al::isEqualString(changeStageId, startId)) {
                restartPlacementInfo.set(playerPlacementInfo.getPlacementIter(),
                                        playerPlacementInfo.getZoneIter());
            }

            struct {
                sead::Vector3f trans;
                sead::Quatf quat;
            } playerStartInfo;
            playerStartInfo.trans = sead::Vector3f::zero;
            playerStartInfo.quat.x = 0.0f;
            playerStartInfo.quat.y = 0.0f;
            playerStartInfo.quat.z = 0.0f;
            playerStartInfo.quat.w = 1.0f;
            if (playerRestartInfo) {
                playerStartInfo.trans = playerRestartInfo->trans;
                playerStartInfo.quat = playerRestartInfo->quat;
            } else {
                if (restartPlacementInfo.getPlacementIter().isValid()) {
                    al::getTrans(&playerStartInfo.trans, restartPlacementInfo);
                    al::getQuat(&playerStartInfo.quat, restartPlacementInfo);
                } else {
                    al::getTrans(&playerStartInfo.trans, playerPlacementInfo);
                    al::getQuat(&playerStartInfo.quat, playerPlacementInfo);
                }
            }

            al::ActorInitInfo playerActorInitInfo;
            playerActorInitInfo.initViewIdSelf(&playerPlacementInfo, actorInitInfo);

            PlayerInitInfo playerInitInfo;
            playerInitInfo.gamePadSystem = initInfo.gameSystemInfo->gamePadSystem;
            playerInitInfo.viewMtxPtr = al::getViewMtxPtr(this, 0);
            playerInitInfo.controllerPort = al::getMainControllerPort();
            playerInitInfo.costumeName = mCostumeName.cstr();
            playerInitInfo.capTypeName = mCapTypeName.cstr();
            playerInitInfo.trans = playerStartInfo.trans;
            playerInitInfo.quat = playerStartInfo.quat;
            playerInitInfo._44 = foundCactus;
            playerInitInfo._45 = (mStateCloset != nullptr);

            const char* costumeName = mCostumeName.cstr();
            if (*costumeName != sead::SafeStringBase<char>::cNullChar) {
                GameDataFunction::wearCostume(mGameDataHolder, costumeName);
            }
            const char* capTypeName2 = mCapTypeName.cstr();
            if (*capTypeName2 != sead::SafeStringBase<char>::cNullChar) {
                GameDataFunction::wearCap(mGameDataHolder, capTypeName2);
            }

            const char* objectName = nullptr;
            al::getObjectName(&objectName, playerActorInitInfo);
            const char* className = nullptr;
            al::getClassName(&className, playerActorInitInfo);
            al::LiveActor* player =
                playerFactory.createActor(playerActorInitInfo, className);

            static_cast<PlayerActorBase*>(player)->initPlayer(playerActorInitInfo,
                                                               playerInitInfo);

            s32 mainControllerPort = static_cast<PlayerActorBase*>(player)->getPortNo();
            al::PadRumbleKeeper* padRumbleKeeper =
                al::createPadRumbleKeeper(player, mainControllerPort);
            alPlayerFunction::registerPlayer(player, padRumbleKeeper);

            auto* cameraTarget = new PlayerCameraTarget(player);
            al::setCameraTarget(this, (al::CameraTargetBase*)cameraTarget);
            auto* cameraInput = new ProjectCameraInput(player);
            al::setCameraInput(this, (const al::ICameraInput*)cameraInput);

            if (al::isObjectName(playerActorInitInfo, "PlayerActorHakoniwa"))
                GameDataFunction::setStageHakoniwa(mGameDataHolder);
            if (al::isObjectName(playerActorInitInfo, "PlayerActor2D3D"))
                GameDataFunction::setStageHakoniwa(mGameDataHolder);

            if (!mCheckpointWarpArriveCamera)
                mCheckpointWarpArriveCamera = rs::initWarpCheckpointCamera(
                    this, mCheckpointWarpCapActor, actorInitInfo, "CheckpointWarpCamera",
                    &mCheckpointWarpTargetPos, &mCheckpointWarpParabolicPathPos, nullptr);
        }
    }

    al::initPlacementByStageInfo(al::getStageInfoMap(this), "SkyList", actorInitInfo);

    ScenarioCameraRelationInfo scenarioCameraBuffer[64];
    TemporaryScenarioCameraHolder tempScenarioCameraHolder(scenarioCameraBuffer, 64);
    mGameDataHolder->setTemporaryScenarioCameraHolder(&tempScenarioCameraHolder);

    al::initPlacementObjectMap(this, actorInitInfo, "ObjectList");
    al::initPlacementObjectDesign(this, actorInitInfo, "ObjectList");
    al::initPlacementObjectSound(this, actorInitInfo, "ObjectList");

    al::ISceneObj* npcEventCtrlInfoObj =
        al::getSceneObj(this, SceneObjID_NpcEventCtrlInfo);
    al::EventFlowExecutorHolder* eventFlowHolder =
        *(al::EventFlowExecutorHolder**)((char*)npcEventCtrlInfoObj + 40);
    eventFlowHolder->initAfterPlacement();

    mScenarioStartCameraHolder = new ScenarioStartCameraHolder();
    mScenarioStartCameraHolder->init(this, actorInitInfo);

    s32 demoStageInfoMapNum = al::getStageInfoMapNum(this);
    bool hasOpeningDemo = false;
    bool hasWaterfallStartDemo = false;
    if (demoStageInfoMapNum >= 1) {
        for (s32 i = 0; i < demoStageInfoMapNum; i++) {
            const al::StageInfo* demoStageInfoMap = al::getStageInfoMap(this, i);
            al::PlacementInfo demoPlacementInfo;
            s32 demoObjCount = 0;
            al::tryGetPlacementInfoAndCount(&demoPlacementInfo, &demoObjCount, demoStageInfoMap,
                                            "DemoObjList");
            if (demoObjCount >= 1) {
                for (s32 j = 0; j < demoObjCount; j++) {
                    al::PlacementInfo demoObjPlacementInfo;
                    al::getPlacementInfoByIndex(&demoObjPlacementInfo, demoPlacementInfo, j);
                    const char* demoObjName = nullptr;
                    al::getObjectName(&demoObjName, demoObjPlacementInfo);
                    if (al::isEqualString(demoObjName, "DemoRegister") ||
                        al::isEqualString(demoObjName, "WorldIntroCamera") ||
                        al::isEqualString(demoObjName, "RaceIntroCamera") ||
                        al::isEqualString(demoObjName, "RaceCountDownCamera"))
                        continue;
                    al::LiveActor* demoActor =
                        al::createPlacementActorFromFactory(actorInitInfo, &demoObjPlacementInfo);
                    if (al::isEqualString(demoObjName, "StageEventDemo") ||
                        al::isEqualString(demoObjName, "StageEventDemoNoSave")) {
                        if (((OpeningStageStartDemo*)demoActor)->isEnableStart()) {
                            mOpeningStageStartDemo = (OpeningStageStartDemo*)demoActor;
                            const char* demoStageName = nullptr;
                            al::getStringArg(&demoStageName, demoObjPlacementInfo,
                                             "DemoStageName");
                            hasWaterfallStartDemo |=
                                al::isEqualString(demoStageName,
                                                  "DemoStartWorldWaterfallStage");
                        }
                    } else if (al::isEqualString(demoObjName, "OpeningStageStartDemo")) {
                        mOpeningStageStartDemo = (OpeningStageStartDemo*)demoActor;
                        if (((OpeningStageStartDemo*)demoActor)->isEnableStart()) {
                            hasOpeningDemo = true;
                        } else {
                            hasOpeningDemo = false;
                            mOpeningStageStartDemo = nullptr;
                        }
                    } else if (!al::isEqualString(demoObjName, "StageTalkDemoNpcCap") &&
                               !al::isEqualString(demoObjName, "StartEntranceCamera")) {
                        al::isEqualString(demoObjName, "StageTalkDemoNpcCapMoonRock");
                    }
                }
            }
        }
    }

    if (GameDataFunction::isFindKoopa(mGameDataHolder) ||
        al::isEqualString(mStageName, "TestKitazonoKoopaLv0Stage")) {
        al::LiveActorGroup* actorGroup = actorInitInfo.allActorsGroup;
        for (s32 i = 0; i < actorGroup->getActorCount(); i++) {
            if (al::isEqualString(actorGroup->getActor(i)->getName(), "KoopaLv1")) {
                mKoopaLv1 = (KoopaLv1*)actorGroup->getActor(i);
                break;
            }
        }
    }

    QuestInfoHolder* questInfoHolder = rs::getQuestInfoHolder(this);
    s32 mainScenarioNo =
        mGameDataHolder->getGameDataFile()->getMainScenarioNoCurrent();
    questInfoHolder->initAfterPlacementQuestObj(mainScenarioNo);

    if (!rs::isModeE3Rom() && !rs::isModeE3LiveRom()) {
        mProjectNfpDirector = (ProjectNfpDirector*)initInfo.gameSystemInfo->nfpDirector;
        AmiiboFunction::tryCreateHelpAmiiboDirector(this);
        mHelpAmiiboDirector =
            (HelpAmiiboDirector*)al::getSceneObj(this, SceneObjID_HelpAmiiboDirector);
        if (mHelpAmiiboDirector)
            mHelpAmiiboDirector->init(mProjectNfpDirector, al::getScenePlayerHolder(this),
                                      getAudioDirector(), layoutInitInfo);
        AmiiboNpcDirector* amiiboNpcDirector =
            (AmiiboNpcDirector*)al::tryGetSceneObj(this, SceneObjID_AmiiboNpcDirector);
        if (amiiboNpcDirector)
            amiiboNpcDirector->init(mProjectNfpDirector, getAudioDirector());
    }

    mPlacementInfo = new al::PlacementInfo();

    if (!al::isPlayingBgm(mDemoShine, "CollectBgm")) {
        mCollectBgmPlayer->_10 = nullptr;
        mCollectBgmPlayer->_18 = nullptr;
    }

    mCollectBgmRegister =
        new CollectBgmRegister(getAudioDirector(), mGameDataHolder, mCollectBgmPlayer);

    mPlayGuideSkip = new PlayGuideSkip("PlayGuideSkip", layoutInitInfo);
    mCinemaCaption = new CinemaCaption(layoutInitInfo);

    mSceneLayout = new StageSceneLayout(
        "シーン情報", layoutInitInfo, al::getScenePlayerHolder(this),
        getLiveActorKit()->getGraphicsSystemInfo()->getSubCameraRenderer());

    mScenarioStartLayout = new al::SimpleLayoutAppearWaitEnd("シナリオ開始", "StartScenario",
                                                              layoutInitInfo, nullptr, false);
    mScenarioStartLayout->kill();

    mWorldStartCountryLayout = new al::SimpleLayoutAppearWaitEnd(
        "ワールド開始[国]", "StartWorld", layoutInitInfo, nullptr, false);
    mWorldStartCountryLayout->kill();

    mWorldStartRegionLayout = new al::SimpleLayoutAppearWaitEnd(
        "ワールド開始[地域]", "StartWorldRegion", layoutInitInfo, nullptr, false);
    mWorldStartRegionLayout->kill();

    mControllerGuideSnapshotCtrl = new ControllerGuideSnapShotCtrl(
        "スナップショットレイアウト更新", layoutInitInfo,
        getLiveActorKit()->getGraphicsSystemInfo()->getPostProcessingFilter());

    auto* inputSep = new InputSeparator(
        mWorldStartCountryLayout ? static_cast<const al::IUseSceneObjHolder*>(mWorldStartCountryLayout) : nullptr, false);
    mInputSeparator = inputSep;

    mLocationNameCtrl = new LocationNameCtrl(al::getSceneAreaObjDirector(this), mGameDataHolder,
                                             layoutInitInfo, al::getScenePlayerHolder(this));

    auto* windowConfirm =
        new al::WindowConfirm(layoutInitInfo, "WindowConfirm", "WindowConfirm");
    mWindowConfirm = windowConfirm;
    windowConfirm->kill();

    mMiniGameMenu = new MiniGameMenu(layoutInitInfo);

    mWipeSimple = new al::WipeSimple("丸ワイプ", "WipeCircle", layoutInitInfo, nullptr);

    if (!mWipeHolder) {
        auto* wipeHolder = new al::WipeHolder(6);
        mWipeHolder = wipeHolder;
        auto* wipeCircle =
            new al::WipeSimple("ワープ用ワイプ", "WipeCircle", layoutInitInfo, nullptr);
        wipeHolder->registerWipe("WipeCircle", wipeCircle);
        auto* fadeBlack =
            new al::WipeSimple("黒フェード", "FadeBlack", layoutInitInfo, nullptr);
        mWipeHolder->registerWipe("FadeBlack", fadeBlack);
        auto* fadeWhite =
            new al::WipeSimple("白フェード", "FadeWhite", layoutInitInfo, nullptr);
        mWipeHolder->registerWipe("FadeWhite", fadeWhite);
        auto* wipeMiss =
            new al::WipeSimple("ミスワイプ", "WipeMiss", layoutInitInfo, nullptr);
        mWipeHolder->registerWipe("WipeMiss", wipeMiss);
        auto* wipeSkip =
            new al::WipeSimple("スキップワイプ", "WipeSkip", layoutInitInfo, nullptr);
        mWipeHolder->registerWipe("WipeSkip", wipeSkip);
    }

    al::createSceneObj(this, SceneObjID_WipeHolderRequester);

    s32 currentWorldId = GameDataFunction::getCurrentWorldId(GameDataHolderAccessor(this));
    mMapLayout = new MapLayout(layoutInitInfo, al::getScenePlayerHolder(this), currentWorldId);
    al::setSceneObj(this, mMapLayout, SceneObjID_MapLayout);

    mPauseMenu = new al::SimpleLayoutAppearWaitEnd("PauseMenu", "Menu", layoutInitInfo, nullptr,
                                                    false);
    mPauseMenu->kill();

    mStateCollectBgm =
        StageSceneStateCollectBgm::tryCreate(this, actorInitInfo, layoutInitInfo, mCollectBgmPlayer);
    mStateMiniGameRanking =
        StageSceneStateMiniGameRanking::tryCreate(this, actorInitInfo, layoutInitInfo);

    mStateGetShine = new StageSceneStateGetShine(
        "シャインゲットデモ", this, mSceneLayout, &initInfo, actorInitInfo, layoutInitInfo, mDemoShine,
        mDemoDotShine, cageShineState, mGameDataHolder, mProjectItemDirector);

    auto* stateGetShineMain = new StageSceneStateGetShineMain(
        "メインシャインゲットデモ", this, mSceneLayout, &initInfo, actorInitInfo, layoutInitInfo,
        mDemoShine, mScenarioStartCameraHolder, mGameDataHolder);
    mStateGetShineMain = stateGetShineMain;

    auto* stateGetShineMainLast = new StageSceneStateGetShineMainLast(
        "メインシャインゲットデモ最後", this,
        ((StageSceneStateGetShineMain*)stateGetShineMain)->getDemoShineActor(),
        ((StageSceneStateGetShineMain*)stateGetShineMain)->getCameraTicket());

    mStateGetShineGrand = new StageSceneStateGetShineGrand(
        "グランドシャインゲットデモ", this, &initInfo, actorInitInfo, layoutInitInfo);

    mStateSkipDemo = new StageSceneStateSkipDemo("デモスキップ", this, mWindowConfirm, mWipeHolder,
                                                  mPlayGuideSkip, mAudioSystemPauseController,
                                                  mDemoSyncedEventKeeper);

    mCheckpointWarpArriveCamera = al::initDemoProgramableCamera(
        this, actorInitInfo, "CheckpointWarpArriveCamera", &mCheckpointWarpTargetPos,
        &mCheckpointWarpParabolicPathPos, nullptr);
    mStateCheckpointWarp = new StageSceneStateCheckpointWarp(
        "チェックポイントワープ到着デモ", this, mCheckpointWarpCapActor, mGameDataHolder,
        mCheckpointWarpArriveCamera, &mCheckpointWarpTargetPos, &mCheckpointWarpParabolicPathPos);

    mStateCarryMeat = new StageSceneStateCarryMeat("肉運びデモ", this);

    mStagePauseMenu = new StageSceneStatePauseMenu(
        "ポーズメニュー", this, mPauseMenu, mGameDataHolder, initInfo, actorInitInfo,
        layoutInitInfo, mWindowConfirm, mSceneLayout, false, mAudioSystemPauseController);

    mStateCollectionList = new StageSceneStateCollectionList(
        "コレクションリスト", this, layoutInitInfo, mMapLayout, mStateCollectBgm,
        mAudioSystemPauseController);

    mStateWorldMap = StageSceneStateWorldMap::tryCreate(this, initInfo, actorInitInfo,
                                                        layoutInitInfo, mStateCollectionList);
    mStateSkipDemo->setWorldMapState(mStateWorldMap);

    mStateMiss = new StageSceneStateMiss(
        "StageScene:ミス", this, layoutInitInfo, mGameDataHolder, mSceneLayout,
        getLiveActorKit()->getGraphicsSystemInfo()->getSubCameraRenderer());

    mStateShop = new StageSceneStateShop("ショップ", this, mSceneLayout);
    mStateShop->initAndSetting(mShoppingWatcherGroup, actorInitInfo);

    mStateSnapShot = new StageSceneStateSnapShot("スナップショット", this,
                                                  mControllerGuideSnapshotCtrl,
                                                  mAudioSystemPauseController, mInputSeparator,
                                                  mNpcEventDirector);
    mStateSnapShot->init();

    mStateYukimaruRace = StageSceneStateYukimaruRace::tryCreate(
        this, actorInitInfo, mSceneLayout, mAudioSystemPauseController, mStateSnapShot);
    mStateYukimaruRaceTutorial = StageSceneStateYukimaruRaceTutorial::tryCreate(
        this, actorInitInfo, layoutInitInfo, mGameDataHolder, mStateSnapShot);
    mStateTitleLogo = StageSceneStateTitleLogo::tryCreate(this, actorInitInfo, layoutInitInfo,
                                                          mWipeHolder, mGameDataHolder);

    mStateScenarioCamera = new StageSceneStateScenarioCamera(
        "シナリオ開始", this, mStageName.cstr(), mScenarioNo,
        ((StageSceneStateGetShineMain*)mStateGetShineMain)->getDemoShineActor());
    mStateScenarioCamera->init();
    mStateScenarioCamera->setStateSkipDemo(mStateSkipDemo);

    mStateRecoverLife = new StageSceneStateRecoverLife("ライフ回復", this, mSceneLayout);
    mStateRecoverLife->init();
    mStateGetShine->setLifeRecoverState(mStateRecoverLife);

    mStateTalk = new StageSceneStateTalk("会話", this, eventFlowExecCtrl, mSceneLayout,
                                          mCinemaCaption, mPlayGuideSkip, mStateMiniGameRanking,
                                          mStateSkipDemo, mStateWorldMap, mStateGetShine,
                                          mStateGetShineMain, mStateCollectionList, mGameDataHolder,
                                          mWipeSimple);

    mStateWarp = new StageSceneStateWarp("ワープ", this, mWipeSimple, mGameDataHolder,
                                         mLocationNameCtrl);
    mStateWarp->init();

    auto* stateGetLifeMaxUpItem =
        new StageSceneStateGetLifeMaxUpItem("最大ライフアップアイテム取得", this, mSceneLayout);
    stateGetLifeMaxUpItem->init();

    StageSceneStateGetShineMainWaterfallWorld* waterfallState =
        StageSceneStateGetShineMainWaterfallWorld::tryCreate(this, actorInitInfo);

    mStateRaceManRace = StageSceneStateRaceManRace::tryCreate(
        this, mGameDataHolder, mSceneLayout, mStateMiss, mStateCollectionList,
        stateGetLifeMaxUpItem, mStateWarp, actorInitInfo,
        mAudioSystemPauseController, mStateSnapShot);

    mStateWorldIntroCamera = new StageSceneStateWorldIntroCamera(
        "ワールド紹介カメラ", actorInitInfo, this, mWorldStartCountryLayout,
        mWorldStartRegionLayout, mStageStartAtmosSe, mStateSkipDemo);

    mStateRadicon = new StageSceneStateRadicon("ラジコン", this, mStateCollectionList,
                                               mStagePauseMenu, mStateSnapShot);

    if (mTimeBalloonNpc) {
        al::ISceneObj* timeBalloonDirObj =
            al::createSceneObj(this, SceneObjID_TimeBalloonDirector);
        mTimeBalloonDirector = timeBalloonDirObj
                                   ? (TimeBalloonDirector*)((char*)timeBalloonDirObj - 8)
                                   : nullptr;
        mTimeBalloonDirector->init(actorInitInfo, mTimeBalloonNpc, mTimeBalloonSequenceInfo, 0, 0,
                                   mSceneLayout);
        mTimeBalloonSequenceInfo->setAccessor(
            (al::LiveActor*)mTimeBalloonDirector->getAccessor());

        mStateTimeBalloon = new StageSceneStateTimeBalloon(
            this, mTimeBalloonDirector, mTimeBalloonSequenceInfo, mGameDataHolder, mSceneLayout,
            actorInitInfo, mTimeBalloonNpc, false, mMiniGameMenu,
            (void*)((char*)mStateCollectionList + 56), mStateWarp);
        mTimeBalloonSequenceInfo->addHioNode();
    }

    al::initSwitchAreaDirector(this, 3, 2);

    al::AreaObjDirector* switchAreaObjDir = al::getSceneAreaObjDirector(this);
    al::PlayerHolder* switchPlayerHolder = al::getScenePlayerHolder(this);

    auto* switchOnPlayerOnGround = SwitchOnPlayerOnGroundAreaGroup::tryCreate(
        "SwitchOnPlayerOnGroundArea", switchAreaObjDir, switchPlayerHolder);
    if (switchOnPlayerOnGround)
        al::registerSwitchOnAreaGroup(this, (al::SwitchOnAreaGroup*)switchOnPlayerOnGround);

    auto* switchOn2D =
        SwitchOn2DAreaGroup::tryCreate("SwitchOn2DArea", switchAreaObjDir, switchPlayerHolder);
    if (switchOn2D)
        al::registerSwitchOnAreaGroup(this, (al::SwitchOnAreaGroup*)switchOn2D);

    auto* switchKeepOn2D = SwitchKeepOn2DAreaGroup::tryCreate("SwitchKeepOn2DArea",
                                                               switchAreaObjDir, switchPlayerHolder);
    if (switchKeepOn2D)
        al::registerSwitchKeepOnAreaGroup(this, (al::SwitchKeepOnAreaGroup*)switchKeepOn2D);

    auto* switchKeepOn2DExcept = SwitchKeepOn2DExceptDokanInAreaGroup::tryCreate(
        "SwitchKeepOn2DExceptDokanInArea", switchAreaObjDir, switchPlayerHolder);
    if (switchKeepOn2DExcept)
        al::registerSwitchKeepOnAreaGroup(this, (al::SwitchKeepOnAreaGroup*)switchKeepOn2DExcept);

    auto* switchKeepOnWater = SwitchKeepOnPlayerInWaterAreaGroup::tryCreate(
        "SwitchKeepOnPlayerInWaterArea", switchAreaObjDir, switchPlayerHolder);
    if (switchKeepOnWater)
        al::registerSwitchKeepOnAreaGroup(this, (al::SwitchKeepOnAreaGroup*)switchKeepOnWater);

    auto* switchKeepOnIgnoreOff = SwitchKeepOnIgnoreOffAreaTarget::tryCreate(
        "SwitchKeepOnIgnoreOffAreaTarget", switchAreaObjDir);
    if (switchKeepOnIgnoreOff)
        al::registerSwitchKeepOnAreaGroup(this, (al::SwitchKeepOnAreaGroup*)switchKeepOnIgnoreOff);

    rs::applyGameConfigData(this, mGameDataHolder->getGameConfigData());
    bool isSeparatePlay = rs::isSeparatePlay(this);
    rs::setSeparatePlayMode(this, isSeparatePlay);

    al::Scene::endInit(actorInitInfo);

    al::createAudioEffect(this, "DistanceReverbSe");
    al::createAudioEffect(this, "DistanceReverbBgm");
    al::createAudioEffect(this, "BgmLevelDetection");

    NerveExecutor::initNerve(&NrvStageScene.StartStageBgm, 30);

    if (mStateCollectBgm)
        al::initNerveState(this, mStateCollectBgm, &NrvStageScene.CollectBgm, "コレクトBGM");
    if (mStateCollectionList)
        al::initNerveState(this, mStateCollectionList, &NrvStageScene.CollectionList,
                           "コレクションリスト");
    if (mStateMiniGameRanking)
        al::initNerveState(this, mStateMiniGameRanking, &NrvStageScene.MiniGameRanking,
                           "ミニゲームランキング");
    if (mStateShop)
        al::initNerveState(this, mStateShop, &NrvStageScene.Shop, "ショップ");
    if (mStateCloset)
        al::initNerveState(this, mStateCloset, &NrvStageScene.Closet, "クローゼット");

    al::initNerveState(this, mStateGetShine, &NrvStageScene.DemoShineGet, "シャインゲットデモ");
    al::initNerveState(this, mStateTalk, &NrvStageScene.DemoTalk, "会話");
    al::initNerveState(this, mStateRadicon, &NrvStageScene.Radicon, "ラジコン");
    al::addNerveState(this, mStateWarp, &NrvStageScene.Warp, "ワープ");
    al::addNerveState(this, stateGetLifeMaxUpItem, &NrvStageScene.DemoGetLifeMaxUpItem,
                      "最大ライフアップアイテム取得");
    al::initNerveState(this, mStateWorldIntroCamera, &NrvStageScene.DemoWorldIntroCamera,
                       "ワールド紹介カメラ");
    al::addNerveState(this, mStateWorldIntroCamera,
                      &NrvStageScene.DemoWorldIntroCameraBeforeAppearElectricDemo,
                      "ワールド紹介カメラ[電線デモ前]");
    al::addNerveState(this, mStateSnapShot, &NrvStageScene.SnapShot, "スナップショット");

    if (mPyramid) {
        mStateGetShineMainSandWorld->setStateGetShine(mStateGetShineMain);
        mStateGetShineMainSandWorld->setStateScenarioCamera(mStateScenarioCamera);
        mStateGetShineMainSandWorld->setStateRecoverLife(mStateRecoverLife);
        mStateGetShineMainSandWorld->setStateGetShineLast(stateGetShineMainLast);
        mStateGetShineMainSandWorld->setPyramid(mPyramid);
        al::initNerveState(this, mStateGetShineMainSandWorld, &NrvStageScene.DemoShineMainGet,
                           "メインシャインゲットデモ[砂ワールド]");
    } else if (waterfallState) {
        waterfallState->setStateGetShine(mStateGetShineMain);
        waterfallState->setStateScenarioCamera(mStateScenarioCamera);
        waterfallState->setStateRecoverLife(mStateRecoverLife);
        waterfallState->setStateGetShineLast(stateGetShineMainLast);
        al::initNerveState(this, waterfallState, &NrvStageScene.DemoShineMainGet,
                           "メインシャインゲットデモ[滝ワールド]");
    } else if (cageShineState) {
        cageShineState->setState(mStateGetShineMain, mStateScenarioCamera, mStateRecoverLife,
                                 stateGetShineMainLast);
        al::initNerveState(this, cageShineState, &NrvStageScene.DemoShineMainGet,
                           "メインシャインゲットデモ[ケージシャイン]");
    } else {
        al::initNerveState(this, mStateGetShineMain, &NrvStageScene.DemoShineMainGet,
                           "メインシャインゲットデモ");
    }

    mStateGetShineMain->setScenarioCameraState(mStateScenarioCamera);

    al::initNerveState(this, mStateGetShineGrand, &NrvStageScene.DemoShineGrandGet,
                       "グランドシャインゲットデモ");
    al::initNerveState(this, mStateSkipDemo, &NrvStageScene.SkipDemo, "デモスキップ");
    al::initNerveState(this, mStateMiss, &NrvStageScene.Miss, "ミス");
    al::initNerveState(this, mStateCarryMeat, &NrvStageScene.DemoCarryMeat, "肉運び");
    mStateCarryMeat->setState(mStateSkipDemo);

    if (mStateYukimaruRace)
        al::initNerveState(this, mStateYukimaruRace, &NrvStageScene.RaceYukimaru,
                           "ユキマルレース");
    if (mStateYukimaruRaceTutorial)
        al::initNerveState(this, mStateYukimaruRaceTutorial, &NrvStageScene.RaceYukimaruTutorial,
                           "ユキマルレースチュートリアル");
    if (mStateRaceManRace)
        al::initNerveState(this, mStateRaceManRace, &NrvStageScene.RaceManRace,
                           "かけっこレース");
    if (mStateTitleLogo)
        al::initNerveState(this, mStateTitleLogo, &NrvStageScene.DemoTitleLogo, "タイトルロゴ");
    al::initNerveState(this, mStagePauseMenu, &NrvStageScene.Pause, "ポーズメニュー");
    al::initNerveState(this, mStateCheckpointWarp, &NrvStageScene.ArriveAtCheckpoint,
                       "中間ポイントワープ");
    if (mTimeBalloonNpc)
        al::initNerveState(this, mStateTimeBalloon, &NrvStageScene.TimeBalloon, "風船ゲーム");
    al::addNerveState(this, mStateScenarioCamera, &NrvStageScene.DemoScenarioCamera,
                      "シナリオ開始");

    mStateScenarioCamera->setScenarioStartCameraHolder(mScenarioStartCameraHolder);
    mStateScenarioCamera->setScenarioStartLayout(mScenarioStartLayout);

    if (rs::isShopStatusBuyMaxLifeUpItem(mDemoShine))
        rs::requestStartDemoGetLifeMaxUpItem(mDemoShine);

    if (GameDataFunction::isWarpCheckpoint(mGameDataHolder)) {
        al::setNerve(this, &NrvStageScene.ArriveAtCheckpoint);
    } else {
        if (GameDataFunction::isPlayDemoWorldWarpHole(GameDataHolderAccessor(this))) {
            al::setNerve(this, &NrvStageScene.AppearFromWorldWarpHole);
        } else {
            s32 currentWorldIdNoDevelop =
                GameDataFunction::getCurrentWorldIdNoDevelop(GameDataHolderAccessor(this));
            if (!GameDataFunction::isUnlockedWorld(mGameDataHolder, currentWorldIdNoDevelop)) {
                al::setNerve(this, &NrvStageScene.StartStageBgm);
            } else if (hasOpeningDemo) {
                al::setNerve(this, &NrvStageScene.DemoStageStartOpening);
            } else if (hasWaterfallStartDemo) {
                rs::changeDemoCommonProc(this, mProjectItemDirector);
                al::setNerve(this, &NrvStageScene.DemoWorldIntroCameraBeforeAppearElectricDemo);
            } else if (mOpeningStageStartDemo &&
                       ((OpeningStageStartDemo*)mOpeningStageStartDemo)->isEnableStart()) {
                al::setNerve(this, &NrvStageScene.DemoStageStart);
            } else if (GameDataFunction::isPlayDemoWorldWarp(GameDataHolderAccessor(this)) ||
                       mGameDataHolder->getGameDataFile()->getMainScenarioNoCurrent() == 2) {
                rs::changeDemoCommonProc(this, mProjectItemDirector);
                al::setNerve(this, &NrvStageScene.DemoWorldIntroCamera);
            } else if (tryChangeDemo()) {
                // nerve already set by tryChangeDemo
            } else if (mStateScenarioCamera->tryStart()) {
                al::resetRequestCaptureScreenCover(this);
                al::setNerve(this, &NrvStageScene.DemoScenarioCamera);
            } else if (GameDataFunction::isRaceStartYukimaru(GameDataHolderAccessor(this))) {
                al::setNerve(this, &NrvStageScene.RaceYukimaru);
            } else if (GameDataFunction::isRaceStartYukimaruTutorial(GameDataHolderAccessor(this))) {
                al::setNerve(this, &NrvStageScene.RaceYukimaruTutorial);
            } else if (mStateRaceManRace) {
                al::setNerve(this, &NrvStageScene.RaceManRace);
            } else if (GameDataFunction::isPlayDemoPlayerDownForBattleKoopaAfter(
                           GameDataHolderAccessor(this))) {
                al::setNerve(this, &NrvStageScene.DemoPlayerDown);
                GameDataFunction::disablePlayDemoPlayerDownForBattleKoopaAfter(GameDataHolderWriter(this));
            } else if (CapManHeroDemoUtil::isExistTalkDemoStageStart(this)) {
                al::setNerve(this, &NrvStageScene.DemoStageStartCapManHeroTalk);
            } else if (CapManHeroDemoUtil::isExistTalkDemoAfterMoonRockBreakDemo(this)) {
                al::setNerve(this, &NrvStageScene.DemoCapManHeroTalkMoonRock);
            } else {
                if (GameDataFunction::isEnterStageFirst(mGameDataHolder) &&
                    al::isExistSceneObj(this, SceneObjID_GiantWanderBoss)) {
                    GiantWanderBoss* giantWanderBoss =
                        static_cast<GiantWanderBoss*>(al::getSceneObj(
                            this, SceneObjID_GiantWanderBoss));
                    if (GiantWanderBoss::isOnSwitchAll(giantWanderBoss)) {
                        rs::requestStartDemoWithPlayer(this);
                        rs::addDemoActor(giantWanderBoss, false);
                        giantWanderBoss->startDemoWorldEnter();
                        rs::changeDemoCommonProc(this, mProjectItemDirector);
                    }
                }
            }
        }
    }

    if (al::isNerve(this, &NrvStageScene.DemoTalk) ||
        al::isNerve(this, &NrvStageScene.DemoReturnToHome))
        mIsUpdateKitAndGraphics = true;

    if (mPyramid) {
        if (rs::isInvalidChangeStage(mPyramid)) {
            ((Pyramid*)mPyramid)->resetAtCloseAndFly();
        } else {
            s32 scenarioNo = mScenarioNo;
            if (scenarioNo == 2) {
                if (al::isNerve(this, &NrvStageScene.DemoReturnToHome)) {
                    ((Pyramid*)mPyramid)->resetAtOpenAndGround();
                    goto afterPyramidReset;
                }
                scenarioNo = mScenarioNo;
            }
            if (scenarioNo < 2 || (scenarioNo & ~1) == 8) {
                s32 activeQuestNo = rs::getActiveQuestNo(this);
                if (activeQuestNo < 2)
                    ((Pyramid*)mPyramid)->resetAtCloseAndGround();
                else
                    ((Pyramid*)mPyramid)->resetAtOpenAndGround();
            } else {
                ((Pyramid*)mPyramid)->resetAtOpenAndFly();
            }
        }
    }
afterPyramidReset:

    if (mTimeBalloonNpc && mStateTimeBalloon->isAutoStart())
        al::setNerve(this, &NrvStageScene.TimeBalloon);

    if (!al::isNerve(this, &NrvStageScene.StartStageBgm)) {
        rs::offRouteGuideSystem(this);
        rs::endPlayTalkMsgTimeBalloonLayout(this);
        if (_408)
            ((TimeBalloonSequenceInfo*)_408)->disableLayout();
    }

    if (GameDataFunction::isMissEndPrevStageForInit(mGameDataHolder))
        mIsExistShineOrCoinCollectInStage = false;

    GameDataFunction::enteredStage(mGameDataHolder);

    if (rs::isShopStatusBuyShineMany(mDemoShine))
        mStateGetShine->setShopShine10();
    if (rs::isShopStatusBuyItem(mDemoShine))
        mIsUpdateKitAndGraphics = true;
    rs::setShopStatusNone(mDemoShine);

    if (al::isNerve(this, &NrvStageScene.DemoStageStart) ||
        al::isNerve(this, &NrvStageScene.DemoStageStartOpening)) {
        ((OpeningStageStartDemo*)mOpeningStageStartDemo)->startDemo();
    } else if (al::isNerve(this, &NrvStageScene.DemoStageStartCapManHeroTalk)) {
        CapManHeroDemoUtil::startTalkDemoStageStart(this);
    } else if (al::isNerve(this, &NrvStageScene.DemoCapManHeroTalkMoonRock)) {
        CapManHeroDemoUtil::startTalkDemoAfterMoonRockBreakDemo(this);
    }

    s32 mapIconStageInfoMapNum = al::getStageInfoMapNum(this);
    if (mapIconStageInfoMapNum >= 1) {
        for (s32 i = 0; i < mapIconStageInfoMapNum; i++) {
            const al::StageInfo* mapIconStageInfoMap = al::getStageInfoMap(this, i);
            al::PlacementInfo mapIconPlacementInfo;
            s32 mapIconCount = 0;
            al::tryGetPlacementInfoAndCount(&mapIconPlacementInfo, &mapIconCount,
                                            mapIconStageInfoMap, "MapIconList");
            if (mapIconCount >= 1) {
                for (s32 j = 0; j < mapIconCount; j++) {
                    al::PlacementInfo mapIconObjInfo;
                    al::getPlacementInfoByIndex(&mapIconObjInfo, mapIconPlacementInfo, j);
                    const char* mapIconName = nullptr;
                    al::getObjectName(&mapIconName, mapIconObjInfo);
                    if (al::isEqualString(mapIconName, "MapIconShop")) {
                        GameDataFunction::setShopNpcTrans(mGameDataHolder, mapIconObjInfo);
                    } else if (al::isEqualString(mapIconName, "MapIconMiniGame")) {
                        GameDataFunction::setMiniGameInfo(mGameDataHolder, mapIconObjInfo);
                    } else if (al::isEqualString(mapIconName, "MapIconPoetter")) {
                        sead::Vector3f poetterTrans = sead::Vector3f::zero;
                        al::getTrans(&poetterTrans, mapIconObjInfo);
                        GameDataFunction::setPoetterTrans(mGameDataHolder, poetterTrans);
                    }
                }
            }
        }
    }

    mChromakeyDrawer = new al::ChromakeyDrawer(getLiveActorKit()->getGraphicsSystemInfo(),
                                                al::getSceneExecuteDirector(this),
                                                getLiveActorKit()->getEffectSystem());

    GameDataFunction::noPlayDemoWorldWarp(GameDataHolderWriter(this));
}

void StageScene::exeDemoGetLifeMaxUpItem() {
    if (mIsUpdateKitAndGraphics) {
        al::updateKit(this);
        al::updateGraphicsPrev(this);
        mIsUpdateKitAndGraphics = false;
        return;
    }

    if (al::updateNerveState(this)) {
        rs::endDemoCommonProc(this, mProjectItemDirector);
        al::updateKitListPostOnNerveEnd(this);
        al::setNerve(this, &NrvStageScene.Play);
        return;
    }

    al::updateKitListPostOnNerveEnd(this);
}

void StageScene::exeDemoHackStart() {
    bool isRadicon = !GameDataFunction::isMainStage(this) && rs::isPlayerHackRadiconNpc(mDemoShine);

    if (al::isFirstStep(this)) {
        if (isRadicon) {
            mSceneLayout->end();
            rs::setKidsModeLayoutDisable(this);
        }
        rs::killCoinCollectHintEffect(this);
    }

    al::updateKitListPrev(this);
    rs::updateKitListDemoPlayerWithPauseEffect(this);
    if (isRadicon)
        mSceneLayout->update();
    al::updateKitListPostDemoWithPauseNormalEffect(this);

    if (!rs::isActiveDemo(this)) {
        endDemoAndChangeNerve();
        if (isRadicon)
            al::setNerve(this, &NrvStageScene.Radicon);
        return;
    }

    al::updateKitListPostOnNerveEnd(this);
}
