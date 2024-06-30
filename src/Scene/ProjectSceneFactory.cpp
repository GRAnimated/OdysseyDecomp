#include "Scene/ProjectSceneFactory.h"

#include "Scene/TitleMenuScene.h"

static al::NameToCreator<alSceneFunction::SceneCreatorFunction> sProjectSceneFactoryEntries[] = {
    {"DemoChangeWorldScene", nullptr},
    {"DemoScene", nullptr},
    {"DemoSceneWithCinemaCaption", nullptr},
    {"EndingScene", nullptr},
    {"FirstSequenceScene", nullptr},
    {"StageScene", nullptr},
    {"StaffRollScene", nullptr},
    {"TitleMenuScene", alSceneFunction::createSceneFunc<TitleMenuScene>},
    {"WorldWarpHoleScene", nullptr}};

ProjectSceneFactory::ProjectSceneFactory(const char* factoryName) : SceneFactory("シーン生成") {
    initFactory(sProjectSceneFactoryEntries);
}
