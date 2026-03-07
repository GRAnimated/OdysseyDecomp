#pragma once

#include "Library/Nerve/NerveStateBase.h"

class StageScene;
class StageSceneLayout;

class StageSceneStateRecoverLife : public al::HostStateBase<StageScene> {
public:
    StageSceneStateRecoverLife(const char*, StageScene*, StageSceneLayout*);
    void init();

private:
    unsigned char _padding[0x28 - 0x20];
};

static_assert(sizeof(StageSceneStateRecoverLife) == 0x28);
