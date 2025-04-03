#pragma once

#include <basis/seadTypes.h>
#include <heap/seadHeap.h>

namespace alSceneFunction {
class SceneFactory;
}

namespace al {
class AudioDirectorInitInfo;
class GameDataHolderBase;
class ScreenCaptureExecutor;
class SequenceInitInfo;
class IUseSceneCreator;
class AudioDirector;
class Scene;
class Sequence;
class AudioSystemInfo;

void initSceneCreator(IUseSceneCreator*, const SequenceInitInfo&, GameDataHolderBase*,
                      AudioDirector*, ScreenCaptureExecutor*, alSceneFunction::SceneFactory*);
Scene* createSceneAndInit(IUseSceneCreator*, const char*, const char*, s32, const char*);
Scene* createSceneAndUseInitThread(IUseSceneCreator*, const char*, s32, const char*, s32,
                                   const char*);
void setSceneAndInit(IUseSceneCreator*, Scene*, const char*, s32, const char*);
void setSceneAndUseInitThread(IUseSceneCreator*, Scene*, s32, const char*, s32, const char*,
                              sead::Heap*);
bool tryEndSceneInitThread(IUseSceneCreator*);
bool isExistSceneInitThread(const IUseSceneCreator*);
void initAudioDirector(Sequence*, AudioSystemInfo*, AudioDirectorInitInfo&);
void setSequenceAudioKeeperToSceneSeDirector(Sequence*, Scene*);
void setSequenceNameForActorPickTool(Sequence*, Scene*);
}  // namespace al
