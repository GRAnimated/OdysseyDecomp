#pragma once

#include <basis/seadTypes.h>

#include "Library/Scene/ISceneObj.h"

namespace al {
class CameraDirector;
}

class CheckpointFlagWatcher : public al::ISceneObj {
public:
    CheckpointFlagWatcher(al::CameraDirector*);
    void initStageInfo(const char*, s32);

private:
    u8 _padding[0x88];
};

static_assert(sizeof(CheckpointFlagWatcher) == 0x90);
