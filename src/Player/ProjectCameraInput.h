#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}

class ProjectCameraInput {
public:
    ProjectCameraInput(const al::LiveActor*);

private:
    u8 _padding[0x10];
};

static_assert(sizeof(ProjectCameraInput) == 0x10);
