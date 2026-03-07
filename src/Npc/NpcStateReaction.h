#pragma once

#include "Npc/ActorStateReactionBase.h"

namespace al {
class HitSensor;
class LiveActor;
class SensorMsg;
}  // namespace al

class NpcStateReactionParam;

class NpcStateReaction : public ActorStateReactionBase {
public:
    NpcStateReaction(al::LiveActor*, bool);

    static NpcStateReaction* create(al::LiveActor*, const NpcStateReactionParam*);
    static NpcStateReaction* createForHuman(al::LiveActor*, const NpcStateReactionParam*);
    static NpcStateReaction* createForHackActor(al::LiveActor*, const NpcStateReactionParam*);

    void appear() override;
    void kill() override;

    bool receiveMsg(const al::SensorMsg*, al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsgWithoutTrample(const al::SensorMsg*, al::HitSensor* self,
                                  al::HitSensor* other) override;
    bool receiveMsgNoReaction(const al::SensorMsg*, al::HitSensor* self,
                              al::HitSensor* other) override;
    bool isCapReaction() const override;

    void exeReaction();
    void exeCapReaction();

    void setCapReaction(bool v) { mIsCapReaction = v; }

    NpcStateReactionParam* _20 = nullptr;
    bool mIsCapReaction = false;
    bool _29 = false;
};
