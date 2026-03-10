#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class IUseEventFlowData;
class LiveActor;
}  // namespace al

class NpcStateTurnBase : public al::ActorStateBase {
public:
    NpcStateTurnBase(const char* name, al::LiveActor* actor);

    void startActionLocal(const char* action);
    virtual void initEventUserForAction(const al::IUseEventFlowData* user);

protected:
    const al::IUseEventFlowData* mEventFlowUser = nullptr;
};

static_assert(sizeof(NpcStateTurnBase) == 0x28);
