#pragma once

#include "Library/Scene/ISceneObj.h"

namespace al {
class AreaObjGroup;
class CameraDirector;
struct ActorInitInfo;
}  // namespace al

struct PlayerStartInfo;

class PlayerStartInfoHolder : public al::ISceneObj {
public:
    void init(al::CameraDirector*, s32);
    void registerChangeStageAreaGroup(al::AreaObjGroup*, const al::ActorInitInfo&);
    void registerWarpAreaGroup(al::AreaObjGroup*, const al::ActorInitInfo&);
    void registerRestartAreaGroup(al::AreaObjGroup*);
    const PlayerStartInfo* tryFindInitInfoByStartId(const char*);
};
