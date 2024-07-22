#pragma once

#include <basis/seadTypes.h>

namespace al {
class IUseAudioKeeper;
class Resource;

class BgmDataBase {
public:
    static BgmDataBase* create(const char*, const char*);

    BgmDataBase(const Resource*, const Resource*);
};

void startBgm(const al::IUseAudioKeeper*, const char*, s32, s32);
void stopBgm(const al::IUseAudioKeeper*, const char*, s32);
void stopAllBgm(const al::IUseAudioKeeper*, s32);
bool isRunningBgm(const al::IUseAudioKeeper*, const char*);

}  // namespace al
