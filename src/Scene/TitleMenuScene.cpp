#include "Scene/TitleMenuScene.h"
#include <gfx/seadFrameBuffer.h>
#include "Library/Draw/GraphicsSystemInfo.h"
#include "Library/Scene/Scene.h"
#include "Library/Scene/SceneObjHolder.h"
#include "Library/Scene/SceneUtil.h"
#include "Scene/SceneObjFactory.h"
#include "System/GameDataFunction.h"

TitleMenuScene::TitleMenuScene() : al::Scene("TitleMenuScene") {}

void TitleMenuScene::init(al::SceneInitInfo const& info) {
    initDrawSystemInfo(info);
    initAndLoadStageResource("SandWorldHomeStage", 1);
    al::SceneObjHolder* sceneObjHolder = SceneObjFactory::createSceneObjHolder();
    initSceneObjHolder(sceneObjHolder);

    GameDataHolder* holder = nullptr;
    GameDataFunction::getGameDataHolder(holder);
    al::setSceneObj(this, holder, 18);

    initSceneStopCtrl();

    al::GraphicsInitArg graphicsInitArg(al::getSceneDrawContext(this),
                                        al::getSceneFrameBufferConsole(this));
}

void TitleMenuScene::appear() {}

void TitleMenuScene::control() {}

void TitleMenuScene::drawMain() const {}

bool TitleMenuScene::isEnableKill() const {}

bool TitleMenuScene::isChangeLanguage() const {}

void TitleMenuScene::getLanguage() const {}

bool TitleMenuScene::isNewGame() const {}

void TitleMenuScene::startLoadDirect(bool) {}

void TitleMenuScene::setScenario() {}

void TitleMenuScene::setLoadPercent(f32 percent) {}

bool TitleMenuScene::isCancelLoadWorldResource() const {}

void TitleMenuScene::startLoadWorldResource() {}

void TitleMenuScene::exeAppear() {}

void TitleMenuScene::exeMenu() {}

void TitleMenuScene::exeWipe() {}

void TitleMenuScene::exeLoadAppear() {}

void TitleMenuScene::exeLoadWait() {}
