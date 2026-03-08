#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
class WipeSimple;
}  // namespace al

class GameDataHolder;
class LocationNameCtrl;

class StageSceneStateWarp : public al::NerveStateBase {
public:
    StageSceneStateWarp(const char*, al::Scene*, al::WipeSimple*, GameDataHolder*,
                        LocationNameCtrl*);

    bool tryStartWarp();
};
