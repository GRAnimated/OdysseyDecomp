#pragma once

namespace al {
class AudioDirector;
}

class CollectBgmPlayer;
class GameDataHolder;

class CollectBgmRegister {
public:
    CollectBgmRegister(const al::AudioDirector*, GameDataHolder*, CollectBgmPlayer*);
};
