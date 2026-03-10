#pragma once

#include "Library/Event/EventFlowMovement.h"

namespace al {
class LiveActor;
}

class TalkNpcActionAnimInfo;
class TalkNpcParam;

class EventActorMovementTurn : public al::EventFlowMovement {
public:
    EventActorMovementTurn(al::LiveActor*, const TalkNpcParam*, const TalkNpcActionAnimInfo*);

private:
    void* _30 = nullptr;
    void* _38 = nullptr;
    void* _40 = nullptr;
    void* _48 = nullptr;
    void* _50 = nullptr;
    void* _58 = nullptr;
    void* _60 = nullptr;
    void* _68 = nullptr;
    void* _70 = nullptr;
};

static_assert(sizeof(EventActorMovementTurn) == 0x78);
