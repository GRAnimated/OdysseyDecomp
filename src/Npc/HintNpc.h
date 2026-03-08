#pragma once

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/Event/IEventFlowQueryJudge.h"
#include "Library/LiveActor/LiveActor.h"

namespace al {
struct ActorInitInfo;
class EventFlowEventData;
class EventFlowExecutor;
class HitSensor;
class SensorMsg;
}  // namespace al

class ActorStateReactionBase;
class EventActorMovementTurn;
class TalkNpcActionAnimInfo;
class TalkNpcCap;
class TalkNpcParam;
class TalkNpcStateEvent;

class HintNpc : public al::LiveActor,
                public al::IEventFlowEventReceiver,
                public al::IEventFlowQueryJudge {
public:
    HintNpc(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                    al::HitSensor* self) override;

    bool receiveEvent(const al::EventFlowEventData* data) override;
    const char* judgeQuery(const char* query) const override;

    void exeEvent();
    void exeReaction();

    TalkNpcCap* mNpcCap = nullptr;
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    const TalkNpcParam* mParam = nullptr;
    TalkNpcActionAnimInfo* mActionAnimInfo = nullptr;
    const char** mShineInfo = nullptr;
    TalkNpcStateEvent* mStateEvent = nullptr;
    ActorStateReactionBase* mReaction = nullptr;
    EventActorMovementTurn* mMovementTurn = nullptr;
};

static_assert(sizeof(HintNpc) == 0x158);
