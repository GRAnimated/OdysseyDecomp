#pragma once

#include <prim/seadSafeString.h>

#include "Library/Audio/IUseAudioKeeper.h"
#include "Library/Nerve/NerveExecutor.h"
#include "Library/Sequence/IUseSceneCreator.h"
#include "Library/System/GameSystemInfo.h"

namespace al {
class GameSystemInfo;
class SequenceInitInfo;
class AudioSystemInfo;
class AudioDirector;
class Scene;

namespace alSceneFunction {
class SceneFactory;
}
class GameDataHolderBase;
class ScreenCaptureExecutor;
class AudioDirectorInitInfo;

class Sequence : public NerveExecutor, public IUseAudioKeeper, public IUseSceneCreator {
public:
    Sequence(const char* name);
    virtual ~Sequence() override;
    virtual void init(const SequenceInitInfo& info);
    virtual void update();
    virtual void kill();
    virtual void drawMain() const;
    virtual void drawSub() const;

    virtual bool isDisposable() const;

    virtual Scene* getCurrentScene() const;

    void setCurrentScene(Scene* scene) { mCurrentScene = scene; }

    virtual SceneCreator* getSceneCreator() const override;
    virtual void setSceneCreator(SceneCreator* sceneCreator) override;

    AudioKeeper* getAudioKeeper() const override;
    void initAudio(const GameSystemInfo&, const char*, s32, s32, s32, const char*);
    void initAudioKeeper(const char*);
    void initDrawSystemInfo(const SequenceInitInfo&);
    AudioSystemInfo* getAudioSystemInfo();

    AudioDirector* getAudioDirector() const { return mAudioDirector; }

    DrawSystemInfo* getDrawInfo() const { return mDrawSystemInfo; }

private:
    sead::FixedSafeString<0x40> mName;
    Scene* mNextScene;
    Scene* mCurrentScene;
    SceneCreator* mSceneCreator;
    AudioDirector* mAudioDirector;
    AudioKeeper* mAudioKeeper;
    DrawSystemInfo* mDrawSystemInfo;
    bool mIsAlive;
};

void initSceneCreator(IUseSceneCreator*, const SequenceInitInfo&, GameDataHolderBase*,
                      AudioDirector*, ScreenCaptureExecutor*, alSceneFunction::SceneFactory*);
void createSceneAndInit(IUseSceneCreator*, const char*, const char*, s32, const char*);
void createSceneAndUseInitThread(IUseSceneCreator*, const char*, s32, const char*, s32,
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
