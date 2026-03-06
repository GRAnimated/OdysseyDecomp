#pragma once

#include "Library/Nerve/NerveStateBase.h"

struct ActorInitInfo;
class NpcStateReactionParam;
class NpcStateWaitParam;
class NpcStateTurnParam;
class NpcStateRumbleParam;

namespace al {
class LiveActor;
}  // namespace al

class NpcStateWait : public al::ActorStateBase {
public:
    NpcStateWait(al::LiveActor*, const al::ActorInitInfo&, const NpcStateWaitParam*,
                 const NpcStateTurnParam*, const NpcStateRumbleParam*);

private:
    u8 _20[0x28];
};

static_assert(sizeof(NpcStateWait) == 0x48);
