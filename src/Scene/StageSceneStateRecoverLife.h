#pragma once

#include "Library/Nerve/NerveStateBase.h"

class StageScene;
class StageSceneLayout;

class StageSceneStateRecoverLife : public al::HostStateBase<StageScene> {
public:
    StageSceneStateRecoverLife(const char*, StageScene*, StageSceneLayout*);
    void init();
};
