#pragma once

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Message/IUseMessageSystem.h"

namespace al {
class EventFlowEventData;
class EventFlowExecutor;
class MessageSystem;
}  // namespace al

class NpcEventStateScare;
class NpcJointLookAtController;
class NpcStateReaction;
class TalkNpcActionAnimInfo;
class TalkNpcParam;

class HelpNpc : public al::LiveActor,
                public al::IUseMessageSystem,
                public al::IEventFlowEventReceiver {
public:
    HelpNpc(const char* name);

    void init(const al::ActorInitInfo& info) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                    al::HitSensor* self) override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    void control() override;

    bool receiveEvent(const al::EventFlowEventData* event) override;
    const al::MessageSystem* getMessageSystem() const override;

    void exeWait();
    void exeScare();
    void exeReaction();

private:
    NpcEventStateScare* mStateScare = nullptr;
    NpcStateReaction* mStateReaction = nullptr;
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    const al::MessageSystem* mMessageSystem = nullptr;
    NpcJointLookAtController* mJointLookAtController = nullptr;
    TalkNpcParam* mTalkNpcParam = nullptr;
    TalkNpcActionAnimInfo* mActionAnimInfo = nullptr;
    bool mIsCapTarget = false;
};

static_assert(sizeof(HelpNpc) == 0x158);
