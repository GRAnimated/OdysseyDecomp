#pragma once

#include <basis/seadTypes.h>

#include "Library/Camera/ActorCameraTarget.h"

class PlayerCameraTarget : public al::ActorCameraTarget {
public:
    PlayerCameraTarget(const al::LiveActor*);

private:
    u8 _padding[0x8];
};

static_assert(sizeof(PlayerCameraTarget) == 0x30);
