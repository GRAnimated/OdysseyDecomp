#pragma once

#include <prim/seadSafeString.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/LiveActor/LiveActor.h"

namespace al {
struct ActorInitInfo;
class EventFlowExecutor;
class LiveActorGroup;
}  // namespace al

class TalkNpcActionAnimInfo;
class TalkNpcParam;
class TalkNpcStateEvent;
class WorldTravelingPartner;

class WorldTravelingNpc : public al::LiveActor, public al::IEventFlowEventReceiver {
public:
    WorldTravelingNpc(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                    al::HitSensor* self) override;
    bool receiveEvent(const al::EventFlowEventData* data) override;

    void exeEvent();

    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    TalkNpcStateEvent* mTalkNpcStateEvent = nullptr;
    TalkNpcParam* mTalkNpcParam = nullptr;
    TalkNpcActionAnimInfo* mTalkNpcActionAnimInfo = nullptr;
    WorldTravelingPartner* mPartner = nullptr;
    al::LiveActorGroup* mCarGroup = nullptr;
    sead::WFixedSafeString<32> mCharacterName;
};

static_assert(sizeof(WorldTravelingNpc) == 0x198);
