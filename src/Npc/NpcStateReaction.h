#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class HitSensor;
class LiveActor;
class SensorMsg;
}  // namespace al

class NpcStateReactionParam;

class NpcStateReaction : public al::ActorStateBase {
public:
    NpcStateReaction(al::LiveActor*, bool);

    static NpcStateReaction* create(al::LiveActor*, const NpcStateReactionParam*);
    static NpcStateReaction* createForHuman(al::LiveActor*, const NpcStateReactionParam*);
    static NpcStateReaction* createForHackActor(al::LiveActor*, const NpcStateReactionParam*);

    void appear() override;
    void kill() override;

    virtual bool receiveMsg(const al::SensorMsg*, al::HitSensor* self, al::HitSensor* other);
    virtual bool receiveMsgWithoutTrample(const al::SensorMsg*, al::HitSensor* self,
                                          al::HitSensor* other);
    virtual bool receiveMsgNoReaction(const al::SensorMsg*, al::HitSensor* self,
                                      al::HitSensor* other);
    virtual bool isCapReaction() const;

    void exeReaction();
    void exeCapReaction();

    void setCapReaction(bool v) { mIsCapReaction = v; }

    NpcStateReactionParam* _20 = nullptr;
    bool mIsCapReaction = false;
    bool _29 = false;
};
