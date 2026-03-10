#pragma once

#include "Library/Event/EventFlowMovement.h"

namespace al {
class LiveActor;

class EventFlowMovementRailBlend : public EventFlowMovement {
public:
    EventFlowMovementRailBlend(LiveActor* actor);

private:
    void* _30 = nullptr;
    void* _38 = nullptr;
};
}  // namespace al
