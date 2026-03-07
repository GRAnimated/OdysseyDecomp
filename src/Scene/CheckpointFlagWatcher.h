#pragma once

#include "Library/Scene/ISceneObj.h"

namespace al {
class CameraDirector;
}

class CheckpointFlagWatcher : public al::ISceneObj {
public:
    CheckpointFlagWatcher(al::CameraDirector*);
    void initStageInfo(const char*, s32);
};
