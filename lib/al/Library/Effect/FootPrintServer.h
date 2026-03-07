#pragma once

#include <basis/seadTypes.h>

namespace al {
class ActorInitInfo;
class FootPrintServer {
public:
    FootPrintServer(const ActorInitInfo& info, const char* name, s32 maxFootPrints);

private:
    u8 _padding[0x10];
};

static_assert(sizeof(FootPrintServer) == 0x10);
}  // namespace al
