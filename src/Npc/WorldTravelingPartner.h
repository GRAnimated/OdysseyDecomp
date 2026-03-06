#pragma once

#include "Library/LiveActor/LiveActor.h"

namespace al {
struct ActorInitInfo;
class IUseEventFlowData;
}  // namespace al

class NpcStateReaction;
class TalkNpcParam;

class WorldTravelingPartner : public al::LiveActor {
public:
    WorldTravelingPartner(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                    al::HitSensor* self) override;
    void initAfterPlacement() override;

    void startWait();
    void startTalk();
    void exeWait();
    void exeTalk();
    void exeScared();
    void exeReaction();

    TalkNpcParam* mTalkNpcParam = nullptr;
    al::IUseEventFlowData* mEventFlowData = nullptr;
    NpcStateReaction* mNpcStateReaction = nullptr;
};

static_assert(sizeof(WorldTravelingPartner) == 0x120);
