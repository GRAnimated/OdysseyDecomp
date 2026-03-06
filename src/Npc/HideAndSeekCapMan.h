#pragma once

#include "Library/LiveActor/LiveActor.h"

namespace al {
struct ActorInitInfo;
}  // namespace al

class HideAndSeekCapMan {
public:
    static al::LiveActor* tryCreateLinkNpcIfClearGame(al::LiveActor* parent,
                                                      const al::ActorInitInfo& info,
                                                      const char* suffix,
                                                      const char* linkName);
};
