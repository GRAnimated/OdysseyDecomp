#pragma once

#include <basis/seadTypes.h>

namespace al {
class AudioDirector;
class ApplicationMessageReceiver;
}

class DemoSoundSynchronizer {
public:
    DemoSoundSynchronizer(al::ApplicationMessageReceiver* receiver, al::AudioDirector* director);

private:
    u8 _padding[0x20];
};

static_assert(sizeof(DemoSoundSynchronizer) == 0x20);
