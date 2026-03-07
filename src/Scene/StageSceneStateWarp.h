#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
class WipeSimple;
}  // namespace al

class GameDataHolder;
class LocationNameCtrl;

class StageSceneStateWarp : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateWarp(const char*, al::Scene*, al::WipeSimple*, GameDataHolder*,
                        LocationNameCtrl*);
    void init();

private:
    unsigned char _padding[0x48 - 0x20];
};

static_assert(sizeof(StageSceneStateWarp) == 0x48);
