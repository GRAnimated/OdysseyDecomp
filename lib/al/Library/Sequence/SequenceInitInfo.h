#pragma once

#include "Library/System/GameSystemInfo.h"

namespace al {
struct SequenceInitInfo {
public:
    SequenceInitInfo(al::GameSystemInfo const* gameSystemInfo);

    const GameSystemInfo* systemInfo;
};
}  // namespace al
