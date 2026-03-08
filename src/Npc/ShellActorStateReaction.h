#pragma once

#include "Npc/ActorStateReactionBase.h"

class ShellActorStateReaction : public ActorStateReactionBase {
public:
    ShellActorStateReaction(al::LiveActor* actor);

    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                    al::HitSensor* self) override;
    bool receiveMsgWithoutTrample(const al::SensorMsg* msg, al::HitSensor* other,
                                  al::HitSensor* self) override;
    bool receiveMsgNoReaction(const al::SensorMsg* msg, al::HitSensor* other,
                              al::HitSensor* self) override;
    bool isCapReaction() const override;

    void exeReactionStart();
    void exeReactionWait();
    void exeReactionShell();
    void exeReactionCap();
    void exeReactionCapShell();
    void exeReactionEnd();
};

static_assert(sizeof(ShellActorStateReaction) == 0x20);
