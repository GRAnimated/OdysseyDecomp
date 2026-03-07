#pragma once

#include <basis/seadTypes.h>

namespace al {
class AudioDirector;

class DemoSyncedEventKeeper {
public:
    DemoSyncedEventKeeper(AudioDirector* audioDirector);

private:
    u8 _padding[0x20];
};

static_assert(sizeof(DemoSyncedEventKeeper) == 0x20);
}  // namespace al
