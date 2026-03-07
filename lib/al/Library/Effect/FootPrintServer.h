#pragma once

#include <basis/seadTypes.h>

namespace al {
class ActorInitInfo;
class FootPrintServer {
public:
    FootPrintServer(const ActorInitInfo& info, const char* name, s32 maxFootPrints);
};
}  // namespace al
