#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class Scene;
class SimpleAudioUser;
class SimpleLayoutAppearWaitEnd;
struct ActorInitInfo;
}  // namespace al

class StageSceneStateSkipDemo;

class StageSceneStateWorldIntroCamera : public al::HostStateBase<al::Scene> {
public:
    StageSceneStateWorldIntroCamera(const char*, const al::ActorInitInfo&, al::Scene*,
                                    al::SimpleLayoutAppearWaitEnd*,
                                    al::SimpleLayoutAppearWaitEnd*, al::SimpleAudioUser*,
                                    StageSceneStateSkipDemo*);

private:
    unsigned char _padding[0x50 - 0x20];
};

static_assert(sizeof(StageSceneStateWorldIntroCamera) == 0x50);
