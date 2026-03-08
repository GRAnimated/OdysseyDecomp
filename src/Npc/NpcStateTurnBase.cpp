#include "Npc/NpcStateTurnBase.h"

#include "Library/Event/EventFlowUtil.h"
#include "Library/LiveActor/LiveActor.h"

#include "Util/NpcAnimUtil.h"

NpcStateTurnBase::NpcStateTurnBase(const char* name, al::LiveActor* actor)
    : al::ActorStateBase(name, actor) {}

void NpcStateTurnBase::startActionLocal(const char* action) {
    if (mEventFlowUser)
        al::startEventAction(mActor, mEventFlowUser, action);
    else
        rs::startNpcAction(mActor, action);
}

void NpcStateTurnBase::initEventUserForAction(const al::IUseEventFlowData* user) {
    mEventFlowUser = user;
}
