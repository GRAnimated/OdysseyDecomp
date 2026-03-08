#pragma once

#include "Library/LiveActor/LiveActor.h"
#include "Library/Scene/ISceneObj.h"

namespace al {
class StageInfo;
struct ActorInitInfo;
}  // namespace al

class CapMessageMoonNotifier : public al::LiveActor, public al::ISceneObj {
public:
    void initialize(const al::StageInfo*, const al::ActorInitInfo&);
};
