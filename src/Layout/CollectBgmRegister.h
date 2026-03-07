#pragma once

#include <basis/seadTypes.h>

namespace al {
class AudioDirector;
}

class CollectBgmPlayer;
class GameDataHolder;

class CollectBgmRegister {
public:
    CollectBgmRegister(const al::AudioDirector*, GameDataHolder*, CollectBgmPlayer*);

private:
    u8 _padding[0x30];
};

static_assert(sizeof(CollectBgmRegister) == 0x30);
