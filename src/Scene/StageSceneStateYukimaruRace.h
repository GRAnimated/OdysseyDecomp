#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
class Scene;
}  // namespace al

class SceneAudioSystemPauseController;
class StageSceneLayout;
class StageSceneStateSnapShot;

class StageSceneStateYukimaruRace : public al::HostStateBase<al::Scene> {
public:
    static StageSceneStateYukimaruRace* tryCreate(al::Scene*, const al::ActorInitInfo&,
                                                   StageSceneLayout*,
                                                   SceneAudioSystemPauseController*,
                                                   StageSceneStateSnapShot*);
};
