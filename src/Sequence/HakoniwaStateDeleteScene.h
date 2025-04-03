#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
class Sequence;
}  // namespace al
class WorldResourceLoader;

class HakoniwaStateDeleteScene : public al::HostStateBase<al::Sequence> {
public:
    HakoniwaStateDeleteScene(al::Sequence*, WorldResourceLoader*);
    void deleteScene();
    void appear() override;
    void kill() override;
    void start(al::Scene*, bool, bool, s32);
    void exePrepare();
    void exeFinalizeAudio();
    void exeDeleteScene();

private:
    void* filler[4];
};
