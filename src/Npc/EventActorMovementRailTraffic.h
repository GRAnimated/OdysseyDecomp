#pragma once

#include "Library/Event/EventFlowMovement.h"

namespace al {
class LiveActor;
}  // namespace al

class EventActorMovementRailTraffic : public al::EventFlowMovement {
public:
    EventActorMovementRailTraffic(const char* name, al::LiveActor* actor);
};
