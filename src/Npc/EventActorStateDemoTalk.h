#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}

class EventActorStateDemoTalk : public al::ActorStateBase {
public:
    EventActorStateDemoTalk(al::LiveActor* actor);

    void appear() override;

    const char* mActionName = "Talk";
};
