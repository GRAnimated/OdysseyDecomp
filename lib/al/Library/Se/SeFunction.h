#pragma once

#include <prim/seadSafeString.h>

namespace al {
class IUseAudioKeeper;
class AudioDirector;

bool tryStartSe(const IUseAudioKeeper*, const sead::SafeString&);
void startSe(const IUseAudioKeeper*, const sead::SafeString&);
}  // namespace al

namespace alSeFunction {
void stopAllSe(const al::AudioDirector*, u32);
}