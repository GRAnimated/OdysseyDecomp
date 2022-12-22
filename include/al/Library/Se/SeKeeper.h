#pragma once

#include "prim/seadSafeString.h"
#include "IUse/IUseAudioKeeper.h"

namespace al {
    void startSe(al::IUseAudioKeeper const*,sead::SafeStringBase<char> const&);
}