#pragma once

#include <basis/seadTypes.h>

namespace al {
struct ActorInitInfo;
}

class StageSceneLayout;
class TimeBalloonNpc;
class TimeBalloonSequenceInfo;

class TimeBalloonDirector {
public:
    void init(const al::ActorInitInfo&, TimeBalloonNpc*, TimeBalloonSequenceInfo*, s32, s32,
              StageSceneLayout*);
    void* getAccessor();
};
